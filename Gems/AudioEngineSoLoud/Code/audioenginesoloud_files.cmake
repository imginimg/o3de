#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Include/AudioEngineSoLoud/AudioEngineSoLoud.h

    Source/AudioEngineSoLoudGemSystemComponent.h
    Source/AudioEngineSoLoudGemSystemComponent.cpp
    Source/Util.h
    Source/Util.cpp
    Source/EditorEngineInterop.h
    Source/BusData.h
    Source/BusData.cpp
    Source/AtlData.h
    Source/AtlData.cpp
    Source/Config.h

    Source/Engine/AudioSystemImpl_SoLoud.h
    Source/Engine/AudioSystemImpl_SoLoud.cpp
    Source/Engine/ATLEntities.cpp
    Source/Engine/ATLEntities.h
    Source/Engine/AudioSystemImplCVars.h
    Source/Engine/AudioSystemImplCVars.cpp
    Source/Engine/AudioBusManager.h
    Source/Engine/AudioBusManager.cpp
    Source/Engine/AudioFilterManager.h
    Source/Engine/AudioFilterManager.cpp
    Source/Engine/AudioEngineSoLoudRequestHandler.h
    Source/Engine/AudioEngineSoLoudRequestHandler.cpp
    Source/Engine/AudioBusLayoutLoaderComponent.h
    Source/Engine/AudioBusLayoutLoaderComponent.cpp
)

# Skip the following file that is also used in the editor shared target so the compiler will recognize its the same symbol
set(SKIP_UNITY_BUILD_INCLUSION_FILES
    Source/AudioEngineSoLoudGemSystemComponent.cpp
)
