/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Name/Name.h>
#include <AzCore/XML/rapidxml.h>

#include <BusData.h>

namespace Audio
{
    constexpr const char* AudioFileTag = "AudioFile";
    constexpr const char* AudioFilePathTag = "AudioFilePath";
    constexpr const char* AudioFileLocalizedTag = "Localized";

    namespace AudioAction
    {
        enum Type
        {
            Start,
            Stop,
            Pause,
            Resume,

            Count
        };

        constexpr static const char* Tag = "AudioAction";

        const char* ToString(Type type);
        AudioAction::Type FromString(const char* str);
    } // namespace AudioAction

    namespace AttenuationMode
    {
        // Must be in sync with SoLoud::AudioSource::ATTENUATION_MODELS.
        enum Type
        {
            NoAttenuation,
            InverseDistance,
            LinearDistance,
            ExponentialDistance,

            Count
        };

        constexpr static const char* Tag = "AttenuationMode";

        const char* ToString(Type type);
        Type FromString(const char* str);
    } // namespace AttenuationMode

    namespace InaudibleBehavior
    {
        enum Type
        {
            Pause,
            Tick,
            Kill,

            Count
        };

        constexpr static const char* Tag = "InaudibleBehavior";

        const char* ToString(Type type);
        Type FromString(const char* str);
    } // namespace InaudibleBehavior

    struct AudioFileToTriggerParams
    {
        AudioFileToTriggerParams() = default;
        ~AudioFileToTriggerParams() = default;

        void ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;

        constexpr static const char* AudioBusNameTag = "AudioBusName";
        constexpr static const char* VolumeTag = "Volume";
        constexpr static const char* MinDistanceTag = "MinDistance";
        constexpr static const char* MaxDistanceTag = "MaxDistance";
        constexpr static const char* AttenuationRolloffFactorTag = "AttenuationRolloffFactor";
        constexpr static const char* PositionalTag = "Positional";
        constexpr static const char* LoopingTag = "Looping";
        constexpr static const char* ProtectedTag = "Protected";
        constexpr static const char* PlaySpeedTag = "PlaySpeed";

        AZ::Name m_audioBusName = AZ::Name(MasterBusName);
        FilterBlockData m_filterBlock;
        AudioAction::Type m_action = AudioAction::Start;
        AttenuationMode::Type m_attenuationMode = AttenuationMode::NoAttenuation;
        InaudibleBehavior::Type m_inaudibleBehavior = InaudibleBehavior::Pause;
        float m_volume = 0.0f;
        float m_playSpeed = 1.0f;
        float m_minDistance = 0.1f;
        float m_maxDistance = 10.0f;
        float m_attenuationRolloffFactor = 0.1f;
        bool m_positional = false;
        bool m_looping = false;
        bool m_protected = false;
    };

    namespace AudioFileRtpc
    {
        enum Type
        {
            Volume,
            PlaySpeed,
            Seek,

            Count
        };

        constexpr static const char* Tag = "AudioFileRtpcType";

        const char* ToString(Type type);
        Type FromString(const char* str);
    } // namespace AudioFileRtpc

    struct AudioFileToRtpcParams
    {
        AudioFileToRtpcParams() = default;
        ~AudioFileToRtpcParams() = default;

        bool ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;

        constexpr static const char* PerObjectTag = "PerObject";

        AudioFileRtpc::Type m_type = AudioFileRtpc::Volume;
        bool m_perObject = false;
    };

    namespace GlobalRtpc
    {
        enum Type
        {
            GlobalVolume,

            Count
        };

        constexpr static const char* Tag = "GlobalRtpcType";

        const char* ToString(Type type);
        GlobalRtpc::Type FromString(const char* str);
    } // namespace GlobalRtpc
} // namespace Audio
