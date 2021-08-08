/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Console/IConsole.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <IAudioConnection.h>
#include <IAudioInterfacesCommonData.h>
#include <IAudioSystemControl.h>
#include <IAudioSystemEditor.h>

namespace AudioControls
{
    class CAudioSystemEditor_SoLoud : public IAudioSystemEditor
    {
    public:
        CAudioSystemEditor_SoLoud();
        ~CAudioSystemEditor_SoLoud() = default;

        // IAudioSystemEditor
        void Reload() override;
        IAudioSystemControl* CreateControl(const SControlDef& controlDefinition) override;
        IAudioSystemControl* GetRoot() override;
        IAudioSystemControl* GetControl(CID id) const override;
        EACEControlType ImplTypeToATLType(TImplControlType type) const override;
        TImplControlTypeMask GetCompatibleTypes(EACEControlType atlControlType) const override;
        TConnectionPtr CreateConnectionToControl(EACEControlType atlControlType, IAudioSystemControl* middlewareControl) override;
        TConnectionPtr CreateConnectionFromXMLNode(AZ::rapidxml::xml_node<char>* node, EACEControlType atlControlType) override;
        AZ::rapidxml::xml_node<char>* CreateXMLNodeFromConnection(const TConnectionPtr connection, const EACEControlType atlControlType) override;
        void ConnectionRemoved(IAudioSystemControl* middlewareControl) override;
        const AZStd::string_view GetTypeIcon(TImplControlType type) const override;
        const AZStd::string_view GetTypeIconSelected(TImplControlType type) const override;
        AZStd::string GetName() const override;
        AZ::IO::FixedMaxPath GetDataPath() const override;
        void DataSaved() override;
        QWidget* CreateConnectionPropertiesWidget(const TConnectionPtr connection, EACEControlType atlControlType) override;
        // ~IAudioSystemEditor

    private:
        IAudioSystemControl* GetControlByName(AZStd::string name, bool isLocalized = false) const;
        CID GetID(const AZStd::string_view name) const;
        void ScanAudioFilesAndCreateControlsRecursive(AZ::IO::PathView dirPathToScan, bool localized = false);


        AZStd::string m_currentLanguageName;
        IAudioSystemControl m_rootControl;
        IAudioSystemControl m_localizedParentControl;

        using TControlPtr = AZStd::shared_ptr<IAudioSystemControl>;
        using TControlMap = AZStd::unordered_map<CID, TControlPtr>;
        TControlMap m_controls;

        using TConnectionsMap = AZStd::unordered_map<CID, int>;
        TConnectionsMap m_connectionsByID;

        AZ::rapidxml::memory_pool<> m_xmlAlloc;
    };
} // namespace AudioControls
