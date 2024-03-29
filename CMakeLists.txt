cmake_minimum_required(VERSION 3.8)

project(Programming-an-RTS VERSION 0.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
# GLFW options
set(GLFW_VULKAN_STATIC ON CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
# GLSLANG options
set(ENABLE_GLSLANG_BINARIES  OFF  CACHE BOOL "")
set(ENABLE_HLSL              OFF  CACHE BOOL "")
set(ENABLE_CTEST             OFF  CACHE BOOL "")
set(ENABLE_OPT               OFF  CACHE BOOL "")
set(SKIP_GLSLANG_INSTALL     ON   CACHE BOOL "")
include_directories(ThirdParty)
include_directories(ThirdParty/stb)
include_directories(ThirdParty/glfw/include)
include_directories(ThirdParty/spdlog/include)
include_directories(ThirdParty/VulkanMemoryAllocator/include)
include_directories(ThirdParty/SPIRV-Reflect)
include_directories(ThirdParty/SPIRV-Reflect/include)
include_directories(ThirdParty/freetype/include)
include_directories(ThirdParty/assimp/)
include_directories(ThirdParty/assimp/include)
include_directories(ThirdParty/meshoptimizer/src)
include_directories(build/ThirdParty/assimp/include)
include_directories(ThirdParty/easy_profiler/easy_profiler_core/include)
include_directories(ThirdParty/glad/include)
include_directories(ThirdParty/imgui)

add_subdirectory(ThirdParty)

set_property(TARGET assimp          PROPERTY FOLDER "ThirdParty/assimp")
set_property(TARGET unit          PROPERTY FOLDER "ThirdParty/assimp")
set_property(TARGET uninstall          PROPERTY FOLDER "ThirdParty/assimp")
set_property(TARGET freetype          PROPERTY FOLDER "ThirdParty")
set_property(TARGET glfw          PROPERTY FOLDER "ThirdParty/glfw")
set_property(TARGET update_mappings          PROPERTY FOLDER "ThirdParty/glfw")
set_property(TARGET glm          PROPERTY FOLDER "ThirdParty")
set_property(TARGET GenericCodeGen       PROPERTY FOLDER "ThirdParty/glslang")
set_property(TARGET glslang       PROPERTY FOLDER "ThirdParty/glslang")
set_property(TARGET glslang-default-resource-limits       PROPERTY FOLDER "ThirdParty/glslang")
set_property(TARGET OGLCompiler   PROPERTY FOLDER "ThirdParty/glslang")
set_property(TARGET OSDependent   PROPERTY FOLDER "ThirdParty/glslang")
set_property(TARGET MachineIndependent   PROPERTY FOLDER "ThirdParty/glslang")
set_property(TARGET SPIRV         PROPERTY FOLDER "ThirdParty/glslang")
set_property(TARGET spirv-reflect-static         PROPERTY FOLDER "ThirdParty")
set_property(TARGET meshoptimizer          PROPERTY FOLDER "ThirdParty")
set_property(TARGET spdlog          PROPERTY FOLDER "ThirdParty")
set_property(TARGET stb          PROPERTY FOLDER "ThirdParty")
set_property(TARGET VulkanMemoryAllocator          PROPERTY FOLDER "ThirdParty")
set_property(TARGET glad PROPERTY FOLDER "ThirdParty")
if(WIN32)
#set_property(TARGET zlib          PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET zlibstatic    PROPERTY FOLDER "ThirdParty")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdParty")
endif()




find_package(Vulkan)
include_directories(AFTER ${Vulkan_INCLUDE_DIRS})
#include_directories(Vulkan_INCLUDE_DIRS)
if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -DVK_USE_PLATFORM_WIN32_KHR -DNOMINMAX -D__GL__TOP__LEFT__ -D__GL__ZERO__TO__ONE__)
endif()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
link_libraries(${Vulkan_LIBRARY})
link_libraries(spirv-reflect-static)
add_subdirectory(common)
include_directories(common)
add_subdirectory(Programming-an-RTS)



