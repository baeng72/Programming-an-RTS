cmake_minimum_required(VERSION 3.8)
file(GLOB COMMON_FILES "*.h")
set(COMMON_FILES ${COMMON_FILES} PARENT_SCOPE)



add_subdirectory(Platform)
add_subdirectory(Core)
add_subdirectory(Renderer)
add_subdirectory(mesh)
add_subdirectory(anim)
set(SOURCE_FILES ${COMMON_FILES} ${PLATFORM_FILES} ${CORE_FILES} ${RENDERER_FILES} ${MESH_FILES} ${ANIM_FILES})

add_library(common ${SOURCE_FILES})


set_property(TARGET common PROPERTY FOLDER "Common")

source_group(common files ${common_files})
source_group(platform files ${platform_files})
source_group(core files ${core_files})
source_group(renderer files ${renderer_files})
source_group(mesh files ${mesh_files})
source_group(anim files ${anim_files})
#message("Src: " ${SRC_BUILD_FILES})
#set_source_files_properties(${COMMON_FILES} PROPERTIES HEADER_FILES_ONLY TRUE)
# source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
#         PREFIX "common"
 #        FILES ${COMMON_FILES})
