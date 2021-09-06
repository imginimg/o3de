/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <ATLEntityData.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/XML/rapidxml.h>
#include <AzCore/std/containers/unordered_map.h>

#include <AtlData.h>
#include <Util.h>
#include <soloud.h>

namespace Audio
{
    struct AtlEventDataSoLoud : public IATLEventData
    {
        AtlEventDataSoLoud() = default;
        ~AtlEventDataSoLoud() = default;

        SoLoud::handle m_soloudHandle;
        bool m_isPlayEvent = false;
    };

    struct AtlAudioFileEntryDataSoLoud : public IATLAudioFileEntryData
    {
        AtlAudioFileEntryDataSoLoud() = default;
        ~AtlAudioFileEntryDataSoLoud() = default;

        AZ::IO::FixedMaxPath m_fullFilePath;
    };

    struct AtlTriggerImplDataSoLoud : public IATLTriggerImplData
    {
        AtlTriggerImplDataSoLoud() = default;
        ~AtlTriggerImplDataSoLoud() = default;

        AZ::IO::FixedMaxPath m_audioFilePath;
        AudioFileToTriggerParams m_audioFileToTriggerParams;
    };

    namespace RtpcImpl
    {
        enum Type
        {
            Global,
            AudioFile
        };
    }

    struct AtlRtpcImplDataSoLoud : public IATLRtpcImplData
    {
        AtlRtpcImplDataSoLoud() = default;
        ~AtlRtpcImplDataSoLoud() = default;

        bool ReadFromXml(const AZ::rapidxml::xml_node<char>& node);

        RtpcImpl::Type m_type = RtpcImpl::Global;

        struct
        {
            GlobalRtpc::Type m_type = GlobalRtpc::Count;
        } m_global;

        struct
        {
            AZ::IO::FixedMaxPath m_audioFilePath;
            AudioFileToRtpcParams m_params;
        } m_audioFile;
    };

    struct ActiveSoVoiceData
    {
        ActiveSoVoiceData() = default;
        ~ActiveSoVoiceData() = default;

        SoLoud::handle m_handle;
        float m_volume = 0.0f;
    };

    struct AtlAudioObjectDataSoLoud : public IATLAudioObjectData
    {
        AtlAudioObjectDataSoLoud() = default;
        ~AtlAudioObjectDataSoLoud() = default;

        AZ::Vector3 m_position;

        // Key - audio file path, same as AudioSystemImpl_SoLoud::m_audioSources.
        AZStd::unordered_multimap<AZ::IO::FixedMaxPath, ActiveSoVoiceData> m_activeSoVoices;
    };
} // namespace Audio
