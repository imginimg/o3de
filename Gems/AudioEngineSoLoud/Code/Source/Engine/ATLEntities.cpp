/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <ATLEntities.h>
#include <AzCore/StringFunc/StringFunc.h>

namespace Audio
{
    bool AtlRtpcImplDataSoLoud::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        if (AZ::StringFunc::Equal(node.name(), AudioFileTag))
        {
            m_type = RtpcImpl::AudioFile;

            auto attr = node.first_attribute(AudioFilePathTag);
            if (!attr)
            {
                return false;
            }

            m_audioFile.m_audioFilePath = attr->value();
            if (m_audioFile.m_audioFilePath.empty())
            {
                return false;
            }

            return m_audioFile.m_params.ReadFromXml(node);
        }
        else
        {
            m_type = RtpcImpl::Global;
            m_global.m_type = GlobalRtpc::FromString(node.name());
            return m_global.m_type != GlobalRtpc::Count;
        }
    }
} // namespace Audio
