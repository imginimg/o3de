/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Name/Name.h>
#include <AzCore/std/functional.h>
#include <AzCore/std/string/string.h>

namespace SoLoud
{
    class Filter;
}

namespace Audio
{
    using AudioFilterCreatorFunc = AZStd::function<SoLoud::Filter*()>;

    class AudioEngineSoLoudRequests : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        using MutexType = AZStd::mutex;
        static const bool EnableEventQueue = true;

        virtual ~AudioEngineSoLoudRequests() = default;

        virtual void LoadAudioBusLayout(AZStd::string layoutFilePath) = 0;
        virtual void SetAudioBusVolumeDb(AZStd::string busName, float volume) = 0;
        virtual void SetAudioBusVolumeLinear(AZStd::string busName, float volume) = 0;
        virtual void SetAudioBusMuted(AZStd::string busName, bool isMuted) = 0;
        virtual void SetAudioBusMono(AZStd::string busName, bool isMono) = 0;
        virtual void SetAudioBusFilter(AZStd::string busName, int32_t filterIndex, AZStd::string filterName) = 0;
        virtual void SetAudioBusFilterEnabled(AZStd::string busName, int32_t filterIndex, bool isEnabled) = 0;
        virtual void SetAudioBusFilterParam(AZStd::string busName, int32_t filterIndex, AZStd::string paramName, float paramValue) = 0;

        // QueueBroadcast() must be used to ensure guaranteed processing of the request.
        virtual void RegisterAudioFilter(AZ::Name filterName, AudioFilterCreatorFunc filterCreator) = 0;
    };
    using AudioEngineSoLoudRequestBus = AZ::EBus<AudioEngineSoLoudRequests>;
} // namespace Audio
