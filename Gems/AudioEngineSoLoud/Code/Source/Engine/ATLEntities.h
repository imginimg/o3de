/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <ATLEntityData.h>
#include <AzCore/XML/rapidxml.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/string/string.h>
#include <Common.h>
#include <soloud.h>

namespace Audio
{
    struct AtlEventDataSoLoud : public IATLEventData
    {
        SoLoud::handle m_soloudHandle;
        bool m_isPlayEvent = false;
    };

    struct AtlAudioFileEntryDataSoLoud : public IATLAudioFileEntryData
    {
        AZStd::string m_fullFilePath;
    };

    struct AtlTriggerImplDataSoLoud : public IATLTriggerImplData
    {
        AZStd::string m_audioFilePath;
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
        RtpcImpl::Type m_type = RtpcImpl::Global;

        struct
        {
            GlobalRtpc::Type m_type = GlobalRtpc::Count;
        } m_global;

        struct
        {
            AZStd::string m_audioFilePath;
            AudioFileToRtpcParams m_params;
        } m_audioFile;

        bool ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
    };

    struct ActiveSoVoiceData
    {
        SoLoud::handle m_handle;
        float m_volume = 1.0f;
    };

    struct AtlAudioObjectDataSoLoud : public IATLAudioObjectData
    {
        AZ::Vector3 m_position;

        // Key - audio file path, same as AudioSystemImpl_SoLoud::m_audioSources.
        AZStd::unordered_multimap<AZStd::string, ActiveSoVoiceData> m_activeSoVoices;
    };
} // namespace Audio
