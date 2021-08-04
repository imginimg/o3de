/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/StringFunc/StringFunc.h>
#include <AzCore/std/string/conversions.h>
#include <Common.h>

namespace Audio
{
    AZ::u32 ESpeakerConfiguration::ToChannelCount(Type conf)
    {
        switch (conf)
        {
            case Mono:
                return 1;

            case Stereo:
                return 2;

            case Quad:
                return 4;

            case _5_1:
                return 6;

            case _7_1:
                return 8;

            default:
                return 2;
        }
    }

    const char* EAudioAction::ToString(Type type)
    {
        static const char* strings[EAudioAction::Count] = { "Start", "Stop", "Pause", "Resume" };

        AZ_Assert(type < EAudioAction::Count, "Invalid EAudioAction value!");
        return strings[type];
    }

    EAudioAction::Type EAudioAction::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "Start"))
        {
            return EAudioAction::Start;
        }
        else if (AZ::StringFunc::Equal(str, "Stop"))
        {
            return EAudioAction::Stop;
        }
        else if (AZ::StringFunc::Equal(str, "Pause"))
        {
            return EAudioAction::Pause;
        }
        else if (AZ::StringFunc::Equal(str, "Resume"))
        {
            return EAudioAction::Resume;
        }
        else
        {
            return EAudioAction::Count;
        }
    }

    const char* EAttenuationMode::ToString(Type type)
    {
        static const char* strings[EAudioAction::Count] = { "NoAttenuation", "InverseDistance", "LinearDistance", "ExponentialDistance" };

        AZ_Assert(type < EAudioAction::Count, "Invalid EAttenuationMode value!");
        return strings[type];
    }

    EAttenuationMode::Type EAttenuationMode::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "NoAttenuation"))
        {
            return EAttenuationMode::NoAttenuation;
        }
        else if (AZ::StringFunc::Equal(str, "InverseDistance"))
        {
            return EAttenuationMode::InverseDistance;
        }
        else if (AZ::StringFunc::Equal(str, "LinearDistance"))
        {
            return EAttenuationMode::LinearDistance;
        }
        else if (AZ::StringFunc::Equal(str, "ExponentialDistance"))
        {
            return EAttenuationMode::ExponentialDistance;
        }
        else
        {
            return EAttenuationMode::Count;
        }
    }

    void SAudioFileToTriggerParams::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        auto attr = node.first_attribute(EAudioAction::Tag);
        if (attr)
        {
            m_action = EAudioAction::FromString(attr->value());
            if (m_action == EAudioAction::Count)
                m_action = EAudioAction::Start;
        }

        attr = node.first_attribute(VolumeTag);
        if (attr)
        {
            m_volume = AZStd::stof(AZStd::string(attr->value()));
        }

        attr = node.first_attribute(MinDistanceTag);
        if (attr)
        {
            m_minDistance = AZStd::stof(AZStd::string(attr->value()));
        }

        attr = node.first_attribute(MaxDistanceTag);
        if (attr)
        {
            m_maxDistance = AZStd::stof(AZStd::string(attr->value()));
        }

        attr = node.first_attribute(EAttenuationMode::Tag);
        if (attr)
        {
            m_attenuationMode = EAttenuationMode::FromString(attr->value());
            if (m_attenuationMode == EAttenuationMode::Count)
                m_attenuationMode = EAttenuationMode::NoAttenuation;
        }

        attr = node.first_attribute(AttenuationRolloffFactorTag);
        if (attr)
        {
            m_attenuationRolloffFactor = AZStd::stof(AZStd::string(attr->value()));
        }

        attr = node.first_attribute(PositionalTag);
        if (attr)
        {
            m_positional = AZStd::stoi(AZStd::string(attr->value()));
        }

        attr = node.first_attribute(LoopingTag);
        if (attr)
        {
            m_looping = AZStd::stoi(AZStd::string(attr->value()));
        }
    }

    void SAudioFileToTriggerParams::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        auto attr = xmlAlloc.allocate_attribute(EAudioAction::Tag, EAudioAction::ToString(m_action));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(VolumeTag, xmlAlloc.allocate_string(AZStd::to_string(m_volume).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(MinDistanceTag, xmlAlloc.allocate_string(AZStd::to_string(m_minDistance).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(MaxDistanceTag, xmlAlloc.allocate_string(AZStd::to_string(m_maxDistance).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(EAttenuationMode::Tag, EAttenuationMode::ToString(m_attenuationMode));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(AttenuationRolloffFactorTag, xmlAlloc.allocate_string(AZStd::to_string(m_attenuationRolloffFactor).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(PositionalTag, xmlAlloc.allocate_string(AZStd::to_string(m_positional).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(LoopingTag, xmlAlloc.allocate_string(AZStd::to_string(m_looping).c_str()));
        node.append_attribute(attr);
    }

    const char* EAudioFileRtpc::ToString(Type type)
    {
        constexpr static const char* strings[EAudioAction::Count] = { "Volume", "PlaySpeed", "Seek" };

        AZ_Assert(type < EAudioFileRtpc::Count, "Invalid EAudioFileRtpc value!");
        return strings[type];
    }

    EAudioFileRtpc::Type EAudioFileRtpc::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "Volume"))
        {
            return EAudioFileRtpc::Volume;
        }
        else if (AZ::StringFunc::Equal(str, "PlaySpeed"))
        {
            return EAudioFileRtpc::PlaySpeed;
        }
        else if (AZ::StringFunc::Equal(str, "Seek"))
        {
            return EAudioFileRtpc::Seek;
        }
        else
        {
            return EAudioFileRtpc::Count;
        }
    }

    bool SAudioFileToRtpcParams::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        auto attr = node.first_attribute(EAudioFileRtpc::Tag);
        if (attr)
        {
            m_type = EAudioFileRtpc::FromString(attr->value());
            if (m_type == EAudioAction::Count)
                return false;
        }

        attr = node.first_attribute(PerObjectTag);
        if (attr)
        {
            m_perObject = AZStd::stoi(AZStd::string(attr->value()));
        }

        return true;
    }

    void SAudioFileToRtpcParams::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        auto attr = xmlAlloc.allocate_attribute(EAudioFileRtpc::Tag, EAudioFileRtpc::ToString(m_type));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(PerObjectTag, xmlAlloc.allocate_string(AZStd::to_string(m_perObject).c_str()));
        node.append_attribute(attr);
    }

    const char* EGlobalRtpc::ToString(Type type)
    {
        static const char* strings[EAudioAction::Count] = { "GlobalVolume" };

        AZ_Assert(type < EGlobalRtpc::Count, "Invalid EGlobalRtpc value!");
        return strings[type];
    }

    EGlobalRtpc::Type EGlobalRtpc::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "GlobalVolume"))
        {
            return EGlobalRtpc::GlobalVolume;
        }
        else
        {
            return EGlobalRtpc::Count;
        }
    }

    void EraseSubStr(AZStd::string& inOutStr, AZStd::string_view strToErase)
    {
        auto pos = inOutStr.find(strToErase);
        if (pos != AZStd::string::npos)
            inOutStr.erase(pos, strToErase.length());
    }
} // namespace Audio
