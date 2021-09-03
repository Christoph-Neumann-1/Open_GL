cmake_minimum_required(VERSION 3.16.0)
project(Opengl VERSION 0.1.0)

enable_testing()

# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

set(CMAKE_CXX_STANDARD 20)

#TODO remove build rpath when making package
set(CMAKE_BUILD_RPATH \$ORIGIN/Lib)

link_directories(${CMAKE_SOURCE_DIR}/Lib )

include_directories(Libs)
include_directories(include)
include_directories(Libs/imgui/backends)
include_directories(Libs/imgui)
include_directories(Libs/glad/include)

#TODO use submodules

add_subdirectory(Libs/glad)
add_subdirectory(Libs/imgui)
add_subdirectory(Libs/glm)

add_compile_options(-Wall -Wno-unknown-pragmas -Wshadow)


add_subdirectory(src)
add_subdirectory(scenes)

add_executable(${PROJECT_NAME} Main.cpp)
add_dependencies(${PROJECT_NAME} OGL)

target_link_libraries(${PROJECT_NAME} glad GL glfw IMGUI OGL pthread)

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