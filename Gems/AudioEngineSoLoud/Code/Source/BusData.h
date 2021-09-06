/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/IO/Path/Path.h>
#include <AzCore/Name/Name.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/XML/rapidxml.h>
#include <AzCore/std/containers/array.h>
#include <AzCore/std/containers/vector.h>

#include <Config.h>

namespace Audio
{
    namespace FilterParamType
    {
        // Must be in synch with SoLoud::Filter::PARAMTYPE.
        enum Type
        {
            Float,
            Int,
            Bool,

            Count
        };

        constexpr static const char* Tag = "Type";

        const char* ToString(Type type);
        Type FromString(const char* str);
    } // namespace FilterParamType

    struct FilterParamData
    {
        AZ_TYPE_INFO(FilterParamData, "{FB1B2CFC-A2E7-4391-9287-3AEE6FACC487}");

        FilterParamData() = default;
        ~FilterParamData() = default;

        void ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;

        constexpr static const char* Tag = "Parameter";
        constexpr static const char* NameTag = "Name";
        constexpr static const char* MinValueTag = "MinValue";
        constexpr static const char* MaxValueTag = "MaxValue";
        constexpr static const char* ValueTag = "Value";

        AZ::Name m_name;
        FilterParamType::Type m_type = FilterParamType::Float;
        float m_minValue = 0.0f;
        float m_maxValue = 0.0f;
        float m_value = 0.0f;
    };

    struct FilterData
    {
        AZ_TYPE_INFO(FilterData, "{00EEB63E-C89C-4342-88B3-9567CA2B79B7}");

        FilterData() = default;
        ~FilterData() = default;

        void ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;

        constexpr static const char* Tag = "Filter";
        constexpr static const char* NameTag = "Name";
        constexpr static const char* IsEnabledTag = "IsEnabled";

        AZ::Name m_name;
        AZStd::vector<FilterParamData> m_params;
        bool m_isEnabled = true;
    };

    struct FilterBlockData
    {
        AZ_TYPE_INFO(FilterBlockData, "{34B5E685-3634-42BD-BAA3-5A19B0CFF3CB}");

        FilterBlockData() = default;
        ~FilterBlockData() = default;

        void ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;

        constexpr static const char* Tag = "FilterBlock";

        AZStd::array<FilterData, NumberOfFiltersPerBus> m_filters;
    };

    struct BusData
    {
        AZ_TYPE_INFO(BusData, "{C0DC2E46-1454-4AE2-A2EE-D191E06852E1}");

        BusData() = default;
        ~BusData() = default;

        void ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;

        constexpr static const char* Tag = "Bus";
        constexpr static const char* NameTag = "Name";
        constexpr static const char* OutputBusNameTag = "OutputBusName";
        constexpr static const char* VolumeTag = "Volume";
        constexpr static const char* IsMutedTag = "Muted";
        constexpr static const char* IsMonoTag = "Mono";

        AZ::Name m_name;
        AZ::Name m_outputBusName;
        float m_volume = 0.0f;
        bool m_isMuted = false;
        bool m_isMono = false;

        FilterBlockData m_filterBlock;
    };

    struct BusLayoutData
    {
        AZ_TYPE_INFO(BusLayoutData, "{D9141ECE-E9BE-484C-B74D-5520BE13CCD0}");

        BusLayoutData() = default;
        ~BusLayoutData() = default;

        static void Reflect(AZ::ReflectContext* context);
        bool Save(AZ::IO::PathView filePath);
        bool Load(AZ::IO::PathView filePath);
        void ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
        void WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const;

        constexpr static const char* Tag = "BusLayoutData";

        AZStd::vector<BusData> m_buses;
    };
} // namespace Audio
