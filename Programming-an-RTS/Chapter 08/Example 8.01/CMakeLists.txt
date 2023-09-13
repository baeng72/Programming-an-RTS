set(EXE_NAME "Example_801")
file(GLOB SRC_FILES "*.c??")
file(GLOB HDR_FILES "*.h")
add_executable(${EXE_NAME} ${SRC_FILES} ${HDR_FILES})

target_link_libraries(${EXE_NAME} common glfw glslang freetype assimp meshoptimizer)