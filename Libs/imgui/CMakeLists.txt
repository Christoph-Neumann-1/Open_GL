file(GLOB IMGUI_SOURCES "*.cpp")
file(GLOB BACKEND_SOURCES "backends/*.cpp")

add_library(IMGUI SHARED ${IMGUI_SOURCES} ${BACKEND_SOURCES})
target_include_directories(IMGUI PRIVATE backends)
target_include_directories(IMGUI PRIVATE .)

set(CPACK_PROJECT_NAME IMGUI)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
target_link_libraries(IMGUI glad)

add_custom_command(TARGET IMGUI 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:IMGUI> ${CMAKE_SOURCE_DIR}/Lib/libIMGUI.so)