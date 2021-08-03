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
    AZ_CVAR_EXTERNED(AZ::u64, s_SoLoud_MemoryPoolSize);
    AZ_CVAR_EXTERNED(AZ::u32, s_SoLoud_SampleRate);
    AZ_CVAR_EXTERNED(AZ::u32, s_SoLoud_Clipper);
    AZ_CVAR_EXTERNED(AZ::u32, s_SoLoud_SpeakerConfiguration);
    AZ_CVAR_EXTERNED(AZ::u32, s_SoLoud_MainResampler);
    AZ_CVAR_EXTERNED(AZ::u32, s_SoLoud_MaxActiveVoiceCount);
    AZ_CVAR_EXTERNED(AZStd::string, s_SoLoud_CurrentBackend);
    AZ_CVAR_EXTERNED(AZ::u32, s_SoLoud_CurrentNumberOfChannels);
}
