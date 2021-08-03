#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Source/AudioEngineSoLoudGemSystemComponent.h
    Source/AudioEngineSoLoudGemSystemComponent.cpp
    Source/Engine/AudioSystemImpl_SoLoud.h
    Source/Engine/AudioSystemImpl_SoLoud.cpp
    Source/Engine/ATLEntities.h
    Source/Engine/AudioSystemImplCVars.h
    Source/Engine/AudioSystemImplCVars.cpp
    Source/Common.h
    Source/Common.cpp
)

# Skip the following file that is also used in the editor shared target so the compiler will recognize its the same symbol
set(SKIP_UNITY_BUILD_INCLUSION_FILES
    Source/AudioEngineSoLoudGemSystemComponent.cpp
)
