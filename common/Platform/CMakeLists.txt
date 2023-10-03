message("Platform CMakeLists.txt")
add_subdirectory(stb)
add_subdirectory(glfw)
add_subdirectory(Vulkan)
add_subdirectory(GL)
add_subdirectory(Assimp)
add_subdirectory(Utils)
add_subdirectory(meshoptimizer)


file(GLOB PLATFORM_FILES "*.c??" "*.h??")

set(PLATFORM_FILES ${STB_FILES} ${GLFW_FILES} ${VULKAN_FILES} ${GL_FILES} ${ASSIMP_FILES} ${UTILS_FILES} ${MESHOPT_FILES} )
set(PLATFORM_FILES ${PLATFORM_FILES} PARENT_SCOPE)
