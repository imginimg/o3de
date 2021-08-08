/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <ATLEntities.h>
#include <AudioSystemImplCVars.h>
#include <AudioSystemImpl_SoLoud.h>
#include <AzCore/StringFunc/StringFunc.h>
#include <AzCore/base.h>
#include <Common.h>
#include <soloud_wav.h>

namespace Audio
{
    AudioSystemImpl_SoLoud::AudioSystemImpl_SoLoud()
    {
        AudioSystemImplementationRequestBus::Handler::BusConnect();
        AudioSystemImplementationNotificationBus::Handler::BusConnect();
    }

    AudioSystemImpl_SoLoud::~AudioSystemImpl_SoLoud()
    {
        AudioSystemImplementationRequestBus::Handler::BusDisconnect();
        AudioSystemImplementationNotificationBus::Handler::BusDisconnect();
    }

    void AudioSystemImpl_SoLoud::OnAudioSystemLoseFocus()
    {
        m_soloud.setPauseAll(true);
    }

    void AudioSystemImpl_SoLoud::OnAudioSystemGetFocus()
    {
        m_soloud.setPauseAll(false);
    }

    void AudioSystemImpl_SoLoud::OnAudioSystemMuteAll()
    {
        MuteAll();
    }

    void AudioSystemImpl_SoLoud::OnAudioSystemUnmuteAll()
    {
        UnmuteAll();
    }

    void AudioSystemImpl_SoLoud::OnAudioSystemRefresh()
    {
        m_audioSources.clear();
    }

    void AudioSystemImpl_SoLoud::Update(float)
    {
        AZ_PROFILE_FUNCTION(AZ::Debug::ProfileCategory::Audio);

        m_soloud.update3dAudio();
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::Initialize()
    {
        AZ::u32 initFlags = 0;
        if (CVars::s_SoLoud_Clipper)
        {
            initFlags |= SoLoud::Soloud::CLIP_ROUNDOFF;
        }

        const AZ::u32 channelCount = SpeakerConfiguration::ToChannelCount((SpeakerConfiguration::Type)(AZ::u32) CVars::s_SoLoud_SpeakerConfiguration);

        auto result = m_soloud.init(initFlags, SoLoud::Soloud::AUTO, CVars::s_SoLoud_SampleRate, SoLoud::Soloud::AUTO, channelCount);
        if (result != SoLoud::SO_NO_ERROR)
        {
            AZ_Error(LogWindow, false, "Unable to init SoLoud! %s", m_soloud.getErrorString(result));
            return eARS_FAILURE;
        }

        m_soloud.setMainResampler(CVars::s_SoLoud_MainResampler);
        m_soloud.setMaxActiveVoiceCount(CVars::s_SoLoud_MaxActiveVoiceCount);

        CVars::s_SoLoud_CurrentBackend = m_soloud.getBackendString();
        CVars::s_SoLoud_CurrentNumberOfChannels = m_soloud.getBackendChannels();
        CVars::s_SoLoud_SampleRate = m_soloud.getBackendSamplerate();
        m_globalVolume = m_soloud.getGlobalVolume();

        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::ShutDown()
    {
        m_audioSources.clear();
        m_audioObjects.clear();

        m_soloud.deinit();
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::Release()
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::StopAllSounds()
    {
        m_soloud.stopAll();

        for (auto& object : m_audioObjects)
        {
            object->m_activeSoVoices.clear();
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::RegisterAudioObject(IATLAudioObjectData*, const char*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::UnregisterAudioObject(IATLAudioObjectData*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::ResetAudioObject(IATLAudioObjectData* objectData)
    {
        auto object = static_cast<AtlAudioObjectDataSoLoud*>(objectData);
        if (!object)
        {
            return eARS_FAILURE;
        }

        *object = AtlAudioObjectDataSoLoud();
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::UpdateAudioObject(IATLAudioObjectData* objectData)
    {
        AZ_PROFILE_FUNCTION(AZ::Debug::ProfileCategory::Audio);

        if (!objectData)
        {
            return eARS_FAILURE;
        }

        CheckObjectForExpiredHandles(*static_cast<AtlAudioObjectDataSoLoud*>(objectData));
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::PrepareTriggerSync(IATLAudioObjectData*, const IATLTriggerImplData*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::UnprepareTriggerSync(IATLAudioObjectData*, const IATLTriggerImplData*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::PrepareTriggerAsync(IATLAudioObjectData*, const IATLTriggerImplData*, IATLEventData*)
    {
        return eARS_FAILURE;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::UnprepareTriggerAsync(IATLAudioObjectData*, const IATLTriggerImplData*, IATLEventData*)
    {
        return eARS_FAILURE;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::ActivateTrigger(IATLAudioObjectData* objectData, const IATLTriggerImplData* triggerData
        , IATLEventData* eventData, const SATLSourceData*)
    {
        auto object = static_cast<AtlAudioObjectDataSoLoud*>(objectData);
        auto trigger = static_cast<const AtlTriggerImplDataSoLoud*>(triggerData);
        auto event = static_cast<AtlEventDataSoLoud*>(eventData);

        if (!object || !trigger || !event)
        {
            return eARS_FAILURE;
        }

        auto audioSourceIt = m_audioSources.find(trigger->m_audioFilePath);
        if (audioSourceIt == m_audioSources.end())
        {
            return eARS_FAILURE;
        }

        AZStd::string_view audioFilePath = audioSourceIt->first;
        SoLoud::AudioSource* const audioSource = audioSourceIt->second.get();

        switch (trigger->m_audioFileToTriggerParams.m_action)
        {
            case AudioAction::Start:
            {
                SoLoud::handle sohandle;
                const float effectiveVoiceVolume = audioSource->mVolume * trigger->m_audioFileToTriggerParams.m_volume;

                if (trigger->m_audioFileToTriggerParams.m_positional)
                {
                    AZ::Vector3 posVec = object->m_position;

                    sohandle = m_soloud.play3d(*audioSource, posVec.GetX(), posVec.GetY(), posVec.GetZ(), 0.0f, 0.0f, 0.0f
                        , effectiveVoiceVolume, true);

                    m_soloud.set3dSourceAttenuation(sohandle, trigger->m_audioFileToTriggerParams.m_attenuationMode
                        , trigger->m_audioFileToTriggerParams.m_attenuationRolloffFactor);

                    m_soloud.set3dSourceMinMaxDistance(sohandle, trigger->m_audioFileToTriggerParams.m_minDistance
                        , trigger->m_audioFileToTriggerParams.m_maxDistance);
                }
                else
                {
                    sohandle = m_soloud.playBackground(*audioSource, effectiveVoiceVolume, true);
                }

                m_soloud.setLooping(sohandle, trigger->m_audioFileToTriggerParams.m_looping);
                m_soloud.setPause(sohandle, false);

                event->m_isPlayEvent = true;
                event->m_soloudHandle = sohandle;
                object->m_activeSoVoices.emplace(audioFilePath, ActiveSoVoiceData{ sohandle, trigger->m_audioFileToTriggerParams.m_volume});
                break;
            }

            case AudioAction::Stop:
            {
                audioSource->stop();
                object->m_activeSoVoices.erase(audioFilePath);
                break;
            }

            case AudioAction::Pause:
            {
                auto range = object->m_activeSoVoices.equal_range(audioFilePath);
                for (auto it = range.first; it != range.second; ++it)
                {
                    m_soloud.setPause(it->second.m_handle, true);
                }

                break;
            }

            case AudioAction::Resume:
            {
                auto range = object->m_activeSoVoices.equal_range(audioFilePath);
                for (auto it = range.first; it != range.second; ++it)
                {
                    m_soloud.setPause(it->second.m_handle, false);
                }

                break;
            }

            default:
                return eARS_FAILURE;
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::StopEvent(IATLAudioObjectData* objectData, const IATLEventData* eventData)
    {
        auto object = static_cast<AtlAudioObjectDataSoLoud*>(objectData);
        auto event = static_cast<const AtlEventDataSoLoud*>(eventData);

        if (!object || !event)
        {
            return eARS_FAILURE;
        }

        if (!event->m_isPlayEvent)
        {
            return eARS_SUCCESS;
        }            

        m_soloud.stop(event->m_soloudHandle);

        for (auto pair : object->m_activeSoVoices)
        {
            if (pair.second.m_handle == event->m_soloudHandle)
            {
                object->m_activeSoVoices.erase(pair.first);
                break;
            }
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::StopAllEvents(IATLAudioObjectData* objectData)
    {
        auto object = static_cast<AtlAudioObjectDataSoLoud*>(objectData);
        if (!object)
        {
            return eARS_FAILURE;
        }

        for (auto pair : object->m_activeSoVoices)
        {
            m_soloud.stop(pair.second.m_handle);
        }

        object->m_activeSoVoices.clear();
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::SetPosition(IATLAudioObjectData* objectData, const SATLWorldPosition& worldPosition)
    {
        auto object = static_cast<AtlAudioObjectDataSoLoud*>(objectData);
        if (!object)
        {
            return eARS_FAILURE;
        }

        AZ::Vector3 posVec = worldPosition.GetPositionVec();
        object->m_position = posVec;

        for (auto pair : object->m_activeSoVoices)
        {
            m_soloud.set3dSourcePosition(pair.second.m_handle, posVec.GetX(), posVec.GetY(), posVec.GetZ());
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::SetMultiplePositions(IATLAudioObjectData*, const MultiPositionParams&)
    {
        return eARS_FAILURE;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::SetRtpc(IATLAudioObjectData* objecData, const IATLRtpcImplData* rtpcData, float value)
    {
        auto object = static_cast<AtlAudioObjectDataSoLoud*>(objecData);
        auto rtpc = static_cast<const AtlRtpcImplDataSoLoud*>(rtpcData);
        if (!object || !rtpc)
        {
            return eARS_FAILURE;
        }

        switch (rtpc->m_type)
        {
            case RtpcImpl::Global:
            {
                switch (rtpc->m_global.m_type)
                {
                    case GlobalRtpc::GlobalVolume:
                        m_soloud.setGlobalVolume(value);
                        break;

                    default:
                        return eARS_FAILURE;
                }
                break;
            }

            case RtpcImpl::AudioFile:
            {
                auto audioSourceIt = m_audioSources.find(rtpc->m_audioFile.m_audioFilePath);
                if (audioSourceIt == m_audioSources.end())
                {
                    return eARS_FAILURE;
                }

                SoLoud::AudioSource* const audioSource = audioSourceIt->second.get();
                auto voiceRange = object->m_activeSoVoices.equal_range(audioSourceIt->first);

                switch (rtpc->m_audioFile.m_params.m_type)
                {
                    case AudioFileRtpc::Volume:
                    {
                        if (rtpc->m_audioFile.m_params.m_perObject)
                        {
                            const float effectiveVoiceVolume = audioSource->mVolume * value;
                            for (auto it = voiceRange.first; it != voiceRange.second; ++it)
                            {
                                it->second.m_volume = value;
                                m_soloud.setVolume(it->second.m_handle, effectiveVoiceVolume);
                            }
                        }
                        else
                        {
                            audioSource->setVolume(value);

                            // Set volume on all currently playing instances.
                            for (auto& obj : m_audioObjects)
                            {
                                for (auto pair : obj->m_activeSoVoices)
                                {
                                    const float effectiveVoiceVolume = pair.second.m_volume * value;
                                    if (pair.first == rtpc->m_audioFile.m_audioFilePath)
                                    {
                                        m_soloud.setVolume(pair.second.m_handle, effectiveVoiceVolume);
                                    }
                                }
                            }
                        }
                        break;
                    }

                    case AudioFileRtpc::PlaySpeed:
                    {
                        for (auto it = voiceRange.first; it != voiceRange.second; ++it)
                        {
                            m_soloud.setRelativePlaySpeed(it->second.m_handle, value);
                        }
                        break;
                    }

                    case AudioFileRtpc::Seek:
                    {
                        for (auto it = voiceRange.first; it != voiceRange.second; ++it)
                        {
                            m_soloud.seek(it->second.m_handle, value);
                        }
                        break;
                    }

                    default:
                        return eARS_FAILURE; 
                }
                break;
            }

            default:
                return eARS_FAILURE;
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::SetSwitchState(IATLAudioObjectData*, const IATLSwitchStateImplData*)
    {
        // Not implemented.
        return eARS_FAILURE;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::SetObstructionOcclusion(IATLAudioObjectData*, float, float)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::SetEnvironment(IATLAudioObjectData*, const IATLEnvironmentImplData*, float)
    {
        // Not implemented.
        return eARS_FAILURE;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::SetListenerPosition(IATLListenerData*, const SATLWorldPosition& newPosition)
    {
        AZ::Vector3 posVec = newPosition.GetPositionVec();
        AZ::Vector3 atVec = newPosition.GetForwardVec();
        AZ::Vector3 upVec = newPosition.GetUpVec();

        m_soloud.set3dListenerParameters(posVec.GetX(), posVec.GetY(), posVec.GetZ(), atVec.GetX(), atVec.GetY()
            , atVec.GetZ(), upVec.GetX(), upVec.GetY(), upVec.GetZ());

        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::ResetRtpc(IATLAudioObjectData*, const IATLRtpcImplData*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::RegisterInMemoryFile(SATLAudioFileEntryInfo* audioFileEntry)
    {
        auto data = static_cast<AtlAudioFileEntryDataSoLoud*>(audioFileEntry->pImplData);
        if (!data)
        {
            return eARS_FAILURE;
        }

        AudioSourcePtr audioSource(azcreate(SoLoud::Wav, (), Audio::AudioImplAllocator, "SoLoud::Wav"));
        if (!audioSource)
        {
            return eARS_FAILURE;
        }

        auto result = audioSource->loadMem(static_cast<const unsigned char*>(audioFileEntry->pFileData)
            , aznumeric_cast<uint32_t>(audioFileEntry->nSize), false, false);
        if (result != SoLoud::SO_NO_ERROR)
        {
            AZ_Error(LogWindow, false, "Unable to load audio file \"%s\"! %s", data->m_fullFilePath.c_str(), m_soloud.getErrorString(result));
            return eARS_FAILURE;
        }

        m_audioSources.emplace(data->m_fullFilePath, AZStd::move(audioSource));
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::UnregisterInMemoryFile(SATLAudioFileEntryInfo* audioFileEntry)
    {
        auto data = static_cast<const AtlAudioFileEntryDataSoLoud*>(audioFileEntry->pImplData);
        if (!data)
        {
            return eARS_FAILURE;
        }

        auto it = m_audioSources.find(data->m_fullFilePath);
        if (it == m_audioSources.end())
        {
            return eARS_FAILURE;
        }

        m_audioSources.erase(it);
        return eARS_SUCCESS;
    }

    EAudioRequestStatus AudioSystemImpl_SoLoud::ParseAudioFileEntry(const AZ::rapidxml::xml_node<char>* audioFileEntryNode
        , SATLAudioFileEntryInfo* fileEntryInfo)
    {
        if (!audioFileEntryNode || !fileEntryInfo)
        {
            return eARS_FAILURE;
        }

        if (!AZ::StringFunc::Equal(audioFileEntryNode->name(), AudioFileTag))
        {
            return eARS_FAILURE;
        }

        auto attr = audioFileEntryNode->first_attribute(AudioFilePathTag);
        if (!attr)
        {
            return eARS_FAILURE;
        }

        const char* audioFilePath = attr->value();
        if (!audioFilePath || audioFilePath[0] == '\0')
        {
            return eARS_FAILURE;
        }

        bool isLocalized = false;
        attr = audioFileEntryNode->first_attribute(AudioFileLocalizedTag);
        if (attr)
        {
            isLocalized = AZStd::stoi(AZStd::string(attr->value()));
        }

        auto data = azcreate(AtlAudioFileEntryDataSoLoud, (), Audio::AudioImplAllocator, "AtlAudioFileEntryDataSoLoud");
        if (!data)
        {
            return eARS_FAILURE;
        }

        AZ::IO::FixedMaxPath fullFilePath = audioFilePath;
        if (isLocalized)
        {
            fullFilePath = AZ::IO::FixedMaxPath(LocalizationDirName) / m_currentLanguageName / fullFilePath; 
        }
        data->m_fullFilePath = fullFilePath.Native();

        fileEntryInfo->pImplData = data;
        fileEntryInfo->sFileName = audioFilePath;
        fileEntryInfo->bLocalized = isLocalized;

        return eARS_SUCCESS;
    }

    void AudioSystemImpl_SoLoud::DeleteAudioFileEntryData(IATLAudioFileEntryData* oldAudioFileEntryData)
    {
        azdestroy(oldAudioFileEntryData, Audio::AudioImplAllocator, AtlAudioFileEntryDataSoLoud);
    }

    const char* const AudioSystemImpl_SoLoud::GetAudioFileLocation(SATLAudioFileEntryInfo* fileEntryInfo)
    {
        if (!fileEntryInfo)
        {
            return nullptr;
        }

        if (fileEntryInfo->bLocalized)
        {
            return m_localizationDirPath.c_str();
        }
        else
        {
            return AudioFilesPath.c_str();
        }
    }

    IATLTriggerImplData* AudioSystemImpl_SoLoud::NewAudioTriggerImplData(const AZ::rapidxml::xml_node<char>* audioTriggerNode)
    {
        if (!audioTriggerNode)
        {
            return nullptr;
        }

        if (!AZ::StringFunc::Equal(audioTriggerNode->name(), AudioFileTag))
        {
            return nullptr;
        }

        auto attr = audioTriggerNode->first_attribute(AudioFilePathTag);
        if (!attr || !attr->value() || attr->value()[0] == '\0')
        {
            return nullptr;
        }

        const char* audioFilePath = attr->value();

        bool isLocalized = false;
        attr = audioTriggerNode->first_attribute(AudioFileLocalizedTag);
        if (attr)
        {
            isLocalized = AZStd::stoi(AZStd::string(attr->value()));
        }

        AZ::IO::FixedMaxPath fullFilePath = audioFilePath;
        if (isLocalized)
        {
            fullFilePath = AZ::IO::FixedMaxPath(LocalizationDirName) / m_currentLanguageName / fullFilePath; 
        }

        AtlTriggerImplDataSoLoud* triggerImpl = azcreate(AtlTriggerImplDataSoLoud, (), Audio::AudioImplAllocator
            , "AtlTriggerImplDataSoLoud");
        if (!triggerImpl)
        {
            return nullptr;
        }

        triggerImpl->m_audioFilePath = fullFilePath.Native();
        triggerImpl->m_audioFileToTriggerParams.ReadFromXml(*audioTriggerNode);
        return triggerImpl;
    }

    void AudioSystemImpl_SoLoud::DeleteAudioTriggerImplData(IATLTriggerImplData* oldTriggerData)
    {
        azdestroy(oldTriggerData, Audio::AudioImplAllocator, AtlTriggerImplDataSoLoud);
    }

    IATLRtpcImplData* AudioSystemImpl_SoLoud::NewAudioRtpcImplData(const AZ::rapidxml::xml_node<char>* audioRtpcNode)
    {
        if (!audioRtpcNode)
        {
            return nullptr;
        }

        auto rtpcData = azcreate(AtlRtpcImplDataSoLoud, (), Audio::AudioImplAllocator, "AtlRtpcImplDataSoLoud");
        if (!rtpcData)
        {
            return nullptr;
        }

        if (!rtpcData->ReadFromXml(*audioRtpcNode))
        {
            azdestroy(rtpcData, Audio::AudioImplAllocator, AtlRtpcImplDataSoLoud);
            return nullptr;
        }

        return rtpcData;
    }

    void AudioSystemImpl_SoLoud::DeleteAudioRtpcImplData(IATLRtpcImplData* oldRtpcData)
    {
        azdestroy(oldRtpcData, Audio::AudioImplAllocator, AtlRtpcImplDataSoLoud);
    }

    IATLSwitchStateImplData* AudioSystemImpl_SoLoud::NewAudioSwitchStateImplData(const AZ::rapidxml::xml_node<char>*)
    {
        // Not implemented.
        return nullptr;
    }

    void AudioSystemImpl_SoLoud::DeleteAudioSwitchStateImplData(IATLSwitchStateImplData*)
    {
        // Not implemented.
    }

    IATLEnvironmentImplData* AudioSystemImpl_SoLoud::NewAudioEnvironmentImplData(const AZ::rapidxml::xml_node<char>*)
    {
        // Not implemented.
        return nullptr;
    }

    void AudioSystemImpl_SoLoud::DeleteAudioEnvironmentImplData(IATLEnvironmentImplData*)
    {
        // Not implemented.
    }

    IATLAudioObjectData* AudioSystemImpl_SoLoud::NewGlobalAudioObjectData(TAudioObjectID objectId)
    {
        return NewAudioObjectData(objectId);
    }

    IATLAudioObjectData* AudioSystemImpl_SoLoud::NewAudioObjectData(TAudioObjectID)
    {
        AudioObjectPtr object(azcreate(AtlAudioObjectDataSoLoud, (), Audio::AudioImplAllocator, "AtlAudioObjectDataSoLoud"));
        if (!object)
        {
            return nullptr;
        }

        auto pair = m_audioObjects.emplace(AZStd::move(object));
        if (!pair.second)
        {
            return nullptr;
        }

        return pair.first->get();
    }

    void AudioSystemImpl_SoLoud::DeleteAudioObjectData(IATLAudioObjectData* const oldObjectData)
    {
        if (!oldObjectData)
        {
            return;
        }

        auto pred = [&](AudioObjectPtr& ptr) { return ptr.get() == oldObjectData; };
        auto it = AZStd::find_if(m_audioObjects.begin(), m_audioObjects.end(), pred);
        if (it != m_audioObjects.end())
        {
            m_audioObjects.erase(it);
        }
    }

    IATLListenerData* AudioSystemImpl_SoLoud::NewDefaultAudioListenerObjectData(TATLIDType objectId)
    {
        return NewAudioListenerObjectData(objectId);
    }

    IATLListenerData* AudioSystemImpl_SoLoud::NewAudioListenerObjectData(TATLIDType)
    {
        return azcreate(IATLListenerData, (), Audio::AudioImplAllocator, "IATLListenerData");
    }

    void AudioSystemImpl_SoLoud::DeleteAudioListenerObjectData(IATLListenerData* oldListenerData)
    {
        azdestroy(oldListenerData, Audio::AudioImplAllocator, IATLListenerData);
    }

    IATLEventData* AudioSystemImpl_SoLoud::NewAudioEventData(TAudioEventID)
    {
        return azcreate(AtlEventDataSoLoud, (), Audio::AudioImplAllocator, "AtlEventDataSoLoud");
    }

    void AudioSystemImpl_SoLoud::DeleteAudioEventData(IATLEventData* oldEventData)
    {
        azdestroy(oldEventData, Audio::AudioImplAllocator, AtlEventDataSoLoud);
    }

    void AudioSystemImpl_SoLoud::ResetAudioEventData(IATLEventData* eventData)
    {
        auto event = static_cast<AtlEventDataSoLoud*>(eventData);
        if (event)
        {
            *event = AtlEventDataSoLoud();
        }
    }

    void AudioSystemImpl_SoLoud::SetLanguage(const char* language)
    {
        if (!language)
        {
            return;
        }

        m_currentLanguageName = language;
        AZStd::to_lower(m_currentLanguageName.begin(), m_currentLanguageName.end());
        m_localizationDirPath = AudioFilesPath / LocalizationDirName / language;
    }

    const char* const AudioSystemImpl_SoLoud::GetImplSubPath() const
    {
        return "soloud/";
    }

    const char* const AudioSystemImpl_SoLoud::GetImplementationNameString() const
    {
        return "SoLoud";
    }

    void AudioSystemImpl_SoLoud::GetMemoryInfo(SAudioImplMemoryInfo& memoryInfo) const
    {
        memoryInfo.nPrimaryPoolSize = AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().Capacity();
        memoryInfo.nPrimaryPoolUsedSize = memoryInfo.nPrimaryPoolSize - AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().NumAllocatedBytes();
        memoryInfo.nPrimaryPoolAllocations = 0;
        memoryInfo.nSecondaryPoolSize = 0;
        memoryInfo.nSecondaryPoolUsedSize = 0;
        memoryInfo.nSecondaryPoolAllocations = 0;
    }

    AZStd::vector<AudioImplMemoryPoolInfo> AudioSystemImpl_SoLoud::GetMemoryPoolInfo()
    {
        return AZStd::vector<AudioImplMemoryPoolInfo>();
    }

    bool AudioSystemImpl_SoLoud::CreateAudioSource(const SAudioInputConfig&)
    {
        // Not implemented.
        return false;
    }

    void AudioSystemImpl_SoLoud::DestroyAudioSource(TAudioSourceId)
    {
        // Not implemented.
    }

    void AudioSystemImpl_SoLoud::SetPanningMode(PanningMode)
    {
        // Not implemented.
    }

    void AudioSystemImpl_SoLoud::CheckObjectForExpiredHandles(AtlAudioObjectDataSoLoud& object)
    {
        AZStd::vector<AZStd::unordered_multimap<AZStd::string, ActiveSoVoiceData>::iterator> iterators;
        iterators.reserve(object.m_activeSoVoices.size());

        for (auto it = object.m_activeSoVoices.begin(); it != object.m_activeSoVoices.end(); ++it)
        {
            if (!m_soloud.isValidVoiceHandle(it->second.m_handle))
            {
                iterators.push_back(it);
            }
        }

        for (auto it : iterators)
        {
            object.m_activeSoVoices.erase(it);
        }
    }

    void AudioSystemImpl_SoLoud::MuteAll()
    {
        m_globalVolume = m_soloud.getGlobalVolume();
        m_soloud.setGlobalVolume(0.0f);
    }

    void AudioSystemImpl_SoLoud::UnmuteAll()
    {
        m_soloud.setGlobalVolume(m_globalVolume);
    }
} // namespace Audio
