/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AudioAllocators.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/containers/unordered_set.h>
#include <IAudioSystemImplementation.h>
#include <soloud.h>

namespace SoLoud { class Wav; }

namespace Audio
{
    struct SATLAudioObjectDataSoLoud;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    class CAudioSystemImpl_SoLoud : public AudioSystemImplementation
    {
    public:
        AUDIO_IMPL_CLASS_ALLOCATOR(CAudioSystemImpl_SoLoud)

        explicit CAudioSystemImpl_SoLoud();
        ~CAudioSystemImpl_SoLoud() override;

        // AudioSystemImplementationNotificationBus
        void OnAudioSystemLoseFocus() override;
        void OnAudioSystemGetFocus() override;
        void OnAudioSystemMuteAll() override;
        void OnAudioSystemUnmuteAll() override;
        void OnAudioSystemRefresh() override;
        // ~AudioSystemImplementationNotificationBus

        // AudioSystemImplementationRequestBus
        void Update(float updateIntervalMS) override;
        EAudioRequestStatus Initialize() override;
        EAudioRequestStatus ShutDown() override;
        EAudioRequestStatus Release() override;
        EAudioRequestStatus StopAllSounds() override;
        EAudioRequestStatus RegisterAudioObject(IATLAudioObjectData* objectData, const char* objectName = nullptr) override;
        EAudioRequestStatus UnregisterAudioObject(IATLAudioObjectData* objectData) override;
        EAudioRequestStatus ResetAudioObject(IATLAudioObjectData* objectData) override;
        EAudioRequestStatus UpdateAudioObject(IATLAudioObjectData* objectData) override;
        EAudioRequestStatus PrepareTriggerSync(IATLAudioObjectData* audioObjectData, const IATLTriggerImplData* triggerData) override;
        EAudioRequestStatus UnprepareTriggerSync(IATLAudioObjectData* objectData, const IATLTriggerImplData* triggerData) override;
        EAudioRequestStatus PrepareTriggerAsync(IATLAudioObjectData* objectData, const IATLTriggerImplData* triggerData,
            IATLEventData* eventData) override;
        EAudioRequestStatus UnprepareTriggerAsync(IATLAudioObjectData* pAudioObjectData, const IATLTriggerImplData* pTriggerData,
            IATLEventData* pEventData) override;
        EAudioRequestStatus ActivateTrigger(IATLAudioObjectData* objectData, const IATLTriggerImplData* triggerData,
            IATLEventData* tventData, const SATLSourceData* sourceData) override;
        EAudioRequestStatus StopEvent(IATLAudioObjectData* objectData, const IATLEventData* eventData) override;
        EAudioRequestStatus StopAllEvents(IATLAudioObjectData* objectData) override;
        EAudioRequestStatus SetPosition(IATLAudioObjectData* objectData, const SATLWorldPosition& worldPosition) override;
        EAudioRequestStatus SetMultiplePositions(IATLAudioObjectData* objectData, const MultiPositionParams& multiPositions) override;
        EAudioRequestStatus SetRtpc(IATLAudioObjectData* objectData, const IATLRtpcImplData* rtpcData, float value) override;
        EAudioRequestStatus SetSwitchState(IATLAudioObjectData* objectData, const IATLSwitchStateImplData* switchStateData) override;
        EAudioRequestStatus SetObstructionOcclusion(IATLAudioObjectData* objectData, float obstruction, float occlusion) override;
        EAudioRequestStatus SetEnvironment(IATLAudioObjectData* objectData, const IATLEnvironmentImplData* environmentData,
            float amount) override;
        EAudioRequestStatus SetListenerPosition(IATLListenerData* listenerData, const SATLWorldPosition& newPosition) override;
        EAudioRequestStatus ResetRtpc(IATLAudioObjectData* objectData, const IATLRtpcImplData* rtpcData) override;
        EAudioRequestStatus RegisterInMemoryFile(SATLAudioFileEntryInfo* audioFileEntry) override;
        EAudioRequestStatus UnregisterInMemoryFile(SATLAudioFileEntryInfo* audioFileEntry) override;
        EAudioRequestStatus ParseAudioFileEntry(const AZ::rapidxml::xml_node<char>* audioFileEntryNode, SATLAudioFileEntryInfo* fileEntryInfo) override;
        void DeleteAudioFileEntryData(IATLAudioFileEntryData* oldAudioFileEntryData) override;
        const char* const GetAudioFileLocation(SATLAudioFileEntryInfo* fileEntryInfo) override;
        IATLTriggerImplData* NewAudioTriggerImplData(const AZ::rapidxml::xml_node<char>* audioTriggerNode) override;
        void DeleteAudioTriggerImplData(IATLTriggerImplData* oldTriggerData) override;
        IATLRtpcImplData* NewAudioRtpcImplData(const AZ::rapidxml::xml_node<char>* audioRtpcNode) override;
        void DeleteAudioRtpcImplData(IATLRtpcImplData* oldRtpcData) override;
        IATLSwitchStateImplData* NewAudioSwitchStateImplData(const AZ::rapidxml::xml_node<char>* audioSwitchStateNode) override;
        void DeleteAudioSwitchStateImplData(IATLSwitchStateImplData* oldAudioSwitchStateData) override;
        IATLEnvironmentImplData* NewAudioEnvironmentImplData(const AZ::rapidxml::xml_node<char>* audioEnvironmentNode) override;
        void DeleteAudioEnvironmentImplData(IATLEnvironmentImplData* oldEnvironmentData) override;
        IATLAudioObjectData* NewGlobalAudioObjectData(TAudioObjectID objectId) override;
        IATLAudioObjectData* NewAudioObjectData(TAudioObjectID objectId) override;
        void DeleteAudioObjectData(IATLAudioObjectData* oldObjectData) override;
        IATLListenerData* NewDefaultAudioListenerObjectData(TATLIDType objectId) override;
        IATLListenerData* NewAudioListenerObjectData(TATLIDType objectId) override;
        void DeleteAudioListenerObjectData(IATLListenerData* oldListenerData) override;
        IATLEventData* NewAudioEventData(TAudioEventID eventID) override;
        void DeleteAudioEventData(IATLEventData* oldEventData) override;
        void ResetAudioEventData(IATLEventData* eventData) override;
        void SetLanguage(const char* language) override;
        const char* const GetImplSubPath() const override;
        const char* const GetImplementationNameString() const override;
        void GetMemoryInfo(SAudioImplMemoryInfo& memoryInfo) const override;
        AZStd::vector<AudioImplMemoryPoolInfo> GetMemoryPoolInfo() override;
        bool CreateAudioSource(const SAudioInputConfig& sourceConfig) override;
        void DestroyAudioSource(TAudioSourceId sourceId) override;
        void SetPanningMode(PanningMode mode) override;
        // ~AudioSystemImplementationRequestBus

    private:
        void CheckObjectForExpiredHandles(SATLAudioObjectDataSoLoud& object);
        void CleanupAudioSources();
        void CleanupAudioObjects();
        void MuteAll();
        void UnmuteAll();

        SoLoud::Soloud m_soloud;
        AZStd::unordered_set<SATLAudioObjectDataSoLoud*> m_audioObjects;
        AZStd::unordered_map<AZStd::string, SoLoud::Wav*> m_audioSources; // Key - audio file path.
        float m_globalVolume;
    };
} // namespace Audio
