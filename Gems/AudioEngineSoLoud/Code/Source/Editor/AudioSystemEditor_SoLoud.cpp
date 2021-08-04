/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioConnections.h>
#include <AudioFileToTriggerParamsWidget.h>
#include <AudioFileToRtpcParamsWidget.h>
#include <AudioFileUtils.h>
#include <AudioSystemEditor_SoLoud.h>
#include <AzCore/StringFunc/StringFunc.h>
#include <AzCore/Utils/Utils.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <IAudioSystem.h>

namespace AudioControls
{
    //----------------------------------------------
    namespace ESoLoudControlType
    {
        enum Type
        {
            Invalid = 0,
            AudioFile = AUDIO_BIT(0),
            GlobalRTPC = AUDIO_BIT(1),
        };
    } // namespace ESoLoudControlType

    //----------------------------------------------
    void CAudioSystemEditor_SoLoud::Reload()
    {
        // set all the controls as placeholder as we don't know if
        // any of them have been removed but still have connections to them
        for (const auto& idControlPair : m_controls)
        {
            TControlPtr control = idControlPair.second;
            if (control)
            {
                control->SetPlaceholder(true);
            }
        }

        // Create global RTPCs.
        for (int i = 0; i < Audio::EGlobalRtpc::Count; ++i)
        {
            SControlDef controlDef(Audio::EGlobalRtpc::ToString(static_cast<Audio::EGlobalRtpc::Type>(i)), ESoLoudControlType::GlobalRTPC);
            CreateControl(controlDef);
        }

        ScanAudioFilesAndCreateControls(Audio::AudioFilesPath);

        m_connectionsByID.clear();
        xmlAlloc.clear();
    }

    IAudioSystemControl* CAudioSystemEditor_SoLoud::CreateControl(const SControlDef& controlDefinition)
    {
        AZStd::string fullName = controlDefinition.m_name;
        IAudioSystemControl* parent = controlDefinition.m_parentControl;
        if (parent)
        {
            AZ::StringFunc::Path::Join(controlDefinition.m_parentControl->GetName().c_str(), fullName.c_str(), fullName);
        }

        if (!controlDefinition.m_path.empty())
        {
            AZ::StringFunc::Path::Join(controlDefinition.m_path.c_str(), fullName.c_str(), fullName);
        }

        CID id = GetID(fullName);

        IAudioSystemControl* control = GetControl(id);
        if (control)
        {
            if (control->IsPlaceholder())
            {
                control->SetPlaceholder(false);
                if (parent && parent->IsPlaceholder())
                {
                    parent->SetPlaceholder(false);
                }
            }
            return control;
        }
        else
        {
            TControlPtr newControl = AZStd::make_shared<IAudioSystemControl>(controlDefinition.m_name, id, controlDefinition.m_type);
            if (!parent)
            {
                parent = &m_rootControl;
            }

            parent->AddChild(newControl.get());
            newControl->SetParent(parent);
            newControl->SetLocalized(controlDefinition.m_isLocalized);
            m_controls[id] = newControl;
            return newControl.get();
        }
    }

    IAudioSystemControl* CAudioSystemEditor_SoLoud::GetRoot()
    {
        return &m_rootControl;
    }

    IAudioSystemControl* CAudioSystemEditor_SoLoud::GetControl(CID id) const
    {
        auto it = m_controls.find(id);
        if (it != m_controls.end())
            return it->second.get();

        return nullptr;
    }

    EACEControlType CAudioSystemEditor_SoLoud::ImplTypeToATLType(TImplControlType type) const
    {
        switch (type)
        {
            case ESoLoudControlType::AudioFile:
                return eACET_PRELOAD;

            case ESoLoudControlType::GlobalRTPC:
                return eACET_RTPC;
        }

        return eACET_NUM_TYPES;
    }

    TImplControlTypeMask CAudioSystemEditor_SoLoud::GetCompatibleTypes(EACEControlType atlControlType) const
    {
        switch (atlControlType)
        {
            case eACET_TRIGGER:
                return ESoLoudControlType::AudioFile;

            case eACET_PRELOAD:
                return ESoLoudControlType::AudioFile;

            case eACET_RTPC:
                return ESoLoudControlType::GlobalRTPC | ESoLoudControlType::AudioFile;
        }

        return AUDIO_IMPL_INVALID_TYPE;
    }

    TConnectionPtr CAudioSystemEditor_SoLoud::CreateConnectionToControl(EACEControlType atlControlType,
        IAudioSystemControl* middlewareControl)
    {
        if (!middlewareControl)
            return nullptr;

        TConnectionPtr result;

        switch (middlewareControl->GetType())
        {
            case ESoLoudControlType::AudioFile:
            {
                switch (atlControlType)
                {
                    case eACET_TRIGGER:
                        result = AZStd::make_shared<CAudioFileToTriggerConnection>(middlewareControl->GetId());
                        break;

                    case eACET_RTPC:
                        result = AZStd::make_shared<CAudioFileToRtpcConnection>(middlewareControl->GetId());
                        break;

                    case eACET_PRELOAD:
                        result = AZStd::make_shared<IAudioConnection>(middlewareControl->GetId());
                        break;
                }

                break;
            }

            case ESoLoudControlType::GlobalRTPC:
            {
                result = AZStd::make_shared<IAudioConnection>(middlewareControl->GetId());
                break;
            }
        }

        if (result)
        {
            middlewareControl->SetConnected(true);
            ++m_connectionsByID[middlewareControl->GetId()];
        }

        return result;
    }

    TConnectionPtr CAudioSystemEditor_SoLoud::CreateConnectionFromXMLNode(AZ::rapidxml::xml_node<char>* node,
        EACEControlType atlControlType)
    {
        using namespace Audio;

        if (!node)
            return nullptr;

        IAudioSystemControl* control = nullptr;
        TConnectionPtr connection;

        // Connections from AudioFile.
        if (AZ::StringFunc::Equal(node->name(), AudioFileTag))
        {
            const char* controlName = nullptr;
            auto attr = node->first_attribute(AudioFilePathTag);
            if (!attr)
                return nullptr;

            controlName = attr->value();

            control = GetControlByName(controlName);
            if (!control)
            {
                control = CreateControl(SControlDef(controlName, ESoLoudControlType::AudioFile));
                control->SetPlaceholder(true);
            }

            switch (atlControlType)
            {
                case eACET_TRIGGER:
                {
                    auto conn = AZStd::make_shared<CAudioFileToTriggerConnection>(control->GetId());
                    conn->m_params.ReadFromXml(*node);
                    connection = conn;
                    break;
                }

                case eACET_RTPC:
                {
                    auto conn = AZStd::make_shared<CAudioFileToRtpcConnection>(control->GetId());
                    if (!conn->m_params.ReadFromXml(*node))
                        return nullptr;

                    connection = conn;
                    break;
                }

                case eACET_PRELOAD:
                    connection = AZStd::make_shared<IAudioConnection>(control->GetId());
                    break;

                default:
                    AZ_Assert(false, "Impossible branch!");
            }
        }

        // Connections from GlobalRTPC.
        const EGlobalRtpc::Type rtpcType = EGlobalRtpc::FromString(node->name());
        if (rtpcType != EGlobalRtpc::Count)
        {
            const char* controlName = node->name();
            control = GetControlByName(controlName);
            if (!control)
            {
                control = CreateControl(SControlDef(controlName, ESoLoudControlType::GlobalRTPC));
                control->SetPlaceholder(true);
            }

            auto conn = AZStd::make_shared<IAudioConnection>(control->GetId());
            connection = conn;
        }

        if (control)
        {
            control->SetConnected(true);
            ++m_connectionsByID[control->GetId()];
        }

        return connection;
    }

    AZ::rapidxml::xml_node<char>* CAudioSystemEditor_SoLoud::CreateXMLNodeFromConnection(const TConnectionPtr connection,
        const EACEControlType atlControlType)
    {
        using namespace Audio;

        const IAudioSystemControl* control = GetControl(connection->GetID());
        if (!control)
            return nullptr;

        AZ::rapidxml::xml_node<char>* connNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
        const ESoLoudControlType::Type controlType = static_cast<ESoLoudControlType::Type>(control->GetType());

        switch (controlType)
        {
            case ESoLoudControlType::AudioFile:
            {
                connNode->name(AudioFileTag);

                auto attr = xmlAlloc.allocate_attribute(AudioFilePathTag, xmlAlloc.allocate_string(control->GetName().c_str()));
                connNode->append_attribute(attr);

                switch (atlControlType)
                {
                    case eACET_TRIGGER:
                    {
                        const CAudioFileToTriggerConnection* conn = static_cast<const CAudioFileToTriggerConnection*>(connection.get());
                        conn->m_params.WriteToXml(*connNode, xmlAlloc);
                        break;
                    }

                    case eACET_RTPC:
                    {
                        const CAudioFileToRtpcConnection* conn = static_cast<const CAudioFileToRtpcConnection*>(connection.get());
                        conn->m_params.WriteToXml(*connNode, xmlAlloc);
                        break;
                    }

                    case eACET_PRELOAD:
                        break;

                    default:
                        return nullptr;
                }

                break;
            }

            case ESoLoudControlType::GlobalRTPC:
            {
                EGlobalRtpc::Type rtpcType = EGlobalRtpc::FromString(control->GetName().c_str());
                if (rtpcType == EGlobalRtpc::Count)
                    return nullptr;

                connNode->name(xmlAlloc.allocate_string(control->GetName().c_str()));
                break;
            }

            default:
                return nullptr;
        }

        return connNode;
    }

    void CAudioSystemEditor_SoLoud::ConnectionRemoved(IAudioSystemControl* control)
    {
        int connectionCount = m_connectionsByID[control->GetId()] - 1;
        if (connectionCount <= 0)
        {
            connectionCount = 0;
            control->SetConnected(false);
        }
        m_connectionsByID[control->GetId()] = connectionCount;
    }

    const AZStd::string_view CAudioSystemEditor_SoLoud::GetTypeIcon(TImplControlType) const
    {
        return AZStd::string_view();
    }

    const AZStd::string_view CAudioSystemEditor_SoLoud::GetTypeIconSelected(TImplControlType) const
    {
        return AZStd::string_view();
    }

    AZStd::string CAudioSystemEditor_SoLoud::GetName() const
    {
        return "SoLoud";
    }

    AZ::IO::FixedMaxPath CAudioSystemEditor_SoLoud::GetDataPath() const
    {
        auto projectPath = AZ::IO::FixedMaxPath{ AZ::Utils::GetProjectPath() };
        return (projectPath / "sounds" / "soloud");
    }

    IAudioSystemControl* CAudioSystemEditor_SoLoud::GetControlByName(AZStd::string name, bool, IAudioSystemControl* parent) const
    {
        if (parent)
        {
            AZ::StringFunc::Path::Join(parent->GetName().c_str(), name.c_str(), name);
        }

        // TODO: implement localization handling.
        /*if (isLocalized)
        {
            AZ::StringFunc::Path::Join(m_loader.GetLocalizationFolder().c_str(), name.c_str(), name);
        }*/

        return GetControl(GetID(name));
    }

    CID CAudioSystemEditor_SoLoud::GetID(const AZStd::string_view name) const
    {
        return Audio::AudioStringToID<CID>(name.data());
    }

    void CAudioSystemEditor_SoLoud::ScanAudioFilesAndCreateControls(AZStd::string_view folderPath)
    {
        auto foundFiles = Audio::FindFilesInPath(folderPath, "*");

        for (const auto& filePath : foundFiles)
        {
            AZ_Assert(AZ::IO::FileIOBase::GetInstance()->Exists(filePath.c_str()), "FindFiles found file '%s' but FileIO says it doesn't exist!",
                filePath.c_str());

            if (AZ::IO::FileIOBase::GetInstance()->IsDirectory(filePath.c_str()))
                ScanAudioFilesAndCreateControls(filePath.c_str());

            AZStd::string ext;
            AZ::StringFunc::Path::GetExtension(filePath.c_str(), ext, false);

            bool formatSupported = false;
            for (size_t i = 0; i < AZ_ARRAY_SIZE(Audio::AudioFileFormatsArray); ++i)
            {
                if (ext == Audio::AudioFileFormatsArray[i])
                {
                    formatSupported = true;
                    break;
                }
            }

            if (!formatSupported)
                continue;

            AZStd::string localPath = filePath.c_str();

            AZ::StringFunc::Replace(localPath, '\\', '/');
            Audio::EraseSubStr(localPath, Audio::AudioFilesPath);
            if (localPath[0] == '/')
                localPath.erase(0, 1);

            SControlDef controlDef(localPath, ESoLoudControlType::AudioFile);
            CreateControl(controlDef);
        }
    }

    void CAudioSystemEditor_SoLoud::DataSaved()
    {
    }

    QWidget* CAudioSystemEditor_SoLoud::CreateConnectionPropertiesWidget(const TConnectionPtr connection, EACEControlType atlControlType)
    {
        if (!connection)
            return nullptr;

        IAudioSystemControl* control = GetControl(connection->GetID());
        if (!control)
            return nullptr;

        switch (control->GetType())
        {
            case ESoLoudControlType::AudioFile:
            {
                switch (atlControlType)
                {
                    case eACET_TRIGGER:
                        return new CAudioFileToTriggerParamsWidget(connection);

                    case eACET_RTPC:
                        return new CAudioFileToRtpcParamsWidget(connection);

                    default:
                        return nullptr;
                }
            }

            default:
                return nullptr;
        }
    }
} // namespace AudioControls
