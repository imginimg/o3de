/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Common.h>
#include <IAudioConnection.h>

namespace AudioControls
{
    class CAudioFileToTriggerConnection : public IAudioConnection
    {
    public:
        explicit CAudioFileToTriggerConnection(CID id)
            : IAudioConnection(id)
        {
        }

        bool HasProperties() override
        {
            return true;
        }

        Audio::SAudioFileToTriggerParams m_params;
    };

    class CAudioFileToRtpcConnection : public IAudioConnection
    {
    public:
        explicit CAudioFileToRtpcConnection(CID id)
            : IAudioConnection(id)
        {
        }

        bool HasProperties() override
        {
            return true;
        }

        Audio::SAudioFileToRtpcParams m_params;
    };
} // namespace AudioControls
