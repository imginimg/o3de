#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(SOLOUD_COMPILE_DEFINITIONS_PRIVATE
    WITH_WINMM
    WITH_WASAPI
    WITH_XAUDIO2
)
target_compile_definitions(SoLoud.Static PRIVATE ${SOLOUD_COMPILE_DEFINITIONS_PRIVATE})

