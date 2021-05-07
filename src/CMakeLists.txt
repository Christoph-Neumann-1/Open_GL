file(GLOB SOURCES "*.cpp")

add_library(OGL SHARED ${SOURCES} ../Libs/Image/stb_image.cpp)
target_precompile_headers(OGL PRIVATE pch.hpp)
target_link_libraries(OGL glad GL glfw IMGUI assimp dl pthread)

add_custom_command(TARGET OGL
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OGL> ${CMAKE_SOURCE_DIR}/Lib/libOGL.so)