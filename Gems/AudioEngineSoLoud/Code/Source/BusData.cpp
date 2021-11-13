/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <BusData.h>
#include <Config.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/Json/JsonSerialization.h>
#include <AzCore/Serialization/Json/JsonUtils.h>
#include <AzCore/StringFunc/StringFunc.h>

namespace Audio
{
    const char* FilterParamType::ToString(Type type)
    {
        constexpr static const char* strings[FilterParamType::Count] = { "Float", "Int", "Bool" };

        AZ_Assert(type < FilterParamType::Count, "Invalid FilterParamType value!");
        return strings[type];
    }

    FilterParamType::Type FilterParamType::FromString(const char* str)
    {
        if (AZ::StringFunc::Equal(str, "Float"))
        {
            return FilterParamType::Float;
        }
        else if (AZ::StringFunc::Equal(str, "Int"))
        {
            return FilterParamType::Int;
        }
        else if (AZ::StringFunc::Equal(str, "Bool"))
        {
            return FilterParamType::Bool;
        }
        else
        {
            return FilterParamType::Count;
        }
    }

    void FilterParamData::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        if (!AZ::StringFunc::Equal(node.name(), Tag))
        {
            return;
        }

        auto attr = node.first_attribute(NameTag);
        if (attr)
        {
            m_name = attr->value();
        }

        attr = node.first_attribute(FilterParamType::Tag);
        if (attr)
        {
            m_type = FilterParamType::FromString(attr->value());
            if (m_type == FilterParamType::Count)
            {
                m_type = FilterParamType::Float;
            }
        }

        attr = node.first_attribute(MinValueTag);
        if (attr)
        {
            m_minValue = AZ::StringFunc::ToFloat(attr->value());
        }

        attr = node.first_attribute(MaxValueTag);
        if (attr)
        {
            m_maxValue = AZ::StringFunc::ToFloat(attr->value());
        }

        attr = node.first_attribute(ValueTag);
        if (attr)
        {
            m_value = AZ::StringFunc::ToFloat(attr->value());
        }
    }

    void FilterParamData::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        node.name(Tag);

        auto attr = xmlAlloc.allocate_attribute(NameTag, m_name.GetCStr());
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(FilterParamType::Tag, FilterParamType::ToString(m_type));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(MinValueTag, xmlAlloc.allocate_string(AZStd::to_string(m_minValue).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(MaxValueTag, xmlAlloc.allocate_string(AZStd::to_string(m_maxValue).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(ValueTag, xmlAlloc.allocate_string(AZStd::to_string(m_value).c_str()));
        node.append_attribute(attr);
    }

    void FilterData::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        if (!AZ::StringFunc::Equal(node.name(), Tag))
        {
            return;
        }

        auto attr = node.first_attribute(NameTag);
        if (attr)
        {
            m_name = attr->value();
        }

        m_params.clear();
        auto paramNode = node.first_node(FilterParamData::Tag);
        while (paramNode)
        {
            m_params.push_back();
            m_params.back().ReadFromXml(*paramNode);
            paramNode = paramNode->next_sibling(FilterParamData::Tag);
        }

        attr = node.first_attribute(IsEnabledTag);
        if (attr)
        {
            m_isEnabled = AZ::StringFunc::ToBool(attr->value());
        }
    }

    void FilterData::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        node.name(Tag);

        auto attr = xmlAlloc.allocate_attribute(NameTag, m_name.GetCStr());
        node.append_attribute(attr);

        for (const FilterParamData& param : m_params)
        {
            auto paramNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
            param.WriteToXml(*paramNode, xmlAlloc);
            node.append_node(paramNode);
        }

        attr = xmlAlloc.allocate_attribute(IsEnabledTag, xmlAlloc.allocate_string(AZStd::to_string(m_isEnabled).c_str()));
        node.append_attribute(attr);
    }

    void FilterBlockData::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        if (!AZ::StringFunc::Equal(node.name(), Tag))
        {
            return;
        }

        auto filterNode = node.first_node(FilterData::Tag);
        if (!filterNode)
        {
            return;
        }

        for (size_t i = 0; i < NumberOfFiltersPerBus && filterNode; ++i)
        {
            m_filters[i].ReadFromXml(*filterNode);
            filterNode = filterNode->next_sibling(FilterData::Tag);
        }
    }

    void FilterBlockData::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        node.name(Tag);

        for (size_t i = 0; i < NumberOfFiltersPerBus; ++i)
        {
            auto filterNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
            m_filters[i].WriteToXml(*filterNode, xmlAlloc);
            node.append_node(filterNode);
        }
    }

    void BusData::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        if (!AZ::StringFunc::Equal(node.name(), Tag))
        {
            return;
        }

        auto attr = node.first_attribute(NameTag);
        if (attr)
        {
            m_name = attr->value();
        }

        attr = node.first_attribute(OutputBusNameTag);
        if (attr)
        {
            m_outputBusName = attr->value();
        }

        attr = node.first_attribute(VolumeTag);
        if (attr)
        {
            m_volume = AZ::StringFunc::ToFloat(attr->value());
        }

        attr = node.first_attribute(IsMutedTag);
        if (attr)
        {
            m_isMuted = AZ::StringFunc::ToBool(attr->value());
        }

        attr = node.first_attribute(IsMonoTag);
        if (attr)
        {
            m_isMono = AZ::StringFunc::ToBool(attr->value());
        }

        auto blockNode = node.first_node(FilterBlockData::Tag);
        if (attr)
        {
            m_filterBlock.ReadFromXml(*blockNode);
        }
    }

    void BusData::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        node.name(Tag);

        auto attr = xmlAlloc.allocate_attribute(NameTag, m_name.GetCStr());
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(OutputBusNameTag, m_outputBusName.GetCStr());
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(VolumeTag, xmlAlloc.allocate_string(AZStd::to_string(m_volume).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(IsMutedTag, xmlAlloc.allocate_string(AZStd::to_string(m_isMuted).c_str()));
        node.append_attribute(attr);

        attr = xmlAlloc.allocate_attribute(IsMonoTag, xmlAlloc.allocate_string(AZStd::to_string(m_isMono).c_str()));
        node.append_attribute(attr);

        auto blockNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
        m_filterBlock.WriteToXml(*blockNode, xmlAlloc);
        node.append_node(blockNode);
    }

    void BusLayoutData::Reflect(AZ::ReflectContext* context)
    {
        if (auto serialContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialContext->Class<FilterParamData>()
                ->Version(1)
                ->Field("name", &FilterParamData::m_name)
                ->Field("value", &FilterParamData::m_value);

            serialContext->Class<FilterData>()
                ->Version(1)
                ->Field("name", &FilterData::m_name)
                ->Field("params", &FilterData::m_params)
                ->Field("isEnabled", &FilterData::m_isEnabled);

            serialContext->Class<FilterBlockData>()->Version(1)->Field("filters", &FilterBlockData::m_filters);

            serialContext->Class<BusData>()
                ->Version(1)
                ->Field("name", &BusData::m_name)
                ->Field("outputBusName", &BusData::m_outputBusName)
                ->Field("volume", &BusData::m_volume)
                ->Field("isMuted", &BusData::m_isMuted)
                ->Field("isMono", &BusData::m_isMono)
                ->Field("filterBlock", &BusData::m_filterBlock);

            serialContext->Class<BusLayoutData>()->Version(1)->Field("buses", &BusLayoutData::m_buses);
        }
    }

    bool BusLayoutData::Save(AZ::IO::PathView filePath)
    {
        AZ::JsonSerializerSettings serializeSettings;
        rapidjson::Document jsonDoc;

        auto result = AZ::JsonSerialization::Store(jsonDoc, jsonDoc.GetAllocator(), *this, serializeSettings);
        if (result.GetProcessing() != AZ::JsonSerializationResult::Processing::Completed)
        {
            AZ_Error(LogWindow, false, "Unable to serialize json file '%s'", filePath.Native().data());
            return false;
        }

        auto outcome = AZ::JsonSerializationUtils::WriteJsonFile(jsonDoc, filePath.Native());
        if (!outcome)
        {
            AZ_Error(LogWindow, false, "%s", outcome.GetError().c_str());
            return false;
        }

        return true;
    }

    bool BusLayoutData::Load(AZ::IO::PathView filePath)
    {
        auto outcome = AZ::JsonSerializationUtils::ReadJsonFile(filePath.Native());
        if (!outcome)
        {
            AZ_Error(LogWindow, false, "%s", outcome.GetError().c_str());
            return false;
        }

        AZ::JsonDeserializerSettings deserializeSettings;
        AZ::ComponentApplicationBus::BroadcastResult(
            deserializeSettings.m_serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        auto result = AZ::JsonSerialization::Load(*this, outcome.GetValue(), deserializeSettings);
        if (result.GetProcessing() != AZ::JsonSerializationResult::Processing::Completed)
        {
            AZ_Error(LogWindow, false, "Unable to deserialize json file '%s'", filePath.Native().data());
            return false;
        }

        return true;
    }

    void BusLayoutData::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        if (!AZ::StringFunc::Equal(node.name(), Tag))
        {
            return;
        }

        m_buses.clear();
        auto busNode = node.first_node(BusData::Tag);
        while (busNode)
        {
            m_buses.push_back();
            m_buses.back().ReadFromXml(*busNode);
            busNode = busNode->next_sibling(BusData::Tag);
        }
    }

    void BusLayoutData::WriteToXml(AZ::rapidxml::xml_node<char>& node, AZ::rapidxml::memory_pool<>& xmlAlloc) const
    {
        node.name(Tag);

        for (const BusData& bus : m_buses)
        {
            auto busNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
            bus.WriteToXml(*busNode, xmlAlloc);
            node.append_node(busNode);
        }
    }
} // namespace Audio
