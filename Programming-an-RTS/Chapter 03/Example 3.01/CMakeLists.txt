set(EXE_NAME "Example_301")
file(GLOB SRC_FILES *.c?? *.h??)
add_executable(${EXE_NAME} ${SRC_FILES})

target_link_libraries(${EXE_NAME} common glfw glad glslang freetype easy_profiler)