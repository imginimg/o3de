/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AudioEngineSoLoud/AudioEngineSoLoud.h>

#include <AzCore/RTTI/BehaviorContext.h>

namespace Audio
{
    class AudioEngineSoLoudRequestHandler : public AudioEngineSoLoudRequestBus::Handler
    {
    public:
        AudioEngineSoLoudRequestHandler();
        ~AudioEngineSoLoudRequestHandler();

        static void Reflect(AZ::ReflectContext* context);

        // AudioEngineSoLoudRequestBus
        void LoadAudioBusLayout(AZStd::string layoutFilePath) override;
        void SetAudioBusVolumeDb(AZStd::string busName, float volume) override;
        void SetAudioBusVolumeLinear(AZStd::string busName, float volume) override;
        void SetAudioBusMuted(AZStd::string busName, bool isMuted) override;
        void SetAudioBusMono(AZStd::string busName, bool isMono) override;
        void SetAudioBusFilter(AZStd::string busName, int32_t filterIndex, AZStd::string filterName) override;
        void SetAudioBusFilterEnabled(AZStd::string busName, int32_t filterIndex, bool isEnabled) override;
        void SetAudioBusFilterParam(AZStd::string busName, int32_t filterIndex, AZStd::string paramName, float paramValue) override;
        void RegisterAudioFilter(AZ::Name filterName, AudioFilterCreatorFunc filterCreator) override;
        // ~AudioEngineSoLoudRequestBus
    };
} // namespace Audio
