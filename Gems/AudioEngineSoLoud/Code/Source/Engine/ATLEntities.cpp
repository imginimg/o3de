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
    bool SATLRtpcImplDataSoLoud::ReadFromXml(const AZ::rapidxml::xml_node<char>& node)
    {
        if (AZ::StringFunc::Equal(node.name(), AudioFileTag))
        {
            m_type = ERtpcImpl::AudioFile;

            auto attr = node.first_attribute(AudioFilePathTag);
            if (!attr)
                return false;

            m_audioFile.m_audioFilePath = attr->value();
            if (m_audioFile.m_audioFilePath.empty() || m_audioFile.m_audioFilePath[0] == '\0')
                return false;

            return m_audioFile.m_params.ReadFromXml(node);
        }

        m_global.m_type = EGlobalRtpc::FromString(node.name());
        return m_global.m_type != EGlobalRtpc::Count;
    }
} // namespace Audio
