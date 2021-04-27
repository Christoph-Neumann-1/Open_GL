cmake_minimum_required(VERSION 3.16.0)
project(Opengl VERSION 0.1.0)

# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

set(CMAKE_CXX_STANDARD 20)

link_directories(${CMAKE_SOURCE_DIR}/Lib )

include_directories(Libs)
include_directories(include)
include_directories(Libs/imgui/backends)
include_directories(Libs/imgui)
include_directories(Libs/glad/include)

add_subdirectory(Libs/glad)
add_subdirectory(Libs/imgui)
add_subdirectory(Libs/glm)

add_compile_options(-Wall -Wno-unknown-pragmas -Wshadow)

file(GLOB SOURCES "src/*.cpp")
add_library(OGL SHARED ${SOURCES} Libs/Image/stb_image.cpp)

add_subdirectory(scenes)
add_subdirectory(modules)

add_executable(${PROJECT_NAME} Main.cpp)
add_dependencies(${PROJECT_NAME} OGL)

target_precompile_headers(OGL PRIVATE pch.hpp)
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

target_link_libraries(OGL glad GL glfw IMGUI assimp dl pthread)
target_link_libraries(${PROJECT_NAME} glad GL glfw IMGUI OGL pthread)

add_custom_command(TARGET OGL
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OGL> ${CMAKE_SOURCE_DIR}/Lib/libOGL.so)

add_subdirectory(tests)

#Seems to work for now. Prevents constant rebuilds
if (CMAKE_GENERATOR MATCHES "Ninja")
    file(
        WRITE "${CMAKE_BINARY_DIR}/GNUMakeRulesOverwrite.cmake" 
            "STRING(REPLACE \"-MD\" \"-MMD\" CMAKE_DEPFILE_FLAGS_C \"\${CMAKE_DEPFILE_FLAGS_C}\")\n"
            "STRING(REPLACE \"-MD\" \"-MMD\" CMAKE_DEPFILE_FLAGS_CXX \"\${CMAKE_DEPFILE_FLAGS_CXX}\")\n"
    )
    set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_BINARY_DIR}/GNUMakeRulesOverwrite.cmake" CACHE INTERNAL "")
endif(CMAKE_GENERATOR MATCHES "Ninja")