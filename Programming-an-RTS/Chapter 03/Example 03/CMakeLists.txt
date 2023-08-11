set(EXE_NAME "Example_03")
file(GLOB SRC_FILES *.c?? *.h??)
add_executable(${EXE_NAME} ${SRC_FILES})

target_link_libraries(${EXE_NAME} common glfw glslang freetype)