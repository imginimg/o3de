/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Util.h>
#include <cmath>

namespace Audio
{
    uint32_t SpeakerConfiguration::ToChannelCount(Type conf)
    {
        switch (conf)
        {
        case Mono:
            return 1;

        case Stereo:
            return 2;

        case Quad:
            return 4;

        case _5_1:
            return 6;

        case _7_1:
            return 8;

        default:
            return 2;
        }
    }

    void EraseSubStr(AZStd::string& str, AZStd::string_view strToErase)
    {
        auto pos = str.find(strToErase);
        if (pos != AZStd::string::npos)
        {
            str.erase(pos, strToErase.length());
        }
    }

    float DbToLinear(float dbValue)
    {
        // Equivalent to 10 ^ (dbValue / 20)
        return std::exp(dbValue * 0.115129254f);
    }

    float LinearToDb(float linearValue)
    {
        // Equivalent to 20 * log10(linearValue)
        return std::log(linearValue) * 8.68588963f;
    }
} // namespace Audio
