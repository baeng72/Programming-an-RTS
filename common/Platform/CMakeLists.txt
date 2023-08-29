message("Platform CMakeLists.txt")
add_subdirectory(stb)
add_subdirectory(glfw)
add_subdirectory(Vulkan)
add_subdirectory(Assimp)
add_subdirectory(Utils)
add_subdirectory(meshoptimizer)

file(GLOB PLATFORM_FILES "*.c??" "*.h??")
# #source_group(PLATFORM FILES ${PLATFORM_FILES})
# foreach(FILE ${PLATFORM_FILES}) 
    # # Get the directory of the source file
    # get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

    # # Remove common directory prefix to make the group
    # string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

    # # Make sure we are using windows slashes
    # string(REPLACE "/" "\\" GROUP "${GROUP}")
	# message("File: " "${FILE}")
    # # Group into "Source Files" and "Header Files"
    # if ("${FILE}" MATCHES ".*\\.cpp")
       # set(GROUP "Source Files${GROUP}")
    # elseif("${FILE}" MATCHES ".*\\.h")
       # set(GROUP "Header Files${GROUP}")
    # endif()

    # source_group("${GROUP}" FILES "${FILE}")
# endforeach()

set(PLATFORM_FILES ${STB_FILES} ${GLFW_FILES} ${VULKAN_FILES} ${ASSIMP_FILES} ${UTILS_FILES} ${MESHOPT_FILES})
set(PLATFORM_FILES ${PLATFORM_FILES} PARENT_SCOPE)
