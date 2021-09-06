/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioBusLayoutLoaderComponent.h>
#include <EditorEngineInterop.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Audio
{
    void AudioBusLayoutLoaderComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serialContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialContext->Class<AudioBusLayoutLoaderComponent, AZ::Component>()->Version(1)->Field(
                "layoutFilePath", &AudioBusLayoutLoaderComponent::m_layoutFilePath);

            if (auto editContext = serialContext->GetEditContext())
            {
                editContext->Class<AudioBusLayoutLoaderComponent>("Audio Bus Layout Loader", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Audio")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
                    ->DataElement(AZ_CRC("LayoutFilePath"), &AudioBusLayoutLoaderComponent::m_layoutFilePath, "Layout file", "");
            }
        }
    }

    void AudioBusLayoutLoaderComponent::Activate()
    {
        if (!m_layoutFilePath.empty())
        {
            AZ::IO::FixedMaxPath path = m_layoutFilePath.c_str();
            AudioBusManagerRequestBus::QueueBroadcast(&AudioBusManagerRequestBus::Events::LoadAudioBusLayout, path);
        }
    }

    void AudioBusLayoutLoaderComponent::Deactivate()
    {
    }
} // namespace Audio
