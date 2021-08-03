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
    CAudioSystemImpl_SoLoud::CAudioSystemImpl_SoLoud()
    {
        AudioSystemImplementationRequestBus::Handler::BusConnect();
        AudioSystemImplementationNotificationBus::Handler::BusConnect();
    }

    CAudioSystemImpl_SoLoud::~CAudioSystemImpl_SoLoud()
    {
        AudioSystemImplementationRequestBus::Handler::BusDisconnect();
        AudioSystemImplementationNotificationBus::Handler::BusDisconnect();
    }

    void CAudioSystemImpl_SoLoud::OnAudioSystemLoseFocus()
    {
        m_soloud.setPauseAll(true);
    }

    void CAudioSystemImpl_SoLoud::OnAudioSystemGetFocus()
    {
        m_soloud.setPauseAll(false);
    }

    void CAudioSystemImpl_SoLoud::OnAudioSystemMuteAll()
    {
        MuteAll();
    }

    void CAudioSystemImpl_SoLoud::OnAudioSystemUnmuteAll()
    {
        UnmuteAll();
    }

    void CAudioSystemImpl_SoLoud::OnAudioSystemRefresh()
    {
        CleanupAudioSources();
    }

    void CAudioSystemImpl_SoLoud::Update(float)
    {
        AZ_PROFILE_FUNCTION(AZ::Debug::ProfileCategory::Audio);

        m_soloud.update3dAudio();
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::Initialize()
    {
        AZ::u32 flags = 0;
        if (CVars::s_SoLoud_Clipper)
            flags |= SoLoud::Soloud::CLIP_ROUNDOFF;

        const AZ::u32 channelCount = ESpeakerConfiguration::ToChannelCount((ESpeakerConfiguration::Type)(AZ::u32)CVars::s_SoLoud_SpeakerConfiguration);

        auto result = m_soloud.init(flags, SoLoud::Soloud::AUTO, CVars::s_SoLoud_SampleRate, SoLoud::Soloud::AUTO, channelCount);
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

    EAudioRequestStatus CAudioSystemImpl_SoLoud::ShutDown()
    {
        CleanupAudioSources();
        CleanupAudioObjects();

        m_soloud.deinit();
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::Release()
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::StopAllSounds()
    {
        m_soloud.stopAll();

        for (auto object : m_audioObjects)
            object->m_activeSoVoices.clear();

        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::RegisterAudioObject(IATLAudioObjectData*, const char*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::UnregisterAudioObject(IATLAudioObjectData*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::ResetAudioObject(IATLAudioObjectData* objectData)
    {
        SATLAudioObjectDataSoLoud* object = static_cast<SATLAudioObjectDataSoLoud*>(objectData);
        if (!object)
            return eARS_FAILURE;

        *object = SATLAudioObjectDataSoLoud();
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::UpdateAudioObject(IATLAudioObjectData* objectData)
    {
        AZ_PROFILE_FUNCTION(AZ::Debug::ProfileCategory::Audio);

        CheckObjectForExpiredHandles(*static_cast<SATLAudioObjectDataSoLoud*>(objectData));
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::PrepareTriggerSync(IATLAudioObjectData*, const IATLTriggerImplData*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::UnprepareTriggerSync(IATLAudioObjectData*, const IATLTriggerImplData*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::PrepareTriggerAsync(IATLAudioObjectData*, const IATLTriggerImplData*, IATLEventData*)
    {
        return eARS_FAILURE;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::UnprepareTriggerAsync(IATLAudioObjectData*, const IATLTriggerImplData*, IATLEventData*)
    {
        return eARS_FAILURE;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::ActivateTrigger(IATLAudioObjectData* objectData, const IATLTriggerImplData* triggerData,
        IATLEventData* eventData, const SATLSourceData*)
    {
        SATLAudioObjectDataSoLoud* object = static_cast<SATLAudioObjectDataSoLoud*>(objectData);
        const SATLTriggerImplDataSoLoud* trigger = static_cast<const SATLTriggerImplDataSoLoud*>(triggerData);
        SATLEventDataSoLoud* event = static_cast<SATLEventDataSoLoud*>(eventData);

        if (!object || !trigger || !event)
            return eARS_FAILURE;

        auto audioSourceIt = m_audioSources.find(trigger->m_audioFilePath);
        if (audioSourceIt == m_audioSources.end())
            return eARS_FAILURE;

        AZStd::string_view audioFilePath = audioSourceIt->first;
        SoLoud::AudioSource* const audioSource = audioSourceIt->second;

        switch (trigger->m_audioFileToTriggerParams.m_action)
        {
            case EAudioAction::Start:
            {
                SoLoud::handle sohandle;

                if (trigger->m_audioFileToTriggerParams.m_positional)
                {
                    AZ::Vector3 posVec = object->m_pos.GetPositionVec();

                    sohandle = m_soloud.play3d(*audioSource, posVec.GetX(), posVec.GetY(), posVec.GetZ(), 0.0f, 0.0f, 0.0f,
                        trigger->m_audioFileToTriggerParams.m_volume, true);

                    m_soloud.set3dSourceAttenuation(sohandle, trigger->m_audioFileToTriggerParams.m_attenuationMode,
                        trigger->m_audioFileToTriggerParams.m_attenuationRolloffFactor);

                    m_soloud.set3dSourceMinMaxDistance(sohandle, trigger->m_audioFileToTriggerParams.m_minDistance,
                        trigger->m_audioFileToTriggerParams.m_maxDistance);
                }
                else
                {
                    sohandle = m_soloud.playBackground(*audioSource, trigger->m_audioFileToTriggerParams.m_volume, true);
                }

                m_soloud.setLooping(sohandle, trigger->m_audioFileToTriggerParams.m_looping);
                m_soloud.setPause(sohandle, false);

                event->m_isPlayEvent = true;
                event->m_soloudHandle = sohandle;
                object->m_activeSoVoices.insert(AZStd::make_pair(audioFilePath, sohandle));
                break;
            }

            case EAudioAction::Stop:
            {
                auto range = object->m_activeSoVoices.equal_range(audioFilePath);
                for (auto it = range.first; it != range.second; ++it)
                    m_soloud.stop(it->second);

                object->m_activeSoVoices.erase(audioFilePath);
                break;
            }

            case EAudioAction::Pause:
            {
                auto range = object->m_activeSoVoices.equal_range(audioFilePath);
                for (auto it = range.first; it != range.second; ++it)
                    m_soloud.setPause(it->second, true);

                break;
            }

            case EAudioAction::Resume:
            {
                auto range = object->m_activeSoVoices.equal_range(audioFilePath);
                for (auto it = range.first; it != range.second; ++it)
                    m_soloud.setPause(it->second, false);

                break;
            }

            default:
                return eARS_FAILURE;
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::StopEvent(IATLAudioObjectData* objectData, const IATLEventData* eventData)
    {
        SATLAudioObjectDataSoLoud* object = static_cast<SATLAudioObjectDataSoLoud*>(objectData);
        const SATLEventDataSoLoud* event = static_cast<const SATLEventDataSoLoud*>(eventData);

        if (!object || !event)
            return eARS_FAILURE;

        if (!event->m_isPlayEvent)
            return eARS_SUCCESS;

        m_soloud.stop(event->m_soloudHandle);

        for (auto pair : object->m_activeSoVoices)
        {
            if (pair.second == event->m_soloudHandle)
            {
                object->m_activeSoVoices.erase(pair.first);
                break;
            }
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::StopAllEvents(IATLAudioObjectData* objectData)
    {
        SATLAudioObjectDataSoLoud* object = static_cast<SATLAudioObjectDataSoLoud*>(objectData);
        if (!object)
            return eARS_FAILURE;

        for (auto pair : object->m_activeSoVoices)
            m_soloud.stop(pair.second);

        object->m_activeSoVoices.clear();
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::SetPosition(IATLAudioObjectData* objectData, const SATLWorldPosition& worldPosition)
    {
        SATLAudioObjectDataSoLoud* object = static_cast<SATLAudioObjectDataSoLoud*>(objectData);
        if (!object)
            return eARS_FAILURE;

        object->m_pos = worldPosition;
        AZ::Vector3 posVec = worldPosition.GetPositionVec();

        for (auto pair : object->m_activeSoVoices)
        {
            m_soloud.set3dSourcePosition(pair.second, posVec.GetX(), posVec.GetY(), posVec.GetZ());
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::SetMultiplePositions(IATLAudioObjectData*, const MultiPositionParams&)
    {
        return eARS_FAILURE;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::SetRtpc(IATLAudioObjectData*, const IATLRtpcImplData* rtpcData, float value)
    {
        const SATLRtpcImplDataSoLoud* rtpc = static_cast<const SATLRtpcImplDataSoLoud*>(rtpcData);
        if (!rtpc)
            return eARS_FAILURE;

        switch (rtpc->m_type)
        {
            case ERtpcType::GlobalVolume:
                m_soloud.setGlobalVolume(value);
                break;

            default:
                return eARS_FAILURE;
        }

        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::SetSwitchState(IATLAudioObjectData*, const IATLSwitchStateImplData*)
    {
        // Not implemented.
        return eARS_FAILURE;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::SetObstructionOcclusion(IATLAudioObjectData*, float, float)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::SetEnvironment(IATLAudioObjectData*, const IATLEnvironmentImplData*, float)
    {
        // Not implemented.
        return eARS_FAILURE;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::SetListenerPosition(IATLListenerData*, const SATLWorldPosition& newPosition)
    {
        AZ::Vector3 posVec = newPosition.GetPositionVec();
        AZ::Vector3 atVec = newPosition.GetForwardVec();
        AZ::Vector3 upVec = newPosition.GetUpVec();

        m_soloud.set3dListenerParameters(posVec.GetX(), posVec.GetY(), posVec.GetZ(), atVec.GetX(), atVec.GetY(),
            atVec.GetZ(), upVec.GetX(), upVec.GetY(), upVec.GetZ());

        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::ResetRtpc(IATLAudioObjectData*, const IATLRtpcImplData*)
    {
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::RegisterInMemoryFile(SATLAudioFileEntryInfo* audioFileEntry)
    {
        SATLAudioFileEntryDataSoLoud* data = static_cast<SATLAudioFileEntryDataSoLoud*>(audioFileEntry->pImplData);
        if (!data)
            return eARS_FAILURE;

        SoLoud::Wav* source = azcreate(SoLoud::Wav, (), Audio::AudioImplAllocator, "SoLoud::Wav");
        if (!source)
            return eARS_FAILURE;

        auto result = source->loadMem(static_cast<const unsigned char*>(audioFileEntry->pFileData), audioFileEntry->nSize, false, false);
        if (result != SoLoud::SO_NO_ERROR)
        {
            AZ_Error(LogWindow, false, "Unable to load audio file \"%s\"! %s", data->m_fullFilePath.c_str(), m_soloud.getErrorString(result));
            azdestroy(source, Audio::AudioImplAllocator, SoLoud::Wav);
            return eARS_FAILURE;
        }

        m_audioSources[data->m_fullFilePath] = source;
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::UnregisterInMemoryFile(SATLAudioFileEntryInfo* audioFileEntry)
    {
        const SATLAudioFileEntryDataSoLoud* data = static_cast<const SATLAudioFileEntryDataSoLoud*>(audioFileEntry->pImplData);
        if (!data)
            return eARS_FAILURE;

        auto it = m_audioSources.find(data->m_fullFilePath);
        if (it == m_audioSources.end())
            return eARS_FAILURE;

        azdestroy(it->second, Audio::AudioImplAllocator, SoLoud::Wav);
        m_audioSources.erase(it);
        return eARS_SUCCESS;
    }

    EAudioRequestStatus CAudioSystemImpl_SoLoud::ParseAudioFileEntry(const AZ::rapidxml::xml_node<char>* audioFileEntryNode,
        SATLAudioFileEntryInfo* fileEntryInfo)
    {
        if (!audioFileEntryNode || !fileEntryInfo)
            return eARS_FAILURE;

        if (!AZ::StringFunc::Equal(audioFileEntryNode->name(), AudioFileTag))
            return eARS_FAILURE;

        auto attr = audioFileEntryNode->first_attribute(AudioFilePathTag);
        if (!attr)
            return eARS_FAILURE;

        const char* audioFilePath = attr->value();
        if (!audioFilePath || audioFilePath[0] == '\0')
            return eARS_FAILURE;

        SATLAudioFileEntryDataSoLoud* data = azcreate(SATLAudioFileEntryDataSoLoud, (), Audio::AudioImplAllocator, "ATLAudioFileEntryDataSoLoud");
        if (!data)
            return eARS_FAILURE;

        data->m_fullFilePath = audioFilePath;
        fileEntryInfo->pImplData = data;
        fileEntryInfo->sFileName = audioFilePath;

        return eARS_SUCCESS;
    }

    void CAudioSystemImpl_SoLoud::DeleteAudioFileEntryData(IATLAudioFileEntryData* oldAudioFileEntryData)
    {
        azdestroy(oldAudioFileEntryData, Audio::AudioImplAllocator, SATLAudioFileEntryDataSoLoud);
    }

    const char* const CAudioSystemImpl_SoLoud::GetAudioFileLocation(SATLAudioFileEntryInfo*)
    {
        return AudioFilesPath;
    }

    IATLTriggerImplData* CAudioSystemImpl_SoLoud::NewAudioTriggerImplData(const AZ::rapidxml::xml_node<char>* audioTriggerNode)
    {
        if (!audioTriggerNode)
            return nullptr;

        if (!AZ::StringFunc::Equal(audioTriggerNode->name(), AudioFileTag))
            return nullptr;

        auto attr = audioTriggerNode->first_attribute(AudioFilePathTag);
        if (!attr || !attr->value() || attr->value()[0] == '\0')
            return nullptr;

        SATLTriggerImplDataSoLoud* triggerImpl = azcreate(SATLTriggerImplDataSoLoud, (), Audio::AudioImplAllocator, "SATLTriggerImplDataSoLoud");
        if (!triggerImpl)
            return nullptr;

        triggerImpl->m_audioFilePath = attr->value();
        triggerImpl->m_audioFileToTriggerParams.ReadFromXml(*audioTriggerNode);
        return triggerImpl;
    }

    void CAudioSystemImpl_SoLoud::DeleteAudioTriggerImplData(IATLTriggerImplData* oldTriggerData)
    {
        azdestroy(oldTriggerData, Audio::AudioImplAllocator, SATLTriggerImplDataSoLoud);
    }

    IATLRtpcImplData* CAudioSystemImpl_SoLoud::NewAudioRtpcImplData(const AZ::rapidxml::xml_node<char>* audioRtpcNode)
    {
        if (!audioRtpcNode)
            return nullptr;

        ERtpcType::Type rtpcType = ERtpcType::FromString(audioRtpcNode->name());
        if (rtpcType == ERtpcType::Count)
            return nullptr;

        SATLRtpcImplDataSoLoud* rtpcData = azcreate(SATLRtpcImplDataSoLoud, (), Audio::AudioImplAllocator, "SATLRtpcImplDataSoLoud");
        if (!rtpcData)
            return nullptr;

        rtpcData->m_type = rtpcType;

        auto attr = audioRtpcNode->first_attribute(SRtpcParams::ContextTag);
        if (attr)
            rtpcData->m_params.context = attr->value();

        return rtpcData;
    }

    void CAudioSystemImpl_SoLoud::DeleteAudioRtpcImplData(IATLRtpcImplData* oldRtpcData)
    {
        azdestroy(oldRtpcData, Audio::AudioImplAllocator, SATLRtpcImplDataSoLoud);
    }

    IATLSwitchStateImplData* CAudioSystemImpl_SoLoud::NewAudioSwitchStateImplData(const AZ::rapidxml::xml_node<char>*)
    {
        // Not implemented.
        return nullptr;
    }

    void CAudioSystemImpl_SoLoud::DeleteAudioSwitchStateImplData(IATLSwitchStateImplData*)
    {
        // Not implemented.
    }

    IATLEnvironmentImplData* CAudioSystemImpl_SoLoud::NewAudioEnvironmentImplData(const AZ::rapidxml::xml_node<char>*)
    {
        // Not implemented.
        return nullptr;
    }

    void CAudioSystemImpl_SoLoud::DeleteAudioEnvironmentImplData(IATLEnvironmentImplData*)
    {
        // Not implemented.
    }

    IATLAudioObjectData* CAudioSystemImpl_SoLoud::NewGlobalAudioObjectData(TAudioObjectID objectId)
    {
        return NewAudioObjectData(objectId);
    }

    IATLAudioObjectData* CAudioSystemImpl_SoLoud::NewAudioObjectData(TAudioObjectID)
    {
        SATLAudioObjectDataSoLoud* object = azcreate(SATLAudioObjectDataSoLoud, (), Audio::AudioImplAllocator, "SATLAudioObjectDataSoLoud");
        if (!object)
            return nullptr;

        m_audioObjects.insert(object);
        return object;
    }

    void CAudioSystemImpl_SoLoud::DeleteAudioObjectData(IATLAudioObjectData* const oldObjectData)
    {
        if (!oldObjectData)
            return;

        auto it = m_audioObjects.find(static_cast<SATLAudioObjectDataSoLoud*>(oldObjectData));
        if (it != m_audioObjects.end())
            m_audioObjects.erase(it);

        azdestroy(oldObjectData, Audio::AudioImplAllocator, SATLAudioObjectDataSoLoud);
    }

    IATLListenerData* CAudioSystemImpl_SoLoud::NewDefaultAudioListenerObjectData(TATLIDType objectId)
    {
        return NewAudioListenerObjectData(objectId);
    }

    IATLListenerData* CAudioSystemImpl_SoLoud::NewAudioListenerObjectData(TATLIDType)
    {
        return azcreate(IATLListenerData, (), Audio::AudioImplAllocator, "IATLListenerData");
    }

    void CAudioSystemImpl_SoLoud::DeleteAudioListenerObjectData(IATLListenerData* oldListenerData)
    {
        azdestroy(oldListenerData, Audio::AudioImplAllocator, IATLListenerData);
    }

    IATLEventData* CAudioSystemImpl_SoLoud::NewAudioEventData(TAudioEventID)
    {
        return azcreate(SATLEventDataSoLoud, (), Audio::AudioImplAllocator, "SATLEventDataSoLoud");
    }

    void CAudioSystemImpl_SoLoud::DeleteAudioEventData(IATLEventData* oldEventData)
    {
        azdestroy(oldEventData, Audio::AudioImplAllocator, SATLEventDataSoLoud);
    }

    void CAudioSystemImpl_SoLoud::ResetAudioEventData(IATLEventData* eventData)
    {
        SATLEventDataSoLoud* event = static_cast<SATLEventDataSoLoud*>(eventData);
        if (event)
            *event = SATLEventDataSoLoud();
    }

    void CAudioSystemImpl_SoLoud::SetLanguage(const char*)
    {
    }

    const char* const CAudioSystemImpl_SoLoud::GetImplSubPath() const
    {
        return "soloud/";
    }

    const char* const CAudioSystemImpl_SoLoud::GetImplementationNameString() const
    {
        return "SoLoud";
    }

    void CAudioSystemImpl_SoLoud::GetMemoryInfo(SAudioImplMemoryInfo& memoryInfo) const
    {
        memoryInfo.nPrimaryPoolSize = AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().Capacity();
        memoryInfo.nPrimaryPoolUsedSize = memoryInfo.nPrimaryPoolSize - AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().NumAllocatedBytes();
        memoryInfo.nPrimaryPoolAllocations = 0;
        memoryInfo.nSecondaryPoolSize = 0;
        memoryInfo.nSecondaryPoolUsedSize = 0;
        memoryInfo.nSecondaryPoolAllocations = 0;
    }

    AZStd::vector<AudioImplMemoryPoolInfo> CAudioSystemImpl_SoLoud::GetMemoryPoolInfo()
    {
        return AZStd::vector<AudioImplMemoryPoolInfo>();
    }

    bool CAudioSystemImpl_SoLoud::CreateAudioSource(const SAudioInputConfig&)
    {
        // Not implemented.
        return false;
    }

    void CAudioSystemImpl_SoLoud::DestroyAudioSource(TAudioSourceId)
    {
        // Not implemented.
    }

    void CAudioSystemImpl_SoLoud::SetPanningMode(PanningMode)
    {
        // Not implemented.
    }

    void CAudioSystemImpl_SoLoud::CheckObjectForExpiredHandles(SATLAudioObjectDataSoLoud& object)
    {
        AZStd::vector<AZStd::unordered_multimap<AZStd::string, SoLoud::handle>::iterator> iterators;
        iterators.reserve(object.m_activeSoVoices.size());

        for (auto it = object.m_activeSoVoices.begin(); it != object.m_activeSoVoices.end(); ++it)
        {
            const SoLoud::handle handle = it->second;

            if (!m_soloud.isValidVoiceHandle(handle))
            {
                iterators.push_back(it);
            }
        }

        for (auto it : iterators)
            object.m_activeSoVoices.erase(it);
    }

    void CAudioSystemImpl_SoLoud::CleanupAudioSources()
    {
        for (const auto& pair : m_audioSources)
        {
            azdestroy(pair.second, Audio::AudioImplAllocator, SoLoud::Wav);
        }

        m_audioSources.clear();
    }

    void CAudioSystemImpl_SoLoud::CleanupAudioObjects()
    {
        for (auto object : m_audioObjects)
        {
            azdestroy(object, Audio::AudioImplAllocator, SATLAudioObjectDataSoLoud);
        }

        m_audioObjects.clear();
    }

    void CAudioSystemImpl_SoLoud::MuteAll()
    {
        m_globalVolume = m_soloud.getGlobalVolume();
        m_soloud.setGlobalVolume(0.0f);
    }

    void CAudioSystemImpl_SoLoud::UnmuteAll()
    {
        m_soloud.setGlobalVolume(m_globalVolume);
    }
} // namespace Audio
