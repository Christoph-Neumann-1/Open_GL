add_library(glad SHARED src/glad.c)

target_compile_options(glad PRIVATE -O3)

add_custom_command(TARGET glad 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:glad> ${CMAKE_SOURCE_DIR}/Lib/libglad.so)