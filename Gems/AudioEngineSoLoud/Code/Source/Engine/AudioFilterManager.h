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
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

#include <EditorEngineInterop.h>
#include <Util.h>
#include <soloud.h>

namespace Audio
{
    class AudioFilter
    {
    public:
        AZ_DISABLE_COPY(AudioFilter);
        AudioFilter(const AZ::Name& name, SoLoud::Filter* sofilter);
        ~AudioFilter() = default;

        const AZ::Name& GetName() const;
        void GetData(FilterData& data) const;
        int32_t GetParamIndexByName(const AZ::Name& paramName) const;
        SoLoud::Filter* GetSoloudFilter() const;

    private:
        AZ::Name m_name;
        AZStd::unique_ptr<SoLoud::Filter, AudioDeleter<SoLoud::Filter>> m_sofilter;
    };

    class AudioFilterManager
    {
    public:
        AZ_DISABLE_COPY_MOVE(AudioFilterManager)
        AudioFilterManager() = default;
        ~AudioFilterManager() = default;

        void Reset();

        template<class T>
        bool RegisterFilter(const AZ::Name& filterName);

        bool RegisterFilter(const AZ::Name& filterName, SoLoud::Filter* filter);

        AudioFilter* GetFilter(const AZ::Name& filterName);
        bool GetFilterData(const AZ::Name& filterName, FilterData& data);
        AZStd::vector<FilterData> GetAllFiltersData();

    private:
        void RegisterSoLoudBuiltinFilters();

        AZStd::unordered_map<AZ::Name, AudioFilter> m_filters; // Key - filter name.
    };

    template<class T>
    inline bool AudioFilterManager::RegisterFilter(const AZ::Name& filterName)
    {
        SoLoud::Filter* sofilter = azcreate(T, (), Audio::AudioImplAllocator);
        if (!sofilter || !RegisterFilter(filterName, sofilter))
        {
            azdestroy(sofilter, AudioImplAllocator, T);
            return false;
        }

        return true;
    }
} // namespace Audio
