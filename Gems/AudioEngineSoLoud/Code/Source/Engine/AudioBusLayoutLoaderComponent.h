/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>

namespace Audio
{
    class AudioBusLayoutLoaderComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT(AudioBusLayoutLoaderComponent, "{5F8D7215-F0CE-445E-9700-79DF0BC1577F}", AZ::Component);

        static void Reflect(AZ::ReflectContext* context);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
        // ~AZ::Component

    private:
        AZStd::string m_layoutFilePath;
    };
} // namespace Audio
