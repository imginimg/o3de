/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioEngineSoLoud_Traits_Platform.h>
#include <AudioSystemImplCVars.h>

namespace Audio::CVars
{
    AZ_CVAR(
        uint64_t,
        s_SoLoud_MemoryPoolSize,
        AZ_TRAIT_AUDIOENGINESOLOUD_POOL_SIZE,
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "The size in KiB of the memory pool used by the SoLoud audio integration.\n");

    AZ_CVAR(
        uint32_t,
        s_SoLoud_Clipper,
        1,
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "The clipping mode used by the SoLoud audio integration.\n"
        "Valid values: 0 - \"hard\" clipping to -1/+1, 1 - roundoff clipper\n");

    AZ_CVAR(
        float,
        s_SoLoud_PostClipScaler,
        0.95f,
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "The post-clip scaler used by the SoLoud audio integration.\n");

    AZ_CVAR(
        uint32_t,
        s_SoLoud_SampleRate,
        44100,
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "The sample rate used by the SoLoud audio integration.\n");

    AZ_CVAR(
        uint32_t,
        s_SoLoud_SpeakerConfiguration,
        1,
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "The speaker configuration used by the SoLoud audio integration.\n"
        "Valid values: 0 - Mono, 1 - Stereo, 2 - Quad, 3 - 5.1, 4 - 7.1.\n");

    AZ_CVAR(
        uint32_t,
        s_SoLoud_MainResampler,
        1,
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "The main resampler used by the SoLoud audio integration.\n"
        "Valid values: 0 - Point, 1 - Linear, 2 - Catmull-Rom.\n");

    AZ_CVAR(
        uint32_t,
        s_SoLoud_MaxActiveVoiceCount,
        16,
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "The number of audible voices SoLoud will play simultaneously.\n"
        "Valid values between 1 and 1023 inclusive.\n");

    AZ_CVAR(
        AZStd::string,
        s_SoLoud_CurrentBackend,
        "",
        nullptr,
        AZ::ConsoleFunctorFlags::ReadOnly,
        "The current backend used by the SoLoud audio integration.\n");

    AZ_CVAR(
        uint32_t,
        s_SoLoud_CurrentNumberOfChannels,
        0,
        nullptr,
        AZ::ConsoleFunctorFlags::ReadOnly,
        "The current number of channels used by the SoLoud backend.\n");
} // namespace Audio::CVars
