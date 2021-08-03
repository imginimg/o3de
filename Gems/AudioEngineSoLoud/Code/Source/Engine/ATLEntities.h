/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <ATLEntityData.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/string/string.h>
#include <Common.h>
#include <soloud.h>

namespace Audio
{
    struct SATLEventDataSoLoud : public IATLEventData
    {
        SoLoud::handle m_soloudHandle;
        bool m_isPlayEvent = false;
    };

    struct SATLAudioFileEntryDataSoLoud : public IATLAudioFileEntryData
    {
        AZStd::string m_fullFilePath;
    };

    struct SATLTriggerImplDataSoLoud : public IATLTriggerImplData
    {
        AZStd::string m_audioFilePath;
        SAudioFileToTriggerParams m_audioFileToTriggerParams;
    };

    struct SATLRtpcImplDataSoLoud : public IATLRtpcImplData
    {
        ERtpcType::Type m_type = ERtpcType::Count;
        SRtpcParams m_params;
    };

    struct SATLAudioObjectDataSoLoud : public IATLAudioObjectData
    {
        SATLWorldPosition m_pos;

        // Key - audio file path, same as CAudioSystemImpl_SoLoud::m_audioSources.
        AZStd::unordered_multimap<AZStd::string, SoLoud::handle> m_activeSoVoices;
    };
} // namespace Audio
