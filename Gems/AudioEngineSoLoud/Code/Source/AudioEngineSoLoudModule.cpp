/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/PlatformDef.h>

#include <AzCore/Memory/SystemAllocator.h>
#include <IGem.h>

#include <AudioEngineSoLoudGemSystemComponent.h>


namespace AudioEngineSoLoudGem
{
    class AudioEngineSoLoudModule : public CryHooksModule
    {
    public:
        AZ_RTTI(AudioEngineSoLoudModule, "{4D09C022-D01C-4E09-BF89-41B71BB60882}", CryHooksModule);
        AZ_CLASS_ALLOCATOR(AudioEngineSoLoudModule, AZ::SystemAllocator, 0);

        AudioEngineSoLoudModule()
            : CryHooksModule()
        {
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    AudioEngineSoLoudGemSystemComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<AudioEngineSoLoudGemSystemComponent>(),
            };
        }
    };

} // namespace AudioEngineSoLoudGem

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(Gem_AudioEngineSoLoud, AudioEngineSoLoudGem::AudioEngineSoLoudModule)
