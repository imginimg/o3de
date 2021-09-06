/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AtlData.h>
#include <IAudioConnection.h>

namespace AudioControls
{
    class AudioFileToTriggerConnection : public IAudioConnection
    {
    public:
        explicit AudioFileToTriggerConnection(CID id)
            : IAudioConnection(id)
        {
        }

        ~AudioFileToTriggerConnection() = default;

        bool HasProperties() override
        {
            return true;
        }

        Audio::AudioFileToTriggerParams m_params;
    };

    class AudioFileToRtpcConnection : public IAudioConnection
    {
    public:
        explicit AudioFileToRtpcConnection(CID id)
            : IAudioConnection(id)
        {
        }

        ~AudioFileToRtpcConnection() = default;

        bool HasProperties() override
        {
            return true;
        }

        Audio::AudioFileToRtpcParams m_params;
    };
} // namespace AudioControls
