/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/StringFunc/StringFunc.h>

#include <AudioFilterManager.h>
#include <Util.h>

#include <soloud_bassboostfilter.h>
#include <soloud_biquadresonantfilter.h>
#include <soloud_dcremovalfilter.h>
#include <soloud_echofilter.h>
#include <soloud_eqfilter.h>
#include <soloud_flangerfilter.h>
#include <soloud_freeverbfilter.h>
#include <soloud_lofifilter.h>
#include <soloud_robotizefilter.h>
#include <soloud_waveshaperfilter.h>

namespace Audio
{
    AudioFilter::AudioFilter(const AZ::Name& name, SoLoud::Filter* sofilter)
        : m_name(name)
        , m_sofilter(sofilter)
    {
        AZ_Assert(!name.IsEmpty(), "");
    }

    const AZ::Name& AudioFilter::GetName() const
    {
        return m_name;
    }

    void AudioFilter::GetData(FilterData& data) const
    {
        data.m_name = m_name;

        SoLoud::Filter* soloudFilter = GetSoloudFilter();

        int paramCount = soloudFilter->getParamCount();
        data.m_params.resize(paramCount);

        AZStd::unique_ptr<SoLoud::FilterInstance> tmpInst(m_sofilter->createInstance());

        for (int i = 0; i < paramCount; ++i)
        {
            FilterParamData& paramInfo = data.m_params[i];

            paramInfo.m_name = soloudFilter->getParamName(i);
            paramInfo.m_type = static_cast<FilterParamType::Type>(soloudFilter->getParamType(i));
            paramInfo.m_minValue = soloudFilter->getParamMin(i);
            paramInfo.m_maxValue = soloudFilter->getParamMax(i);
            paramInfo.m_value = tmpInst->getFilterParameter(i);
        }
    }

    int32_t AudioFilter::GetParamIndexByName(const AZ::Name& paramName) const
    {
        SoLoud::Filter* soloudFilter = GetSoloudFilter();
        int paramCount = soloudFilter->getParamCount();

        int paramIndex = 0;
        for (; paramIndex < paramCount; ++paramIndex)
        {
            if (AZ::StringFunc::Equal(paramName.GetCStr(), soloudFilter->getParamName(paramIndex)))
            {
                break;
            }
        }

        return (paramIndex < paramCount) ? paramIndex : -1;
    }

    SoLoud::Filter* AudioFilter::GetSoloudFilter() const
    {
        return m_sofilter.get();
    }

    bool AudioFilterManager::RegisterFilter(const AZ::Name& filterName, SoLoud::Filter* filter)
    {
        AZ_Assert(filter, "");

        if (filterName.IsEmpty())
        {
            AZ_Error(LogWindow, false, "Unable to register a filter with empty name.");
            return false;
        }

        if (m_filters.contains(filterName))
        {
            AZ_Error(
                LogWindow, false, "Unable to register filter \"%s\" because another filter with the same name already exists.",
                filterName.GetCStr());
            return false;
        }

        m_filters.try_emplace(filterName, filterName, filter);
        return true;
    }

    AudioFilter* AudioFilterManager::GetFilter(const AZ::Name& filterName)
    {
        auto it = m_filters.find(filterName);
        if (it == m_filters.end())
        {
            return nullptr;
        }

        return &(it->second);
    }

    bool AudioFilterManager::GetFilterData(const AZ::Name& filterName, FilterData& data) 
    {
        AudioFilter* filter = GetFilter(filterName);
        if (!filter)
        {
            return false;
        }

        filter->GetData(data);
        return true;
    }

    AZStd::vector<FilterData> AudioFilterManager::GetAllFiltersData()
    {
        AZStd::vector<FilterData> filtersData;
        filtersData.reserve(m_filters.size());

        for (const auto& pair : m_filters)
        {
            filtersData.push_back();
            pair.second.GetData(filtersData.back());
        }

        return AZStd::move(filtersData);
    }

    void AudioFilterManager::Reset()
    {
        m_filters.clear();
        RegisterSoLoudBuiltinFilters();
    }

    void AudioFilterManager::RegisterSoLoudBuiltinFilters()
    {
        [[maybe_unused]] bool filterCreated = false;
        constexpr const char* errorMessage = "Unable to register on of the builtin SoLoud filters";

        filterCreated = RegisterFilter<SoLoud::BassboostFilter>(AZ::Name("Bass boost filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::BiquadResonantFilter>(AZ::Name("Biquad resonant filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::DCRemovalFilter>(AZ::Name("DC-removal filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::EchoFilter>(AZ::Name("Echo filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::EqFilter>(AZ::Name("Equalizer filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::FlangerFilter>(AZ::Name("Flanger filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::FreeverbFilter>(AZ::Name("Freeverb filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::LofiFilter>(AZ::Name("LO-FI filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::RobotizeFilter>(AZ::Name("Robotize filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);

        filterCreated = RegisterFilter<SoLoud::WaveShaperFilter>(AZ::Name("Wave shaper filter"));
        AZ_Error(LogWindow, filterCreated, errorMessage);
    }
} // namespace Audio
