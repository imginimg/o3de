/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/Name/Name.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>

#include <AudioEngineSoLoud/AudioEngineSoloud.h>
#include <BusData.h>

namespace Audio
{
    class AudioBusManagerRequests : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        using MutexType = AZStd::mutex;
        static const bool EnableEventQueue = true;

        virtual ~AudioBusManagerRequests() = default;

        virtual void ResetAudioBusLayout() = 0;
        virtual void LoadAudioBusLayout(AZ::IO::FixedMaxPath layoutFilePath) = 0;
        virtual void SaveAudioBusLayout(AZ::IO::FixedMaxPath layoutFilePath) = 0;
        virtual void SetVisualizationEnabled(bool isEnabled) = 0;

        virtual void AddAudioBus(AZ::Name busName) = 0;
        virtual void RemoveAudioBus(AZ::Name busName) = 0;
        virtual void ChangeAudioBusName(AZ::Name busName, AZ::Name newName) = 0;
        virtual void ChangeAudioBusIndex(AZ::Name busName, int32_t newBusIndex) = 0;
        virtual void SetAudioBusVolumeDb(AZ::Name busName, float volume) = 0;
        virtual void SetAudioBusVolumeLinear(AZ::Name busName, float volume) = 0;
        virtual void SetAudioBusMuted(AZ::Name busName, bool isMuted) = 0;
        virtual void SetAudioBusMono(AZ::Name busName, bool isMono) = 0;
        virtual void SetAudioBusOutput(AZ::Name busName, AZ::Name outputBusName) = 0;

        virtual void SetAudioBusFilter(AZ::Name busName, int32_t filterIndex, AZ::Name filterName) = 0;
        virtual void SetAudioBusFilterEnabled(AZ::Name busName, int32_t filterIndex, bool isEnabled) = 0;
        virtual void SetAudioBusFilterParams(AZ::Name busName, int32_t filterIndex, AZStd::vector<float> params) = 0;
        virtual void SetAudioBusFilterParam(AZ::Name busName, int32_t filterIndex, AZ::Name paramName, float paramValue) = 0;

        virtual void RequestAudioBusLayout() = 0;
        virtual void RequestAudioBusNames() = 0;
        virtual void RequestAudioFilters() = 0;
        virtual void RequestAudioBusPeakVolumes() = 0;

        virtual void RegisterAudioFilter(AZ::Name filterName, AudioFilterCreatorFunc filterCreator) = 0;
    };
    using AudioBusManagerRequestBus = AZ::EBus<AudioBusManagerRequests>;

    class AudioBusManagerNotifications : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        using MutexType = AZStd::mutex;
        static const bool EnableEventQueue = true;

        virtual ~AudioBusManagerNotifications() = default;

        virtual void OnRequestCompleted_AddAudioBus([[maybe_unused]] bool success, [[maybe_unused]] BusData busData)
        {
        }

        virtual void OnRequestCompleted_RemoveAudioBus([[maybe_unused]] bool success, [[maybe_unused]] AZ::Name busName)
        {
        }

        virtual void OnRequestCompleted_ChangeAudioBusName(
            [[maybe_unused]] bool success, [[maybe_unused]] AZ::Name oldBusName, [[maybe_unused]] AZ::Name newBusName)
        {
        }

        virtual void OnRequestCompleted_ChangeAudioBusIndex(
            [[maybe_unused]] bool success,
            [[maybe_unused]] AZ::Name busName,
            [[maybe_unused]] int32_t oldBusIndex,
            [[maybe_unused]] int32_t newBusIndex)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusVolumeDb(
            [[maybe_unused]] bool success, [[maybe_unused]] AZ::Name busName, [[maybe_unused]] float volume)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusVolumeLinear(
            [[maybe_unused]] bool success, [[maybe_unused]] AZ::Name busName, [[maybe_unused]] float volume)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusMuted(
            [[maybe_unused]] bool success, [[maybe_unused]] AZ::Name busName, [[maybe_unused]] bool isMuted)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusMono(
            [[maybe_unused]] bool success, [[maybe_unused]] AZ::Name busName, [[maybe_unused]] bool isMono)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusOutput(
            [[maybe_unused]] bool success, [[maybe_unused]] AZ::Name busName, [[maybe_unused]] AZ::Name outputBusName)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusFilter(
            [[maybe_unused]] bool success,
            [[maybe_unused]] AZ::Name busName,
            [[maybe_unused]] int32_t filterIndex,
            [[maybe_unused]] FilterData filterData)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusFilterEnabled(
            [[maybe_unused]] bool success,
            [[maybe_unused]] AZ::Name busName,
            [[maybe_unused]] int32_t filterIndex,
            [[maybe_unused]] bool isEnabled)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusFilterParams(
            [[maybe_unused]] bool success,
            [[maybe_unused]] AZ::Name busName,
            [[maybe_unused]] int32_t filterIndex,
            [[maybe_unused]] AZStd::vector<float> params)
        {
        }

        virtual void OnRequestCompleted_SetAudioBusFilterParam(
            [[maybe_unused]] bool success,
            [[maybe_unused]] AZ::Name busName,
            [[maybe_unused]] int32_t filterIndex,
            [[maybe_unused]] AZ::Name paramName,
            [[maybe_unused]] int32_t paramIndex,
            [[maybe_unused]] float paramValue)
        {
        }

        virtual void OnUpdateAudioBusLayout([[maybe_unused]] BusLayoutData busLayout)
        {
        }

        virtual void OnUpdateAudioBusNames([[maybe_unused]] AZStd::vector<AZ::Name> busNames)
        {
        }

        virtual void OnUpdateAudioFilters([[maybe_unused]] AZStd::vector<FilterData> filterData)
        {
        }

        virtual void OnUpdateAudioBusPeakVolumes([[maybe_unused]] AZ::Name busName, [[maybe_unused]] AZStd::vector<float> channelVolumes)
        {
        }
    };
    using AudioBusManagerNotificationBus = AZ::EBus<AudioBusManagerNotifications>;
} // namespace Audio
