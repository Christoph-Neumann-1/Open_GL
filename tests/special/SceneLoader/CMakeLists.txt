add_library(SceneLoaderLib SHARED SceneLoaderLib.cpp)
add_custom_command(TARGET SceneLoaderLib
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SceneLoaderLib> ${CMAKE_SOURCE_DIR}/tests/special/SceneLoader/libSceneLoaderLib.so)

add_executable(t_SceneLoader t_SceneLoader.cpp)
target_link_libraries(t_SceneLoader OGL gtest_main)
add_dependencies(t_SceneLoader OGL)
gtest_discover_tests(t_SceneLoader WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/special/SceneLoader PROPERTIES ENVIRONMENT LD_PRELOAD=./libSceneLoaderLib.so)


add_library(t_SceneLoader_ SHARED TestScene.cpp)
target_link_libraries(t_SceneLoader_ OGL)
add_dependencies(t_SceneLoader_ OGL)
set_target_properties(t_SceneLoader_ PROPERTIES PREFIX "")
add_custom_command(TARGET t_SceneLoader_
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:t_SceneLoader_> ${CMAKE_SOURCE_DIR}/tests/special/SceneLoader/Scene.scene)
