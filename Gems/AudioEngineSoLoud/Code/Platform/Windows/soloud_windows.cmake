#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(SOLOUD_COMPILEDEFINITIONS
    WITH_WINMM
    WITH_WASAPI
    WITH_XAUDIO2
    _CRT_SECURE_NO_WARNINGS
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(SOLOUD_COMPILE_FLAGS "/W0")
else()
    set(SOLOUD_COMPILE_FLAGS " ")
endif()
