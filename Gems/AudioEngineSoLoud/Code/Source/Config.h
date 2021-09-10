/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/IO/Path/Path.h>

namespace Audio
{
    constexpr const char* LogWindow = "Audio SoLoud";

    constexpr const char ControlNamePathSeparator = '/';

    constexpr const auto AudioFilesPath = AZ::IO::FixedMaxPath("sounds/soloud").LexicallyNormal();
    constexpr const char* LocalizationDirName = "localization";
    constexpr const char* AudioFileFormatsArray[] = { "wav", "mp3", "ogg", "flac" };

    constexpr const char* ScriptCanvasTextFilePath = "/editor/audioenginesoloud_scriptcanvas";

    constexpr const char* BusLayoutFileExt = "bus_layout";
    constexpr const char* MasterBusName = "Master";
    constexpr int32_t NumberOfFiltersPerBus = 8; // Should be less than or equal to soloud.h/FILTERS_PER_STREAM.
} // namespace Audio
