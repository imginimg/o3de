/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AudioAllocators.h>
#include <AzCore/Name/Name.h>
#include <AzCore/std/containers/array.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

#include <EditorEngineInterop.h>
#include <soloud.h>

namespace Audio
{
    class AudioBusManager;
    class AudioFilter;
    class AudioFilterManager;

    class AudioBus
    {
    public:
        constexpr static const int32_t InvalidIndex = -1;

        AUDIO_IMPL_CLASS_ALLOCATOR(AudioBus);
        AZ_DISABLE_COPY_MOVE(AudioBus);
        friend class AudioBusManager;

        AudioBus(AudioBusManager& manager, SoLoud::Soloud& soloud, const AZ::Name& name);
        ~AudioBus() = default;

        void Activate();
        void Deactivate();
        bool IsActivated() const;

        bool SetName(const AZ::Name& name);
        const AZ::Name& GetName() const;
        bool SetOutputBusName(const AZ::Name& outputBusName);
        const AZ::Name& GetOutputBusName() const;
        void SetVolumeDb(float volume);
        float GetVolumeDb() const;
        void SetVolumeLinear(float volume);
        float GetVolumeLinear() const;
        void SetMuted(bool isMuted);
        bool IsMuted() const;
        void SetMono(bool isMono);
        bool IsMono() const;

        SoLoud::handle Play(SoLoud::AudioSource& sound, float volume = -1.0f, float pan = 0.0f, bool paused = false);
        SoLoud::handle PlayClocked(SoLoud::time soundTime, SoLoud::AudioSource& sound, float volume = 1.0f, float pan = 0.0f);
        SoLoud::handle Play3d(
            SoLoud::AudioSource& sound,
            float posX,
            float posY,
            float posZ,
            float velX = 0.0f,
            float velY = 0.0f,
            float velZ = 0.0f,
            float volume = -1.0f,
            bool paused = false);
        SoLoud::handle Play3dClocked(
            SoLoud::time soundTime,
            SoLoud::AudioSource& sound,
            float posX,
            float posY,
            float posZ,
            float velX = 0.0f,
            float velY = 0.0f,
            float velZ = 0.0f,
            float volume = -1.0f);
        SoLoud::handle PlayBackground(SoLoud::AudioSource& sound, float volume = -1.0f, bool paused = false);

        void SetFilter(int32_t filterIndex, AudioFilter* filter);
        AudioFilter* GetFilter(int32_t filterIndex) const;
        void SetFilterEnabled(int32_t filterIndex, bool isEnabled);
        bool IsFilterEnabled(int32_t filterIndex) const;
        void SetFilterParams(int32_t filterIndex, const AZStd::vector<float>& params);
        void GetFilterParams(int32_t filterIndex, AZStd::vector<float>& params) const;
        void SetFilterParam(int32_t filterIndex, int32_t paramIndex, float paramValue);
        bool SetFilterParam(int32_t filterIndex, const AZ::Name& paramName, float paramValue, int32_t* paramIndex = nullptr);

        void GatherBusData(BusData& busData) const;

        void SetVisualizationEnabled(bool isVisualizationEnabled);
        AZStd::vector<float> GetPeakVolumeOfChannels();

    private:
        void ApplyMonoState();
        void ApplyFilterParamValues(int32_t filterIndex);

        AudioBusManager& m_manager;
        SoLoud::Soloud& m_soloud;
        AZStd::array<AudioFilter*, NumberOfFiltersPerBus> m_filters;
        AZStd::array<bool, NumberOfFiltersPerBus> m_filterEnabled;
        AZStd::array<AZStd::vector<float>, NumberOfFiltersPerBus> m_filterParamValues;
        AZ::Name m_name;
        AZ::Name m_outputBusName;
        SoLoud::Bus m_bus;
        SoLoud::handle m_busHandle;
        int32_t m_numberOfChannels = 2;
        bool m_isActivated = false;
        bool m_isMuted = false;
        bool m_isMono = false;
    };

    class AudioBusManager : private AudioBusManagerRequestBus::Handler
    {
    public:
        constexpr static const int32_t InvalidIndex = -1;

        AUDIO_IMPL_CLASS_ALLOCATOR(AudioBusManager);
        AZ_DISABLE_COPY_MOVE(AudioBusManager)
        AudioBusManager(SoLoud::Soloud& soloud, AudioFilterManager& audioFilterManager);
        ~AudioBusManager();

        void Reset();

        AudioBus* CreateBus(const AZ::Name& busName);
        bool DestroyBus(const AZ::Name& busName);
        bool DestroyBus(int32_t busIndex);

        bool IsContainsBus(const AZ::Name& busName) const;
        int32_t GetBusIndexByName(const AZ::Name& busName) const;
        int32_t GetNumberOfBuses() const;
        AudioBus* GetBus(const AZ::Name& busName) const;
        AudioBus* GetBus(int32_t busIndex) const;
        AZStd::vector<AZ::Name> GetAudioBusNames() const;

        bool ChangeBusName(const AZ::Name& busName, const AZ::Name& newBusName);
        void ChangeBusIndex(int32_t busIndex, int32_t newBusIndex, bool& outputBusChanged);

        void ActivateAllBuses();
        void DeactivateAllBuses();

        bool LoadBusLayout(AZ::IO::PathView layoutFilePath);
        bool SaveBusLayout(AZ::IO::PathView layoutFilePath) const;
        void SetVisEnabled(bool isVisualizationEnabled);

        void GatherBusLayout(BusLayoutData& layout) const;

    private:
        // AudioBusManagerRequests
        void ResetAudioBusLayout() override;
        void LoadAudioBusLayout(AZ::IO::FixedMaxPath layoutFilePath) override;
        void SaveAudioBusLayout(AZ::IO::FixedMaxPath layoutFilePath) override;
        void SetVisualizationEnabled(bool isEnabled) override;
        void AddAudioBus(AZ::Name busName) override;
        void RemoveAudioBus(AZ::Name busName) override;
        void ChangeAudioBusName(AZ::Name busName, AZ::Name newName) override;
        void ChangeAudioBusIndex(AZ::Name busName, int32_t newBusIndex) override;
        void SetAudioBusVolumeDb(AZ::Name busName, float volume) override;
        void SetAudioBusVolumeLinear(AZ::Name busName, float volume) override;
        void SetAudioBusMuted(AZ::Name busName, bool isMuted) override;
        void SetAudioBusMono(AZ::Name busName, bool isMono) override;
        void SetAudioBusOutput(AZ::Name busName, AZ::Name outputBusName) override;
        void SetAudioBusFilter(AZ::Name busName, int32_t filterIndex, AZ::Name filterName) override;
        void SetAudioBusFilterEnabled(AZ::Name busName, int32_t filterIndex, bool isEnabled) override;
        void SetAudioBusFilterParams(AZ::Name busName, int32_t filterIndex, AZStd::vector<float> params) override;
        void SetAudioBusFilterParam(AZ::Name busName, int32_t filterIndex, AZ::Name paramName, float paramValue) override;
        void RequestAudioBusLayout() override;
        void RequestAudioBusNames() override;
        void RequestAudioFilters() override;
        void RequestAudioBusPeakVolumes() override;
        void RegisterAudioFilter(AZ::Name filterName, AudioFilterCreatorFunc filterCreator) override;
        // ~AudioBusManagerRequests

        SoLoud::Soloud& m_soloud;
        AudioFilterManager& m_audioFilterManager;
        AZStd::vector<AZStd::unique_ptr<AudioBus>> m_buses;
        bool m_isVisualizationEnabled = false;
    };
} // namespace Audio
