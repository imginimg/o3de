#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(SOLOUD_COMPILE_OPTIONS_PRIVATE "/W0")
    set(SOLOUD_COMPILE_DEFINITIONS_PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(SOLOUD_COMPILE_OPTIONS_PRIVATE "-w")
endif()

target_compile_definitions(SoLoud.Static PRIVATE ${SOLOUD_COMPILE_DEFINITIONS_PRIVATE})
target_compile_options(SoLoud.Static PRIVATE ${SOLOUD_COMPILE_OPTIONS_PRIVATE})


