#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

find_package(SDL2)
if (SDL2_FOUND)
    target_include_directories(SoLoud.Static PRIVATE ${SDL2_INCLUDE_DIRS})
    target_compile_definitions(SoLoud.Static PRIVATE WITH_SDL2)
endif()

target_compile_definitions(SoLoud.Static PRIVATE WITH_MINIAUDIO)
