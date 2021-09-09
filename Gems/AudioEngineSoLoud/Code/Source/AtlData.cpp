/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AtlData.h>

#include <AzCore/StringFunc/StringFunc.h>
#include <AzCore/std/string/conversions.h>

namespace Audio
{
    const char* AudioAction::ToString(Type type)
    {
        static const char* strings[AudioAction::Count] = { "Start", "Stop", "Pause", "Resume" };

        AZ_Assert(type < AudioAction::Count, "Invalid AudioAction value!");
        return strings[type];
    }

    AudioAction::Type AudioAction::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "Start"))
        {
            return AudioAction::Start;
        }
        else if (AZ::StringFunc::Equal(str, "Stop"))
        {
            return AudioAction::Stop;
        }
        else if (AZ::StringFunc::Equal(str, "Pause"))
        {
            return AudioAction::Pause;
        }
        else if (AZ::StringFunc::Equal(str, "Resume"))
        {
            return AudioAction::Resume;
        }
        else
        {
            return AudioAction::Count;
        }
    }

    const char* AttenuationMode::ToString(Type type)
    {
        static const char* strings[AttenuationMode::Count] = { "NoAttenuation", "InverseDistance", "LinearDistance",
                                                               "ExponentialDistance" };

        AZ_Assert(type < AttenuationMode::Count, "Invalid AttenuationMode value!");
        return strings[type];
    }

    AttenuationMode::Type AttenuationMode::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "NoAttenuation"))
        {
            return AttenuationMode::NoAttenuation;
        }
        else if (AZ::StringFunc::Equal(str, "InverseDistance"))
        {
            return AttenuationMode::InverseDistance;
        }
        else if (AZ::StringFunc::Equal(str, "LinearDistance"))
        {
            return AttenuationMode::LinearDistance;
        }
        else if (AZ::StringFunc::Equal(str, "ExponentialDistance"))
        {
            return AttenuationMode::ExponentialDistance;
        }
        else
        {
            return AttenuationMode::Count;
        }
    }

    const char* InaudibleBehavior::ToString(Type type)
    {
        static const char* strings[InaudibleBehavior::Count] = { "Pause", "Tick", "Kill" };

        AZ_Assert(type < InaudibleBehavior::Count, "Invalid InaudibleBehavior value!");
        return strings[type];
    }

    InaudibleBehavior::Type InaudibleBehavior::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "Pause"))
        {
            return InaudibleBehavior::Pause;
        }
        else if (AZ::StringFunc::Equal(str, "Tick"))
        {
            return InaudibleBehavior::Tick;
        }
        else if (AZ::StringFunc::Equal(str, "Kill"))
        {
            return InaudibleBehavior::Kill;
        }
        else
        {
            return InaudibleBehavior::Count;
        }
    }

    void AudioFileToTriggerParams::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        auto attr = node.first_attribute(AudioBusNameTag);
        if (attr)
        {
            m_audioBusName = attr->value();
        }

        attr = node.first_attribute(AudioAction::Tag);
        if (attr)
        {
            m_action = AudioAction::FromString(attr->value());
            if (m_action == AudioAction::Count)
            {
                m_action = AudioAction::Start;
            }
        }

        attr = node.first_attribute(VolumeTag);
        if (attr)
        {
            m_volume = AZ::StringFunc::ToFloat(attr->value());
        }

        attr = node.first_attribute(MinDistanceTag);
        if (attr)
        {
            m_minDistance = AZ::StringFunc::ToFloat(attr->value());
        }

        attr = node.first_attribute(MaxDistanceTag);
        if (attr)
        {
            m_maxDistance = AZ::StringFunc::ToFloat(attr->value());
        }

        attr = node.first_attribute(AttenuationMode::Tag);
        if (attr)
        {
            m_attenuationMode = AttenuationMode::FromString(attr->value());
            if (m_attenuationMode == AttenuationMode::Count)
            {
                m_attenuationMode = AttenuationMode::NoAttenuation;
            }
        }

        attr = node.first_attribute(AttenuationRolloffFactorTag);
        if (attr)
        {
            m_attenuationRolloffFactor = AZ::StringFunc::ToFloat(attr->value());
        }

        attr = node.first_attribute(PositionalTag);
        if (attr)
        {
            m_positional = AZ::StringFunc::ToBool(attr->value());
        }

        attr = node.first_attribute(LoopingTag);
        if (attr)
        {
            m_looping = AZ::StringFunc::ToBool(attr->value());
        }

        attr = node.first_attribute(ProtectedTag);
        if (attr)
        {
            m_protected = AZ::StringFunc::ToBool(attr->value());
        }

        attr = node.first_attribute(InaudibleBehavior::Tag);
        if (attr)
        {
            m_inaudibleBehavior = InaudibleBehavior::FromString(attr->value());
            if (m_inaudibleBehavior == InaudibleBehavior::Count)
            {
                m_inaudibleBehavior = InaudibleBehavior::Pause;
            }
        }

        attr = node.first_attribute(PlaySpeedTag);
        if (attr)
        {
            m_playSpeed = AZ::StringFunc::ToFloat(attr->value());
        }

        auto blockNode = node.first_node(FilterBlockData::Tag);
        if (blockNode)
        {
            m_filterBlock.ReadFromXml(*blockNode);
        }
    }

    void AudioFileToTriggerParams::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        auto attr = xmlAlloc.allocate_attribute(AudioBusNameTag, m_audioBusName.GetCStr());
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(AudioAction::Tag, AudioAction::ToString(m_action));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(VolumeTag, xmlAlloc.allocate_string(AZStd::to_string(m_volume).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(MinDistanceTag, xmlAlloc.allocate_string(AZStd::to_string(m_minDistance).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(MaxDistanceTag, xmlAlloc.allocate_string(AZStd::to_string(m_maxDistance).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(AttenuationMode::Tag, AttenuationMode::ToString(m_attenuationMode));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(
            AttenuationRolloffFactorTag, xmlAlloc.allocate_string(AZStd::to_string(m_attenuationRolloffFactor).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(PositionalTag, xmlAlloc.allocate_string(AZStd::to_string(m_positional).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(LoopingTag, xmlAlloc.allocate_string(AZStd::to_string(m_looping).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(ProtectedTag, xmlAlloc.allocate_string(AZStd::to_string(m_protected).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(InaudibleBehavior::Tag, InaudibleBehavior::ToString(m_inaudibleBehavior));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(PlaySpeedTag, xmlAlloc.allocate_string(AZStd::to_string(m_playSpeed).c_str()));
        node.append_attribute(attr);

        auto blockNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
        m_filterBlock.WriteToXml(*blockNode, xmlAlloc);
        node.append_node(blockNode);
    }

    const char* AudioFileRtpc::ToString(Type type)
    {
        constexpr static const char* strings[AudioFileRtpc::Count] = { "Volume", "PlaySpeed", "Seek" };

        AZ_Assert(type < AudioFileRtpc::Count, "Invalid AudioFileRtpc value!");
        return strings[type];
    }

    AudioFileRtpc::Type AudioFileRtpc::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "Volume"))
        {
            return AudioFileRtpc::Volume;
        }
        else if (AZ::StringFunc::Equal(str, "PlaySpeed"))
        {
            return AudioFileRtpc::PlaySpeed;
        }
        else if (AZ::StringFunc::Equal(str, "Seek"))
        {
            return AudioFileRtpc::Seek;
        }
        else
        {
            return AudioFileRtpc::Count;
        }
    }

    bool AudioFileToRtpcParams::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        auto attr = node.first_attribute(AudioFileRtpc::Tag);
        if (attr)
        {
            m_type = AudioFileRtpc::FromString(attr->value());
            if (m_type == AudioFileRtpc::Count)
            {
                return false;
            }
        }

        attr = node.first_attribute(PerObjectTag);
        if (attr)
        {
            m_perObject = AZ::StringFunc::ToBool(attr->value());
        }

        return true;
    }

    void AudioFileToRtpcParams::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        auto attr = xmlAlloc.allocate_attribute(AudioFileRtpc::Tag, AudioFileRtpc::ToString(m_type));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(PerObjectTag, xmlAlloc.allocate_string(AZStd::to_string(m_perObject).c_str()));
        node.append_attribute(attr);
    }

    const char* GlobalRtpc::ToString(Type type)
    {
        static const char* strings[GlobalRtpc::Count] = { "GlobalVolume" };

        AZ_Assert(type < GlobalRtpc::Count, "Invalid GlobalRtpc value!");
        return strings[type];
    }

    GlobalRtpc::Type GlobalRtpc::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "GlobalVolume"))
        {
            return GlobalRtpc::GlobalVolume;
        }
        else
        {
            return GlobalRtpc::Count;
        }
    }
} // namespace Audio

