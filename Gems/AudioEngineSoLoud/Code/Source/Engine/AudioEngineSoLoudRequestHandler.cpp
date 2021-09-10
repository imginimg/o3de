/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioEngineSoLoudRequestHandler.h>
#include <EditorEngineInterop.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Audio
{
    void AudioEngineSoLoudRequestHandler::Reflect(AZ::ReflectContext* context)
    {
        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<AudioEngineSoLoudRequestBus>("AudioEngineSoLoudRequestBus")
                ->Event("LoadAudioBusLayout", &AudioEngineSoLoudRequestBus::Events::LoadAudioBusLayout)
                ->Event("SetAudioBusVolumeDb", &AudioEngineSoLoudRequestBus::Events::SetAudioBusVolumeDb)
                ->Event("SetAudioBusVolumeLinear", &AudioEngineSoLoudRequestBus::Events::SetAudioBusVolumeLinear)
                ->Event("SetAudioBusMuted", &AudioEngineSoLoudRequestBus::Events::SetAudioBusMuted)
                ->Event("SetAudioBusMono", &AudioEngineSoLoudRequestBus::Events::SetAudioBusMono)
                ->Event("SetAudioBusFilter", &AudioEngineSoLoudRequestBus::Events::SetAudioBusFilter)
                ->Event("SetAudioBusFilterEnabled", &AudioEngineSoLoudRequestBus::Events::SetAudioBusFilterEnabled)
                ->Event("SetAudioBusFilterParam", &AudioEngineSoLoudRequestBus::Events::SetAudioBusFilterParam);
        }
    }

    AudioEngineSoLoudRequestHandler::AudioEngineSoLoudRequestHandler()
    {
        AudioEngineSoLoudRequestBus::Handler::BusConnect();
    }

    AudioEngineSoLoudRequestHandler::~AudioEngineSoLoudRequestHandler()
    {
        AudioEngineSoLoudRequestBus::Handler::BusDisconnect();
    }

    void AudioEngineSoLoudRequestHandler::LoadAudioBusLayout(AZStd::string layoutFilePath)
    {
        AZ::IO::FixedMaxPath path = layoutFilePath.c_str();
        AudioBusManagerRequestBus::QueueBroadcast(&AudioBusManagerRequestBus::Events::LoadAudioBusLayout, path);
    }

    void AudioEngineSoLoudRequestHandler::SetAudioBusVolumeDb(AZStd::string busName, float volume)
    {
        AudioBusManagerRequestBus::QueueBroadcast(&AudioBusManagerRequestBus::Events::SetAudioBusVolumeDb, AZ::Name(busName), volume);
    }

    void AudioEngineSoLoudRequestHandler::SetAudioBusVolumeLinear(AZStd::string busName, float volume)
    {
        AudioBusManagerRequestBus::QueueBroadcast(&AudioBusManagerRequestBus::Events::SetAudioBusVolumeLinear, AZ::Name(busName), volume);
    }

    void AudioEngineSoLoudRequestHandler::SetAudioBusMuted(AZStd::string busName, bool isMuted)
    {
        AudioBusManagerRequestBus::QueueBroadcast(&AudioBusManagerRequestBus::Events::SetAudioBusMuted, AZ::Name(busName), isMuted);
    }

    void AudioEngineSoLoudRequestHandler::SetAudioBusMono(AZStd::string busName, bool isMono)
    {
        AudioBusManagerRequestBus::QueueBroadcast(&AudioBusManagerRequestBus::Events::SetAudioBusMono, AZ::Name(busName), isMono);
    }

    void AudioEngineSoLoudRequestHandler::SetAudioBusFilter(AZStd::string busName, int32_t filterIndex, AZStd::string filterName)
    {
        AudioBusManagerRequestBus::QueueBroadcast(
            &AudioBusManagerRequestBus::Events::SetAudioBusFilter, AZ::Name(busName), filterIndex, AZ::Name(filterName));
    }

    void AudioEngineSoLoudRequestHandler::SetAudioBusFilterEnabled(AZStd::string busName, int32_t filterIndex, bool isEnabled)
    {
        AudioBusManagerRequestBus::QueueBroadcast(
            &AudioBusManagerRequestBus::Events::SetAudioBusFilterEnabled, AZ::Name(busName), filterIndex, isEnabled);
    }

    void AudioEngineSoLoudRequestHandler::SetAudioBusFilterParam(
        AZStd::string busName, int32_t filterIndex, AZStd::string paramName, float paramValue)
    {
        AudioBusManagerRequestBus::QueueBroadcast(
            &AudioBusManagerRequestBus::Events::SetAudioBusFilterParam, AZ::Name(busName), filterIndex, AZ::Name(paramName), paramValue);
    }

    void AudioEngineSoLoudRequestHandler::RegisterAudioFilter(AZ::Name filterName, AudioFilterCreatorFunc filterCreator)
    {
        AudioBusManagerRequestBus::QueueBroadcast(&AudioBusManagerRequestBus::Events::RegisterAudioFilter, filterName, filterCreator);
    }
} // namespace Audio
