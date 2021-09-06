/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioBusManager.h>
#include <AudioFilterManager.h>
#include <Util.h>

namespace Audio
{
    AudioBus::AudioBus(AudioBusManager& manager, SoLoud::Soloud& soloud, const AZ::Name& name)
        : m_manager(manager)
        , m_soloud(soloud)
        , m_name(name)
    {
        AZ_Assert(!name.IsEmpty(), "");

        if (name != AZ::Name(MasterBusName))
        {
            m_outputBusName = MasterBusName;
        }

        m_filters.fill(nullptr);
        m_filterEnabled.fill(true);
    }

    void AudioBus::Activate()
    {
        if (m_isActivated)
        {
            return;
        }

        if (m_name == AZ::Name(MasterBusName))
        {
            m_busHandle = m_soloud.playBackground(m_bus);
        }
        else
        {
            AudioBus* outputBus = m_manager.GetBus(m_outputBusName);
            if (!outputBus)
            {
                AZ_Assert(
                    false, "Unable to activate the bus '%s' because the output bus '%s' doesn't exist.", m_name.GetCStr(),
                    m_outputBusName.GetCStr());
                return;
            }

            m_busHandle = outputBus->PlayBackground(m_bus);
        }

        // Dirty-hack-zone begin.
        // This is needed to fix the bug "Bus stop causes all buses to stop" https://github.com/jarikomppa/soloud/issues/280.
        m_bus.findBusHandle();
        // Dirty-hack-zone end.

        if (m_isMuted)
        {
            m_soloud.setVolume(m_busHandle, 0.0f);
        }
        else
        {
            m_soloud.setVolume(m_busHandle, m_bus.mVolume);
        }

        // ApplyMonoState();

        m_isActivated = true;

        for (int32_t i = 0; i < NumberOfFiltersPerBus; ++i)
        {
            ApplyFilterParamValues(i);
        }
    }

    void AudioBus::Deactivate()
    {
        if (!m_isActivated)
        {
            return;
        }

        m_bus.stop();
        m_isActivated = false;
    }

    bool AudioBus::IsActivated() const
    {
        return m_isActivated;
    }

    bool AudioBus::SetName(const AZ::Name& newName)
    {
        return m_manager.ChangeBusName(m_name, newName);
    }

    const AZ::Name& AudioBus::GetName() const
    {
        return m_name;
    }

    bool AudioBus::SetOutputBusName(const AZ::Name& outputBusName)
    {
        if (outputBusName == m_outputBusName)
        {
            return true;
        }

        if (m_name == outputBusName)
        {
            AZ_Error(
                LogWindow, false,
                "Unable to change the output bus of bus '%s' to '%s' because provided output name and bus name are the same.",
                m_name.GetCStr(), outputBusName.GetCStr());
            return false;
        }

        if (m_name == AZ::Name(MasterBusName))
        {
            AZ_Error(LogWindow, false, "It's forbidden to change the output of the master bus.");
            return false;
        }

        AudioBus* outputBus = m_manager.GetBus(outputBusName);
        if (!outputBus)
        {
            AZ_Error(
                LogWindow, false, "Unable to change the output bus of bus '%s' to '%s' because the bus with this name doesn't exist.",
                m_name.GetCStr(), outputBusName.GetCStr());
            return false;
        }

        m_outputBusName = outputBusName;

        if (m_isActivated)
        {
            outputBus->m_bus.annexSound(m_busHandle);
        }

        return true;
    }

    const AZ::Name& AudioBus::GetOutputBusName() const
    {
        return m_outputBusName;
    }

    void AudioBus::SetVolumeDb(float volume)
    {
        m_bus.setVolume(DbToLinear(volume));
        m_soloud.setVolume(m_busHandle, m_bus.mVolume);
    }

    float AudioBus::GetVolumeDb() const
    {
        return LinearToDb(m_bus.mVolume);
    }

    void AudioBus::SetVolumeLinear(float volume)
    {
        m_bus.setVolume(volume);
        m_soloud.setVolume(m_busHandle, volume);
    }

    float AudioBus::GetVolumeLinear() const
    {
        return m_bus.mVolume;
    }

    void AudioBus::SetMuted(bool isMuted)
    {
        if (isMuted == m_isMuted)
        {
            return;
        }

        m_isMuted = isMuted;

        if (isMuted)
        {
            m_soloud.setVolume(m_busHandle, 0.0f);
        }
        else
        {
            m_soloud.setVolume(m_busHandle, m_bus.mVolume);
        }
    }

    bool AudioBus::IsMuted() const
    {
        return m_isMuted;
    }

    void AudioBus::SetMono(bool isMono)
    {
        if (isMono == m_isMono)
        {
            return;
        }

        m_isMono = isMono;

        ApplyMonoState();

        if (m_isActivated)
        {
            m_manager.DeactivateAllBuses();
            m_manager.ActivateAllBuses();
        }
    }

    bool AudioBus::IsMono() const
    {
        return m_isMono;
    }

    SoLoud::handle AudioBus::Play(SoLoud::AudioSource& sound, float volume, float pan, bool paused)
    {
        return m_bus.play(sound, volume, pan, paused);
    }

    SoLoud::handle AudioBus::PlayClocked(SoLoud::time soundTime, SoLoud::AudioSource& sound, float volume, float pan)
    {
        return m_bus.playClocked(soundTime, sound, volume, pan);
    }

    SoLoud::handle AudioBus::Play3d(
        SoLoud::AudioSource& sound, float posX, float posY, float posZ, float velX, float velY, float velZ, float volume, bool paused)
    {
        return m_bus.play3d(sound, posX, posY, posZ, velX, velY, velZ, volume, paused);
    }

    SoLoud::handle AudioBus::Play3dClocked(
        SoLoud::time soundTime,
        SoLoud::AudioSource& sound,
        float posX,
        float posY,
        float posZ,
        float velX,
        float velY,
        float velZ,
        float volume)
    {
        return m_bus.play3dClocked(soundTime, sound, posX, posY, posZ, velX, velY, velZ, volume);
    }

    SoLoud::handle AudioBus::PlayBackground(SoLoud::AudioSource& sound, float volume, bool paused)
    {
        return m_soloud.playBackground(sound, volume, paused, m_bus.mChannelHandle);
    }

    AZStd::vector<float> AudioBus::GetPeakVolumeOfChannels()
    {
        SoLoud::BusInstance* busInst = m_bus.mInstance;
        if (!busInst)
        {
            return AZStd::vector<float>();
        }

        int32_t numberOfChannels = busInst->mChannels;

        AZStd::vector<float> volumes(numberOfChannels);
        for (int32_t i = 0; i < numberOfChannels; ++i)
        {
            volumes[i] = m_bus.getApproximateVolume(i);

            // This is necessary because getApproximateVolume() returns the peak volume of samples before applying bus's own volume.
            volumes[i] *= busInst->mCurrentChannelVolume[i];
        }

        return AZStd::move(volumes);
    }

    void AudioBus::SetFilter(int32_t filterIndex, AudioFilter* filter)
    {
        AZ_Assert(filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus, "");

        m_bus.setFilter(filterIndex, (filter && m_filterEnabled[filterIndex]) ? filter->GetSoloudFilter() : nullptr);
        m_filters[filterIndex] = filter;

        if (filter)
        {
            SoLoud::Filter* slfilter = filter->GetSoloudFilter();
            m_filterParamValues[filterIndex].resize(slfilter->getParamCount());

            if (m_isActivated && m_filterEnabled[filterIndex])
            {
                for (int i = 0; i < slfilter->getParamCount(); ++i)
                {
                    m_filterParamValues[filterIndex][i] = m_soloud.getFilterParameter(m_busHandle, filterIndex, i);
                }

                ApplyFilterParamValues(filterIndex);
            }
            else
            {
                AZStd::unique_ptr<SoLoud::FilterInstance> tmpInstance(slfilter->createInstance());
                for (int i = 0; i < slfilter->getParamCount(); ++i)
                {
                    m_filterParamValues[filterIndex][i] = tmpInstance->getFilterParameter(i);
                }
            }
        }
        else
        {
            m_filterParamValues[filterIndex].clear();
        }
    }

    AudioFilter* AudioBus::GetFilter(int32_t filterIndex) const
    {
        AZ_Assert(filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus, "");
        return m_filters[filterIndex];
    }

    void AudioBus::SetFilterEnabled(int32_t filterIndex, bool isEnabled)
    {
        AZ_Assert(filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus, "");

        if (m_filterEnabled[filterIndex] == isEnabled)
        {
            return;
        }

        m_filterEnabled[filterIndex] = isEnabled;

        if (isEnabled && m_filters[filterIndex])
        {
            m_bus.setFilter(filterIndex, m_filters[filterIndex]->GetSoloudFilter());
            ApplyFilterParamValues(filterIndex);
        }
        else
        {
            m_bus.setFilter(filterIndex, nullptr);
        }
    }

    bool AudioBus::IsFilterEnabled(int32_t filterIndex) const
    {
        AZ_Assert(filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus, "");
        return m_filterEnabled[filterIndex];
    }

    void AudioBus::SetFilterParams(int32_t filterIndex, const AZStd::vector<float>& params)
    {
        AZ_Assert(filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus, "");

        auto& paramValues = m_filterParamValues[filterIndex];
        AZ_Assert(params.size() == paramValues.size(), "");

        m_filterParamValues[filterIndex] = params;
        ApplyFilterParamValues(filterIndex);
    }

    void AudioBus::GetFilterParams(int32_t filterIndex, AZStd::vector<float>& params) const
    {
        AZ_Assert(filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus, "");
        params = m_filterParamValues[filterIndex];
    }

    void AudioBus::SetFilterParam(int32_t filterIndex, int32_t paramIndex, float paramValue)
    {
        AZ_Assert(filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus, "");

        auto& params = m_filterParamValues[filterIndex];
        AZ_Assert(paramIndex >= 0 && paramIndex < params.size(), "");

        params[paramIndex] = paramValue;
        ApplyFilterParamValues(filterIndex);
    }

    bool AudioBus::SetFilterParam(int32_t filterIndex, const AZ::Name& paramName, float paramValue, int32_t* paramIndex)
    {
        AZ_Assert(filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus, "");

        int32_t paramIdx = m_filters[filterIndex]->GetParamIndexByName(paramName);
        if (paramIndex)
        {
            *paramIndex = paramIdx;
        }

        if (paramIdx == -1)
        {
            AZ_Error(
                LogWindow, false, "Unable to set a filter param of bus '%s' because '%s' parameter doesn't exist.", m_name.GetCStr(),
                paramName.GetCStr());

            return false;
        }

        m_filterParamValues[filterIndex][paramIdx] = paramValue;
        ApplyFilterParamValues(filterIndex);
        return true;
    }

    void AudioBus::GatherBusData(BusData& busData) const
    {
        busData.m_name = GetName();
        busData.m_outputBusName = GetOutputBusName();
        busData.m_volume = GetVolumeDb();
        busData.m_isMuted = IsMuted();
        busData.m_isMono = IsMono();

        for (int32_t filterIndex = 0; filterIndex < NumberOfFiltersPerBus; ++filterIndex)
        {
            busData.m_filterBlock.m_filters[filterIndex].m_isEnabled = m_filterEnabled[filterIndex];

            AudioFilter* filter = m_filters[filterIndex];
            if (!filter)
            {
                continue;
            }

            filter->GetData(busData.m_filterBlock.m_filters[filterIndex]);

            AZStd::vector<float> filterParams;
            GetFilterParams(filterIndex, filterParams);

            for (size_t paramIndex = 0; paramIndex < filterParams.size(); ++paramIndex)
            {
                busData.m_filterBlock.m_filters[filterIndex].m_params[paramIndex].m_value = filterParams[paramIndex];
            }
        }
    }

    void AudioBus::SetVisualizationEnabled(bool isVisualizationEnabled)
    {
        m_bus.setVisualizationEnable(isVisualizationEnabled);
    }

    void AudioBus::ApplyMonoState()
    {
        // Dirty-hack-zone begin.
        // if (m_bus.mInstance)
        //{
        //    m_soloud.lockAudioMutex_internal();

        //    if (m_isMono)
        //    {
        //        m_bus.mInstance->mChannels = 1;
        //    }
        //    else
        //    {
        //        m_bus.mInstance->mChannels = m_bus.mChannels;
        //    }

        //    //m_soloud.mActiveVoiceDirty = true;
        //    m_soloud.unlockAudioMutex_internal();
        //}
        // Dirty-hack-zone end.

        if (m_isMono)
        {
            m_bus.setChannels(1);
        }
        else
        {
            m_bus.setChannels(m_numberOfChannels);
        }
    }

    void AudioBus::ApplyFilterParamValues(int32_t filterIndex)
    {
        if (!m_isActivated)
        {
            return;
        }

        const auto& values = m_filterParamValues[filterIndex];
        for (int32_t i = 0; i < values.size(); ++i)
        {
            m_soloud.setFilterParameter(m_busHandle, filterIndex, i, values[i]);
        }
    }

    AudioBusManager::AudioBusManager(SoLoud::Soloud& soloud, AudioFilterManager& audioFilterManager)
        : m_soloud(soloud)
        , m_audioFilterManager(audioFilterManager)
    {
        Reset();

        AudioBusManagerRequestBus::Handler::BusConnect();
    }

    AudioBusManager::~AudioBusManager()
    {
        AudioBusManagerRequestBus::Handler::BusDisconnect();
    }

    void AudioBusManager::Reset()
    {
        m_buses.clear();

        [[maybe_unused]] bool isMasterBusCreated = CreateBus(AZ::Name(MasterBusName));
        AZ_Assert(isMasterBusCreated, "Unable to create the Master audio bus.");
    }

    AudioBus* AudioBusManager::CreateBus(const AZ::Name& busName)
    {
        if (IsContainsBus(busName))
        {
            AZ_Error(
                LogWindow, false, "Unable to create the bus \"%s\" because the manager already contains a bus with this name",
                busName.GetCStr());
            return nullptr;
        }

        auto bus = AZStd::make_unique<AudioBus>(*this, m_soloud, busName);
        if (!bus)
        {
            return nullptr;
        }

        bus->SetVisualizationEnabled(m_isVisualizationEnabled);
        m_buses.emplace_back(AZStd::move(bus));
        return m_buses.back().get();
    }

    bool AudioBusManager::DestroyBus(const AZ::Name& busName)
    {
        if (busName == AZ::Name(MasterBusName))
        {
            AZ_Error(LogWindow, false, "The Master bus can't be destroyed.");
            return false;
        }

        const int32_t busIndex = GetBusIndexByName(busName);
        if (busIndex == InvalidIndex)
        {
            return false;
        }

        return DestroyBus(busIndex);
    }

    bool AudioBusManager::DestroyBus(int32_t busIndex)
    {
        AZ_Assert(busIndex >= 0 && busIndex < m_buses.size(), "");

        if (busIndex == 0)
        {
            AZ_Error(LogWindow, false, "The Master bus can't be destroyed.");
            return false;
        }

        AZ::Name deletedBusName = m_buses[busIndex]->GetName();
        for (auto& ptr : m_buses)
        {
            if (ptr->GetOutputBusName() == deletedBusName)
            {
                ptr->SetOutputBusName(AZ::Name(MasterBusName));
            }
        }

        m_buses.erase(m_buses.begin() + busIndex);
        return true;
    }

    bool AudioBusManager::IsContainsBus(const AZ::Name& busName) const
    {
        return GetBusIndexByName(busName) != InvalidIndex;
    }

    int32_t AudioBusManager::GetBusIndexByName(const AZ::Name& busName) const
    {
        for (int32_t busIndex = 0; busIndex < m_buses.size(); ++busIndex)
        {
            if (m_buses[busIndex]->GetName() == busName)
            {
                return busIndex;
            }
        }

        return InvalidIndex;
    }

    int32_t AudioBusManager::GetNumberOfBuses() const
    {
        return aznumeric_cast<int32_t>(m_buses.size());
    }

    AudioBus* AudioBusManager::GetBus(const AZ::Name& busName) const
    {
        int32_t busIndex = GetBusIndexByName(busName);
        if (busIndex == InvalidIndex)
        {
            return nullptr;
        }

        return m_buses[busIndex].get();
    }

    AudioBus* AudioBusManager::GetBus(int32_t busIndex) const
    {
        AZ_Assert(busIndex >= 0 && busIndex < m_buses.size(), "");

        return m_buses[busIndex].get();
    }

    AZStd::vector<AZ::Name> AudioBusManager::GetAudioBusNames() const
    {
        AZStd::vector<AZ::Name> busNames;
        busNames.reserve(m_buses.size());

        for (auto& bus : m_buses)
        {
            busNames.push_back(bus->GetName());
        }

        return AZStd::move(busNames);
    }

    bool AudioBusManager::ChangeBusName(const AZ::Name& busName, const AZ::Name& newBusName)
    {
        if (busName == newBusName)
        {
            return false;
        }

        if (newBusName.IsEmpty())
        {
            AZ_Error(LogWindow, false, "Unable to rename bus \"%s\" because the name supplied is empty.", busName.GetCStr());
            return false;
        }

        if (busName == AZ::Name(MasterBusName))
        {
            AZ_Error(LogWindow, false, "The Master bus can't be renamed.");
            return false;
        }

        if (newBusName == AZ::Name(MasterBusName))
        {
            AZ_Error(LogWindow, false, "There can be only one Master bus.");
            return false;
        }

        if (IsContainsBus(newBusName))
        {
            AZ_Error(
                LogWindow, false, "Unable to change the name of the bus from \"%s\" to \"%s\". A bus with this name already exists.",
                busName.GetCStr(), newBusName.GetCStr());
            return false;
        }

        AudioBus* bus = GetBus(busName);
        if (!bus)
        {
            AZ_Error(LogWindow, false, "Unable to rename bus \"%s\" because it doesn't exist.", busName.GetCStr());
            return false;
        }

        bus->m_name = newBusName;

        for (auto& ptr : m_buses)
        {
            if (ptr->GetOutputBusName() == busName)
            {
                ptr->SetOutputBusName(newBusName);
            }
        }

        return true;
    }

    void AudioBusManager::ChangeBusIndex(int32_t busIndex, int32_t newBusIndex, bool& outputBusChanged)
    {
        AZ_Assert(busIndex >= 0 && busIndex < m_buses.size(), "");
        AZ_Assert(newBusIndex >= 0 && newBusIndex < m_buses.size(), "");
        AZ_Assert(busIndex != newBusIndex, "");

        if (busIndex == 0 || newBusIndex == 0)
        {
            AZ_Error(LogWindow, false, "The Master bus can't be repositioned.");
            return;
        }

        int32_t outputBusIndex = GetBusIndexByName(m_buses[busIndex]->GetOutputBusName());
        if (newBusIndex <= outputBusIndex)
        {
            m_buses[busIndex]->SetOutputBusName(AZ::Name(MasterBusName));
            outputBusChanged = true;
        }
        else
        {
            outputBusChanged = false;
        }

        auto busptr = AZStd::move(m_buses[busIndex]);
        m_buses.erase(m_buses.begin() + busIndex);
        m_buses.emplace(m_buses.begin() + newBusIndex, AZStd::move(busptr));
    }

    void AudioBusManager::ActivateAllBuses()
    {
        for (auto& bus : m_buses)
        {
            bus->Activate();
        }
    }

    void AudioBusManager::DeactivateAllBuses()
    {
        for (auto& bus : m_buses)
        {
            bus->Deactivate();
        }
    }

    bool AudioBusManager::LoadBusLayout(AZ::IO::PathView layoutFilePath)
    {
        BusLayoutData layout;
        if (!layout.Load(layoutFilePath))
        {
            return false;
        }

        Reset();

        for (const auto& busData : layout.m_buses)
        {
            AudioBus* bus = nullptr;
            bool isMasterBus = busData.m_name == AZ::Name(MasterBusName);
            if (isMasterBus)
            {
                bus = GetBus(AZ::Name(MasterBusName));
            }
            else
            {
                bus = CreateBus(busData.m_name);
            }

            if (!bus)
            {
                Reset();
                return false;
            }

            if (!isMasterBus)
            {
                if (!bus->SetOutputBusName(busData.m_outputBusName))
                {
                    Reset();
                    return false;
                }
            }

            bus->SetVolumeDb(busData.m_volume);
            bus->SetMuted(busData.m_isMuted);
            bus->SetMono(busData.m_isMono);

            for (int32_t filterIndex = 0; filterIndex < NumberOfFiltersPerBus && filterIndex < busData.m_filterBlock.m_filters.size();
                 ++filterIndex)
            {
                const auto& filterData = busData.m_filterBlock.m_filters[filterIndex];
                bus->SetFilterEnabled(filterIndex, filterData.m_isEnabled);

                if (filterData.m_name.IsEmpty())
                {
                    continue;
                }

                AudioFilter* filter = m_audioFilterManager.GetFilter(filterData.m_name);
                if (!filter)
                {
                    AZ_Error(
                        LogWindow, false, "Unable to load bus layout '%s' because filter '%s' doesn't exist.", layoutFilePath.Native().data(),
                        filterData.m_name.GetCStr());
                    Reset();
                    return false;
                }

                bus->SetFilter(filterIndex, filter);
                AZStd::vector<float> filterParams;
                bus->GetFilterParams(filterIndex, filterParams);

                for (int32_t paramIndex = 0; paramIndex < filterData.m_params.size(); ++paramIndex)
                {
                    const auto& paramData = filterData.m_params[paramIndex];

                    int32_t realParamIndex = filter->GetParamIndexByName(paramData.m_name);
                    if (realParamIndex == -1)
                    {
                        AZ_Error(
                            LogWindow, false, "Unable to load bus layout '%s' because filter '%s' doesn't have '%s' parameter.",
                            layoutFilePath.Native().data(), paramData.m_name.GetCStr());
                        Reset();
                        return false;
                    }

                    filterParams[realParamIndex] = paramData.m_value;
                }

                bus->SetFilterParams(filterIndex, filterParams);
            }
        }

        return true;
    }

    bool AudioBusManager::SaveBusLayout(AZ::IO::PathView layoutFilePath) const
    {
        BusLayoutData layout;
        GatherBusLayout(layout);
        return layout.Save(layoutFilePath);
    }

    void AudioBusManager::SetVisEnabled(bool isEnabled)
    {
        if (m_isVisualizationEnabled == isEnabled)
        {
            return;
        }

        for (const auto& pair : m_buses)
        {
            pair->SetVisualizationEnabled(isEnabled);
        }

        m_isVisualizationEnabled = isEnabled;
    }

    void AudioBusManager::GatherBusLayout(BusLayoutData& layout) const
    {
        const int32_t numberOfBusses = GetNumberOfBuses();
        layout.m_buses.clear();
        layout.m_buses.reserve(numberOfBusses);

        for (int32_t busIndex = 0; busIndex < GetNumberOfBuses(); ++busIndex)
        {
            AudioBus* bus = GetBus(busIndex);

            BusData busData;
            bus->GatherBusData(busData);
            layout.m_buses.emplace_back(busData);
        }
    }

    void AudioBusManager::ResetAudioBusLayout()
    {
        Reset();
        ActivateAllBuses();

        BusLayoutData busLayout;
        GatherBusLayout(busLayout);
        AudioBusManagerNotificationBus::QueueBroadcast(&AudioBusManagerNotificationBus::Events::OnUpdateAudioBusLayout, busLayout);

        AudioBusManagerNotificationBus::QueueBroadcast(&AudioBusManagerNotificationBus::Events::OnUpdateAudioBusNames, GetAudioBusNames());
    }

    void AudioBusManager::LoadAudioBusLayout(AZ::IO::FixedMaxPath layoutFilePath)
    {
        LoadBusLayout(layoutFilePath);
        ActivateAllBuses();

        BusLayoutData layout;
        GatherBusLayout(layout);
        AudioBusManagerNotificationBus::QueueBroadcast(&AudioBusManagerNotificationBus::Events::OnUpdateAudioBusLayout, layout);

        AudioBusManagerNotificationBus::QueueBroadcast(&AudioBusManagerNotificationBus::Events::OnUpdateAudioBusNames, GetAudioBusNames());
    }

    void AudioBusManager::SaveAudioBusLayout(AZ::IO::FixedMaxPath layoutFilePath)
    {
        SaveBusLayout(layoutFilePath);
    }

    void AudioBusManager::SetVisualizationEnabled(bool isVisualizationEnabled)
    {
        SetVisEnabled(isVisualizationEnabled);
    }

    void AudioBusManager::AddAudioBus(AZ::Name busName)
    {
        AudioBus* bus = CreateBus(busName);
        bool success = bus != nullptr;
        BusData busData;
        if (bus)
        {
            bus->Activate();
            bus->GatherBusData(busData);
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_AddAudioBus, success, busData);

        if (success)
        {
            AudioBusManagerNotificationBus::QueueBroadcast(
                &AudioBusManagerNotificationBus::Events::OnUpdateAudioBusNames, GetAudioBusNames());
        }
    }

    void AudioBusManager::RemoveAudioBus(AZ::Name busName)
    {
        bool success = DestroyBus(busName);
        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_RemoveAudioBus, success, busName);

        if (success)
        {
            AudioBusManagerNotificationBus::QueueBroadcast(
                &AudioBusManagerNotificationBus::Events::OnUpdateAudioBusNames, GetAudioBusNames());
        }
    }

    void AudioBusManager::ChangeAudioBusName(AZ::Name busName, AZ::Name newName)
    {
        bool success = ChangeBusName(busName, newName);
        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_ChangeAudioBusName, success, busName, newName);

        if (success)
        {
            AudioBusManagerNotificationBus::QueueBroadcast(
                &AudioBusManagerNotificationBus::Events::OnUpdateAudioBusNames, GetAudioBusNames());
        }
    }

    void AudioBusManager::ChangeAudioBusIndex(AZ::Name busName, int32_t newBusIndex)
    {
        bool success = false;
        int32_t oldBusIndex = GetBusIndexByName(busName);

        if (oldBusIndex == InvalidIndex)
        {
            AZ_Error(
                LogWindow, false, "Unable to change the index of bus \"%s\" because the bus with this name doesn't exist.",
                busName.GetCStr());
        }
        else
        {
            bool outputBusChanged = false;
            ChangeBusIndex(oldBusIndex, newBusIndex, outputBusChanged);
            if (outputBusChanged)
            {
                AudioBusManagerNotificationBus::QueueBroadcast(
                    &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusOutput, true, busName,
                    GetBus(busName)->GetOutputBusName());
            }

            success = true;
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_ChangeAudioBusIndex, success, busName, oldBusIndex, newBusIndex);

        if (success)
        {
            AudioBusManagerNotificationBus::QueueBroadcast(
                &AudioBusManagerNotificationBus::Events::OnUpdateAudioBusNames, GetAudioBusNames());
        }
    }

    void AudioBusManager::SetAudioBusVolumeDb(AZ::Name busName, float volume)
    {
        bool success = false;

        AudioBus* bus = GetBus(busName);
        if (!bus)
        {
            AZ_Error(
                LogWindow, false, "Unable to change the volume of bus \"%s\" because the bus with this name doesn't exist.",
                busName.GetCStr());
        }
        else
        {
            bus->SetVolumeDb(volume);
            success = true;
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusVolumeDb, success, busName, volume);
    }

    void AudioBusManager::SetAudioBusVolumeLinear(AZ::Name busName, float volume)
    {
        bool success = false;

        AudioBus* bus = GetBus(busName);
        if (!bus)
        {
            AZ_Error(
                LogWindow, false, "Unable to change the volume of bus \"%s\" because the bus with this name doesn't exist.",
                busName.GetCStr());
        }
        else
        {
            bus->SetVolumeLinear(volume);
            success = true;
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusVolumeLinear, success, busName, volume);
    }

    void AudioBusManager::SetAudioBusMuted(AZ::Name busName, bool isMuted)
    {
        bool success = false;

        AudioBus* bus = GetBus(busName);
        if (!bus)
        {
            AZ_Error(
                LogWindow, false, "Unable to mute/unmute bus \"%s\" because the bus with this name doesn't exist.", busName.GetCStr());
        }
        else
        {
            bus->SetMuted(isMuted);
            success = true;
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusMuted, success, busName, isMuted);
    }

    void AudioBusManager::SetAudioBusMono(AZ::Name busName, bool isMono)
    {
        bool success = false;

        AudioBus* bus = GetBus(busName);
        if (!bus)
        {
            AZ_Error(
                LogWindow, false, "Unable to mono/unmono bus \"%s\" because the bus with this name doesn't exist.", busName.GetCStr());
        }
        else
        {
            bus->SetMono(isMono);
            success = true;
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusMono, success, busName, isMono);
    }

    void AudioBusManager::SetAudioBusOutput(AZ::Name busName, AZ::Name outputBusName)
    {
        bool success = false;

        AudioBus* bus = GetBus(busName);
        if (!bus)
        {
            AZ_Error(
                LogWindow, false, "Unable to set the output of bus \"%s\" because the bus with this name doesn't exist.", busName.GetCStr());
        }
        else
        {
            success = bus->SetOutputBusName(outputBusName);
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusOutput, success, busName, outputBusName);
    }

    void AudioBusManager::SetAudioBusFilter(AZ::Name busName, int32_t filterIndex, AZ::Name filterName)
    {
        bool success = false;
        FilterData filterData;

        AudioBus* bus = GetBus(busName);
        if (bus)
        {
            if (filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus)
            {
                filterData.m_isEnabled = bus->IsFilterEnabled(filterIndex);

                if (filterName.IsEmpty())
                {
                    bus->SetFilter(filterIndex, nullptr);
                    success = true;
                }
                else
                {
                    AudioFilter* filter = m_audioFilterManager.GetFilter(filterName);
                    if (filter)
                    {
                        bus->SetFilter(filterIndex, filter);
                        bus->GetFilter(filterIndex)->GetData(filterData);

                        AZStd::vector<float> filterParams;
                        bus->GetFilterParams(filterIndex, filterParams);
                        for (size_t i = 0; i < filterParams.size(); ++i)
                        {
                            filterData.m_params[i].m_value = filterParams[i];
                        }

                        success = true;
                    }
                    else
                    {
                        AZ_Error(
                            LogWindow, false, "Unable to set a filter of bus '%s' because the filter '%s' doesn't exist.", busName.GetCStr(),
                            filterName.GetCStr());
                    }
                }
            }
            else
            {
                AZ_Error(
                    LogWindow, false, "Unable to set a filter of bus '%s' because supplied filter index is not valid.",
                    busName.GetCStr());
            }
        }
        else
        {
            AZ_Error(
                LogWindow, false, "Unable to set a filter of bus '%s' because the bus with this name doesn't exist.", busName.GetCStr());
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusFilter, success, busName, filterIndex, filterData);
    }

    void AudioBusManager::SetAudioBusFilterEnabled(AZ::Name busName, int32_t filterIndex, bool isEnabled)
    {
        bool success = false;

        AudioBus* bus = GetBus(busName);
        if (bus)
        {
            if (filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus)
            {
                bus->SetFilterEnabled(filterIndex, isEnabled);
                success = true;
            }
            else
            {
                AZ_Error(
                    LogWindow, false, "Unable to enable/disable a filter of bus '%s' because supplied filter index is not valid.",
                    busName.GetCStr());
            }
        }
        else
        {
            AZ_Error(
                LogWindow, false, "Unable to enable/disable a filter of bus '%s' because the bus with this name doesn't exist.",
                busName.GetCStr());
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusFilterEnabled, success, busName, filterIndex, isEnabled);
    }

    void AudioBusManager::SetAudioBusFilterParams(AZ::Name busName, int32_t filterIndex, AZStd::vector<float> params)
    {
        bool success = false;

        AudioBus* bus = GetBus(busName);
        if (bus)
        {
            if (filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus)
            {
                bus->SetFilterParams(filterIndex, params);
                success = true;
            }
            else
            {
                AZ_Error(
                    LogWindow, false, "Unable to set filter params of bus '%s' because supplied filter index is not valid.",
                    busName.GetCStr());
            }
        }
        else
        {
            AZ_Error(
                LogWindow, false, "Unable to set filter params of bus '%s' because the bus with this name doesn't exist.",
                busName.GetCStr());
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusFilterParams, success, busName, filterIndex, params);
    }

    void AudioBusManager::SetAudioBusFilterParam(AZ::Name busName, int32_t filterIndex, AZ::Name paramName, float paramValue)
    {
        bool success = false;
        int32_t paramIndex = AudioBusManager::InvalidIndex;

        AudioBus* bus = GetBus(busName);
        if (bus)
        {
            if (filterIndex >= 0 && filterIndex < NumberOfFiltersPerBus)
            {
                success = bus->SetFilterParam(filterIndex, paramName, paramValue, &paramIndex);
            }
            else
            {
                AZ_Error(
                    LogWindow, false, "Unable to set a filter param of bus '%s' bacause supplied filter index is not valid.",
                    busName.GetCStr());
            }
        }
        else
        {
            AZ_Error(
                LogWindow, false, "Unable to set a filter param of bus '%s' because the bus with this name doesn't exist.",
                busName.GetCStr());
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnRequestCompleted_SetAudioBusFilterParam, success, busName, filterIndex, paramName,
            paramIndex, paramValue);
    }

    void AudioBusManager::RequestAudioBusLayout()
    {
        BusLayoutData busLayout;
        GatherBusLayout(busLayout);
        AudioBusManagerNotificationBus::QueueBroadcast(&AudioBusManagerNotificationBus::Events::OnUpdateAudioBusLayout, busLayout);
    }

    void AudioBusManager::RequestAudioBusNames()
    {
        AudioBusManagerNotificationBus::QueueBroadcast(&AudioBusManagerNotificationBus::Events::OnUpdateAudioBusNames, GetAudioBusNames());
    }

    void AudioBusManager::RequestAudioFilters()
    {
        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnUpdateAudioFilters, m_audioFilterManager.GetAllFiltersData());
    }

    void AudioBusManager::RequestAudioBusPeakVolumes()
    {
        for (int32_t i = 0; i < GetNumberOfBuses(); ++i)
        {
            AudioBus* bus = GetBus(i);
            AudioBusManagerNotificationBus::QueueBroadcast(
                &AudioBusManagerNotificationBus::Events::OnUpdateAudioBusPeakVolumes, bus->GetName(), bus->GetPeakVolumeOfChannels());
        }
    }

    void AudioBusManager::RegisterAudioFilter(AZ::Name filterName, AudioFilterCreatorFunc filterCreator)
    {
        AZ_Assert(filterCreator, "");

        SoLoud::Filter* filter = filterCreator();
        if (!filter)
        {
            return;
        }

        if (!m_audioFilterManager.RegisterFilter(filterName, filterCreator()))
        {
            azdestroy(filter);
            return;
        }

        AudioBusManagerNotificationBus::QueueBroadcast(
            &AudioBusManagerNotificationBus::Events::OnUpdateAudioFilters, m_audioFilterManager.GetAllFiltersData());
    }
} // namespace Audio
