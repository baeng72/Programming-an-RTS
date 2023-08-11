message("GLFW CMakeLists.txt")
file(GLOB GLFW_FILES "*.c??" "*.h")
foreach(FILE ${GLFW_FILES}) 
    # Get the directory of the source file
	
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
	get_filename_component(PARENT_DIR "${PARENT_DIR}" DIRECTORY)
	#message("Parent Dir:" "${PARENT_DIR}")
    # Remove common directory prefix to make the group
    string(REPLACE "${PARENT_DIR}" "" GROUP "${CMAKE_CURRENT_SOURCE_DIR}")
	# Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")
	message("File: " "${FILE}")
    # Group into "Source Files" and "Header Files"
    if ("${FILE}" MATCHES ".*\\.cpp")
       set(GROUP "Source Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.h")
       set(GROUP "Header Files${GROUP}")
    endif()
	message("${GROUP}")
    source_group("${GROUP}" FILES "${FILE}")
endforeach()

set(GLFW_FILES ${GLFW_FILES} PARENT_SCOPE)


