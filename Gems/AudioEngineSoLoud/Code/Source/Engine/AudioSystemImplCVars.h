/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Console/IConsole.h>

namespace Audio::CVars
{
    AZ_CVAR_EXTERNED(uint64_t, s_SoLoud_MemoryPoolSize);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_Clipper);
    AZ_CVAR_EXTERNED(float, s_SoLoud_PostClipScaler);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_SampleRate);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_SpeakerConfiguration);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_MainResampler);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_MaxActiveVoiceCount);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_BufferSize);

    AZ_CVAR_EXTERNED(AZStd::string, s_SoLoud_Backend);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_BackendSampleRate);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_BackendNumberOfChannels);
    AZ_CVAR_EXTERNED(uint32_t, s_SoLoud_BackendBufferSize);
} // namespace Audio::CVars
