/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioEngineSoLoudGemSystemComponent.h>

#include <AzCore/PlatformDef.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <AudioSystemImplCVars.h>
#include <AudioSystemImpl_SoLoud.h>
#include <Common.h>

#if defined(AUDIO_ENGINE_SOLOUD_EDITOR)
#include <AudioSystemEditor_SoLoud.h>
#endif // AUDIO_ENGINE_SOLOUD_EDITOR

namespace AudioEngineSoLoudGem
{
    void AudioEngineSoLoudGemSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<AudioEngineSoLoudGemSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<AudioEngineSoLoudGemSystemComponent>(
                      "Audio Engine SoLoud Gem", "SoLoud implementation of the Audio Engine interfaces")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void AudioEngineSoLoudGemSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("AudioEngineService"));
    }

    void AudioEngineSoLoudGemSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("AudioEngineService"));
    }

    void AudioEngineSoLoudGemSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("AudioSystemService"));
    }

    void AudioEngineSoLoudGemSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC("AudioSystemService"));
    }

    void AudioEngineSoLoudGemSystemComponent::Init()
    {
    }

    void AudioEngineSoLoudGemSystemComponent::Activate()
    {
        Audio::Gem::AudioEngineGemRequestBus::Handler::BusConnect();

#if defined(AUDIO_ENGINE_SOLOUD_EDITOR)
        AudioControlsEditor::EditorImplPluginEventBus::Handler::BusConnect();
#endif // AUDIO_ENGINE_SOLOUD_EDITOR
    }

    void AudioEngineSoLoudGemSystemComponent::Deactivate()
    {
        Audio::Gem::AudioEngineGemRequestBus::Handler::BusDisconnect();

#if defined(AUDIO_ENGINE_SOLOUD_EDITOR)
        AudioControlsEditor::EditorImplPluginEventBus::Handler::BusDisconnect();
#endif // AUDIO_ENGINE_SOLOUD_EDITOR
    }

    bool AudioEngineSoLoudGemSystemComponent::Initialize()
    {
        if (!AZ::AllocatorInstance<Audio::AudioImplAllocator>::IsReady())
        {
            const size_t poolSize = Audio::CVars::s_SoLoud_MemoryPoolSize << 10;

            Audio::AudioImplAllocator::Descriptor allocDesc;
            allocDesc.m_allocationRecords = true;
            allocDesc.m_heap.m_numFixedMemoryBlocks = 1;
            allocDesc.m_heap.m_fixedMemoryBlocksByteSize[0] = poolSize;

            allocDesc.m_heap.m_fixedMemoryBlocks[0] = AZ::AllocatorInstance<AZ::OSAllocator>::Get().Allocate(
                allocDesc.m_heap.m_fixedMemoryBlocksByteSize[0], allocDesc.m_heap.m_memoryBlockAlignment);

            AZ::AllocatorInstance<Audio::AudioImplAllocator>::Create(allocDesc);
        }

        m_audioSystemImpl = AZStd::make_unique<Audio::AudioSystemImpl_SoLoud>();
        if (m_audioSystemImpl)
        {
            Audio::SAudioRequest oAudioRequestData;
            oAudioRequestData.nFlags = (Audio::eARF_PRIORITY_HIGH | Audio::eARF_EXECUTE_BLOCKING);

            Audio::SAudioManagerRequestData<Audio::eAMRT_INIT_AUDIO_IMPL> oAMData;
            oAudioRequestData.pData = &oAMData;
            Audio::AudioSystemRequestBus::Broadcast(&Audio::AudioSystemRequestBus::Events::PushRequestBlocking, oAudioRequestData);
        }
        else
        {
            AZ_Error(Audio::LogWindow, false, "Could not create AudioEngineSoLoud!");
            return false;
        }

        return true;
    }

    void AudioEngineSoLoudGemSystemComponent::Release()
    {
        m_audioSystemImpl.reset();

        if (AZ::AllocatorInstance<Audio::AudioImplAllocator>::IsReady())
        {
            AZ::AllocatorInstance<Audio::AudioImplAllocator>::Destroy();
        }
    }

#if defined(AUDIO_ENGINE_SOLOUD_EDITOR)
    void AudioEngineSoLoudGemSystemComponent::InitializeEditorImplPlugin()
    {
        m_editorImplPlugin.reset(new AudioControls::CAudioSystemEditor_SoLoud());
    }

    void AudioEngineSoLoudGemSystemComponent::ReleaseEditorImplPlugin()
    {
        m_editorImplPlugin.reset();
    }

    AudioControls::IAudioSystemEditor* AudioEngineSoLoudGemSystemComponent::GetEditorImplPlugin()
    {
        return m_editorImplPlugin.get();
    }
#endif // AUDIO_ENGINE_SOLOUD_EDITOR

} // namespace AudioEngineSoLoudGem
