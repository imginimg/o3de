#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
find_library(COREAUDIO_LIBRARY CoreAudio REQUIRED)
find_library(AUDIOUNIT_LIBRARY AudioUnit REQUIRED)
target_link_libraries(SoLoud.Static PRIVATE ${COREFOUNDATION_LIBRARY} ${COREAUDIO_LIBRARY} ${AUDIOUNIT_LIBRARY})

target_compile_definitions(SoLoud.Static PRIVATE WITH_MINIAUDIO MA_NO_RUNTIME_LINKING)
