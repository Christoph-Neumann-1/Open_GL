cmake_minimum_required(VERSION 3.16.0)
project(GL_Proj VERSION 0.1.0)

include(CTest)
enable_testing()


# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_SKIP_RPATH FALSE)
set(BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "Lib")
set(BUILD_RPATH CMAKE_INSTALL_RPATH )

set(CMAKE_CXX_STANDARD 20)

link_directories(${CMAKE_SOURCE_DIR}/Lib )

include_directories(Libs)
include_directories(header)
include_directories(Libs/imgui/backends)
include_directories(Libs/imgui)
include_directories(Libs/glad/include)

add_subdirectory(Libs/imgui)
add_subdirectory(Libs/glm)

add_compile_options(-Wall -Wno-unknown-pragmas)

file(GLOB SOURCES "src/*.cpp")
add_library(OGL SHARED ${SOURCES} Libs/Image/stb_image.cpp)

add_subdirectory(scenes)

add_executable(${PROJECT_NAME} Application.cpp)
add_dependencies(${PROJECT_NAME} OGL)

target_precompile_headers(OGL PRIVATE pch.hpp)
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

target_compile_definitions(${PROJECT_NAME} PUBLIC ROOT_DIR="${PROJECT_SOURCE_DIR}") 
target_compile_definitions(OGL PUBLIC ROOT_DIR="${PROJECT_SOURCE_DIR}") 

target_link_libraries(OGL glad GL glfw IMGUI assimp)
target_link_libraries(${PROJECT_NAME} glad GL glfw IMGUI OGL assimp)


add_custom_command(TARGET OGL
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OGL> ${CMAKE_SOURCE_DIR}/Lib/libOGL.so)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
