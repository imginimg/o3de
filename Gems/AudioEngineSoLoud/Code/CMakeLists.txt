#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

ly_get_list_relative_pal_filename(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME})
ly_get_list_relative_pal_filename(common_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/Common)

include(${pal_dir}/soloud_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake) #for PAL_TRAIT_AUDIO_ENGINE_SOLOUD Traits

set(AUDIOENGINESOLOUD_COMPILEDEFINITIONS
    $<IF:$<CONFIG:Release>,SOLOUD_RELEASE,ENABLE_AUDIO_LOGGING>
)

################################################################################
# Server / Unsupported
################################################################################
if (PAL_TRAIT_BUILD_SERVER_SUPPORTED OR PAL_TRAIT_AUDIO_ENGINE_SOLOUD_USE_STUB)
    # Stub gem for server and unsupported platforms. Audio Engine SoLoud is client only
    ly_add_target(
        NAME AudioEngineSoLoud.Stub ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
        NAMESPACE Gem
        FILES_CMAKE
            audioenginesoloud_stub_files.cmake
        BUILD_DEPENDENCIES
            PRIVATE
                AZ::AzCore
    )
endif()

if (PAL_TRAIT_AUDIO_ENGINE_SOLOUD_USE_STUB)
    # setup aliases so stubs will be used if something references AudioEngineSoLoud(.Editor)
    add_library(Gem::AudioEngineSoLoud ALIAS AudioEngineSoLoud.Stub)
    add_library(Gem::AudioEngineSoLoud.Editor ALIAS AudioEngineSoLoud.Stub)
    return()
endif()

################################################################################
# Runtime / Game
################################################################################
ly_add_target(
    NAME SoLoud.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        soloud_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source/soloud
        PUBLIC
            Source/soloud/include
    COMPILE_DEFINITIONS
        PRIVATE
            ${SOLOUD_COMPILEDEFINITIONS}
    TARGET_PROPERTIES
        COMPILE_FLAGS
            ${SOLOUD_COMPILE_FLAGS}
)

ly_add_target(
    NAME AudioEngineSoLoud.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        audioenginesoloud_files.cmake
        ${pal_dir}/platform_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            Source/Engine
            ${pal_dir}
            ${common_dir}
        PUBLIC
            Include
    COMPILE_DEFINITIONS
        PUBLIC
            ${AUDIOENGINESOLOUD_COMPILEDEFINITIONS}
    BUILD_DEPENDENCIES
        PUBLIC
            Legacy::CryCommon
            Gem::AudioSystem.Static
            Gem::SoLoud.Static
)

ly_add_target(
    NAME AudioEngineSoLoud ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        audioenginesoloud_shared_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            Source/Engine
            ${pal_dir}
            ${common_dir}
    COMPILE_DEFINITIONS
        PUBLIC
            ${AUDIOENGINESOLOUD_COMPILEDEFINITIONS}
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::AudioEngineSoLoud.Static
)

# we'll load the above "Gem::AudioEngineSoLoud" module in clients.
ly_create_alias(NAME AudioEngineSoLoud.Clients NAMESPACE Gem TARGETS Gem::AudioEngineSoLoud)


################################################################################
# Tools / Editor
################################################################################
if (PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME AudioEngineSoLoud.Editor.Static STATIC
        NAMESPACE Gem
        AUTOMOC
        AUTOUIC
        AUTORCC
        FILES_CMAKE
            audioenginesoloud_editor_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
                Source/Engine
                Source/Editor
                ${pal_dir}
        BUILD_DEPENDENCIES
            PRIVATE
                Legacy::EditorCore
                3rdParty::Qt::Core
                3rdParty::Qt::Gui
                3rdParty::Qt::Widgets
            PUBLIC
                AZ::AssetBuilderSDK
                Gem::AudioEngineSoLoud.Static
                Gem::AudioSystem.Editor
        RUNTIME_DEPENDENCIES
            Gem::AudioSystem.Editor
    )

    ly_add_target(
        NAME AudioEngineSoLoud.Editor GEM_MODULE
        NAMESPACE Gem
        FILES_CMAKE
            audioenginesoloud_editor_shared_files.cmake
        COMPILE_DEFINITIONS
            PRIVATE
                AUDIO_ENGINE_SOLOUD_EDITOR
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
                Source/Engine
                Source/Editor
                ${pal_dir}
        BUILD_DEPENDENCIES
            PRIVATE
                Gem::AudioEngineSoLoud.Editor.Static
        RUNTIME_DEPENDENCIES
            Gem::AudioSystem.Editor
    )

    # by default, we'll load the above "Gem::AudioEngineSoLoud.Editor" module in builders and tools.
    ly_create_alias(NAME AudioEngineSoLoud.Builders NAMESPACE Gem TARGETS Gem::AudioEngineSoLoud.Editor)
    ly_create_alias(NAME AudioEngineSoLoud.Tools NAMESPACE Gem TARGETS Gem::AudioEngineSoLoud.Editor)
endif()
