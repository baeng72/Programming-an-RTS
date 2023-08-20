set(EXE_NAME "Example_409")
file(GLOB SRC_FILES "*.c??")
file(GLOB HDR_FILES "*.h")
#message("${EXE_NAME}" " SRC: " "${SRC_FILES}" " HDR: " "${HDR_FILES}")
#source_group("Source Files" ${SRC_FILES})
#source_group("Header Files" ${HDR_FILES})
add_executable(${EXE_NAME} ${SRC_FILES} ${HDR_FILES})

target_link_libraries(${EXE_NAME} common glfw glslang freetype assimp)