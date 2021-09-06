/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AudioAllocators.h>
#include <AzCore/std/string/string.h>

namespace Audio
{
    //------------------------------------
    template<class T>
    struct AudioDeleter
    {
        void operator()(T* ptr)
        {
            azdestroy(ptr, Audio::AudioImplAllocator, T);
        }
    };

    //------------------------------------
    namespace SpeakerConfiguration
    {
        enum Type
        {
            Mono,
            Stereo,
            Quad,
            _5_1,
            _7_1
        };

        uint32_t ToChannelCount(Type conf);
    } // namespace SpeakerConfiguration

    //------------------------------------
    void EraseSubStr(AZStd::string& str, AZStd::string_view strToErase);

    float DbToLinear(float dbValue);
    float LinearToDb(float linearValue);
} // namespace Audio
