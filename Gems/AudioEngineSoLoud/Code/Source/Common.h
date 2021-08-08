/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/XML/rapidxml.h>
#include <AzCore/base.h>
#include <AzCore/std/string/string.h>
#include <AzCore/IO/Path/Path.h>

namespace Audio
{
    constexpr static const char* LogWindow = "Audio SoLoud";

    constexpr static const char ControlNamePathSeparator = '/';

    constexpr static const auto AudioFilesPath = AZ::IO::FixedMaxPath("sounds/soloud").LexicallyNormal();
    constexpr static const char* LocalizationDirName = "localization";
    constexpr static const char* AudioFileFormatsArray[] = { "wav", "mp3", "ogg", "flac" };

    constexpr static const char* AudioFileTag = "AudioFile";
    constexpr static const char* AudioFilePathTag = "AudioFilePath";
    constexpr static const char* AudioFileLocalizedTag = "Localized";

    //------------------------------------
    namespace SpeakerConfiguration
    {
        enum Type
        {
            Mono,
            Stereo,
            Quad,
            _5_1,
            _7_1
        };

        AZ::u32 ToChannelCount(Type conf);
    } // namespace SpeakerConfiguration

    //------------------------------------
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

    struct AudioFileToTriggerParams
    {
        AudioAction::Type m_action = AudioAction::Start;
        float m_volume = 1.0f;
        float m_minDistance = 0.1f, m_maxDistance = 10.0f;
        AttenuationMode::Type m_attenuationMode = AttenuationMode::NoAttenuation;
        float m_attenuationRolloffFactor = 0.1f;
        bool m_positional = false;
        bool m_looping = false;

        constexpr static char* VolumeTag = "Volume";
        constexpr static char* MinDistanceTag = "MinDistance";
        constexpr static char* MaxDistanceTag = "MaxDistance";
        constexpr static char* AttenuationRolloffFactorTag = "AttenuationRolloffFactor";
        constexpr static char* PositionalTag = "Positional";
        constexpr static char* LoopingTag = "Looping";

        void ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;
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
    }

    struct AudioFileToRtpcParams
    {
        AudioFileRtpc::Type m_type = AudioFileRtpc::Volume;
        bool m_perObject = false;

        constexpr static char* PerObjectTag = "PerObject";

        bool ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;
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

    //------------------------------------
    void EraseSubStr(AZStd::string& str, AZStd::string_view strToErase);
} // namespace Audio
