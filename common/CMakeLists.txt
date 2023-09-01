#project(common)
file(GLOB COMMON_FILES "*.h")
set(COMMON_FILES ${COMMON_FILES} PARENT_SCOPE)



add_subdirectory(Platform)
add_subdirectory(Core)
add_subdirectory(Renderer)
add_subdirectory(mesh)
add_subdirectory(anim)
set(SOURCE_FILES ${COMMON_FILES} ${PLATFORM_FILES} ${CORE_FILES} ${RENDERER_FILES} ${MESH_FILES} ${ANIM_FILES})
add_library(common ${SOURCE_FILES})

# set(_src_root_path ${CMAKE_CURRENT_SOURCE_DIR})
# foreach(_source IN ITEMS ${CORE_FILES})
	
    # get_filename_component(_source_path "${_source}" PATH)
	# message("Source Path: " ${_source_path})
    # file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
	# message("Source REl Path: " ${_source_path_rel})
    # string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
	# message("Group: " ${_group_path})
    # source_group("${_group_path}" FILES "${_source}")
# endforeach()
#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${ANIM_FILES})
set_property(TARGET common PROPERTY FOLDER "Common")

# # EXCLUDE_CURRENT to exclude source in the current directory
# function(glob_source sources curdir)
    # glob_source_recurse(sources ${curdir} "" ${ARGN})
    # # Need to set sources in the parent scope https://stackoverflow.com/a/50953059/213871
    # #set(${sources} ${${sources}} ${SOURCE_FILES} PARENT_SCOPE)
# endfunction()

# macro(glob_source_recurse sources curdir prefix)
    # subdirlist(subdirs ${curdir})
    # foreach(subdir ${subdirs})
        # if ("${prefix}" STREQUAL "")
            # glob_source_recurse(sources "${curdir}/${subdir}" "${subdir}")
        # else()
            # glob_source_recurse(sources "${curdir}/${subdir}" "${prefix}/${subdir}")
        # endif()
    # endforeach()

    # set (extra_macro_args ${ARGN})
    # if (NOT extra_macro_args STREQUAL "EXCLUDE_CURRENT")
        # file(GLOB SOURCE_FILES "${curdir}/*.cpp" "${curdir}/*.h" "${curdir}/*.hpp")
        # source_group("${prefix}" FILES ${SOURCE_FILES})
        # list(APPEND sources ${SOURCE_FILES})
    # endif()
# endmacro()

# MACRO(SUBDIRLIST result curdir)
  # FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  # SET(dirlist "")
  # FOREACH(child ${children})
	
    # IF(IS_DIRECTORY ${curdir}/${child})
      # LIST(APPEND dirlist ${child})
    # ENDIF()
  # ENDFOREACH()
  # SET(${result} ${dirlist})
# ENDMACRO()

# glob_source(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR} SRC)