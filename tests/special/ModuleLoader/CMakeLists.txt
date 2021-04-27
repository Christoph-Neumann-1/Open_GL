add_executable(t_ModuleLoader t_ModuleLoader.cpp)
target_link_libraries(t_ModuleLoader OGL gtest_main)
add_dependencies(t_ModuleLoader OGL)
gtest_discover_tests(t_ModuleLoader WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/special/ModuleLoader)

add_library(t_Module SHARED TestModule.cpp)
add_custom_command(TARGET t_Module
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:t_Module> ${CMAKE_SOURCE_DIR}/tests/special/ModuleLoader/t_Module.module)