cmake_minimum_required(VERSION 3.2)
project(ThirdParty LANGUAGES C CXX)

#glm
add_library(glm INTERFACE)
set(GLM_DIR ${CMAKE_CURRENT_DIR}/glm)
target_sources(glm INTERFACE ${GLM_DIR}/glm/glm.hpp)
target_include_directories(glm INTERFACE ${GLM_DIR})

target_compile_definitions(glm INTERFACE
    GLM_FORCE_RADIANS
    GLM_FORCE_ZERO_ON
    )

#glslang
option(ENABLE_SPVREMAPPER OFF)
option(ENABLE_GLSLANG_BINARIES OFF)
set(ENABLE_HSLS OFF)
option(BUILD_TESTING OFF)
option(BUILD_EXTERNAL OFF)
option(BUILD_SHARED_LIBS ON)

add_subdirectory(glslang)

if (NOT TARGET glslang-default-resource-limits)
    add_library(glslang-default-resource-limits
                glslang/StandAlone/ResourceLimits.cpp)

    set_target_properties(glslang-default-resource-limits PROPERTIES FOLDER "ThirdParty" POSITION_INDEPENDENT_CODE ON)

    target_include_directories(glslang-default-resource-limits
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glslang)
endif()

if(NOT MSVC)
    target_compile_options(glslang PRIVATE
        "-Wno-logical-op-parentheses"
        "-Wno-unused-parameter")

    target_compile_options(SPIRV PRIVATE
        "-Wno-logical-op-parentheses"
        "-Wno-unused-parameter")
endif()

set_target_properties(glslang PROPERTIES FOLDER "ThirdParty" POSITION_INDEPENDENT_CODE ON)
set_target_properties(OGLCompiler PROPERTIES FOLDER "ThirdParty" POSITION_INDEPENDENT_CODE ON)
set_target_properties(OSDependent PROPERTIES FOLDER "ThirdParty" POSITION_INDEPENDENT_CODE ON)
set_target_properties(SPIRV PROPERTIES FOLDER "ThirdParty" POSITION_INDEPENDENT_CODE ON)

#glfw
add_subdirectory(glfw)

#SPIRV-Reflect
set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_EXAMPLES OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)
add_subdirectory(SPIRV-Reflect)

#vma
add_subdirectory(VulkanMemoryAllocator)

#stb 
add_library(stb INTERFACE)
set(STB_DIR ${CMAKE_CURRENT_DIR}/stb)
target_sources(stb INTERFACE ${STB_DIR}/stb_image.h)
target_include_directories(stb INTERFACE ${STB_DIR})

#spdlog
add_subdirectory(spdlog)

#freetype
add_subdirectory(freetype)