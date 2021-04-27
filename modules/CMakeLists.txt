
file(GLOB MODULES "*.cpp")
foreach(MODULE ${MODULES})
    string(REPLACE ".cpp" "" MODULE_N2 ${MODULE})
    get_filename_component(MODULE_N ${MODULE_N2} NAME)
    add_library(${MODULE_N} SHARED ${MODULE} )
    target_link_libraries(${MODULE_N} glad GL glfw IMGUI OGL assimp pthread)
    add_dependencies(${MODULE_N} OGL)
    set_target_properties(${MODULE_N} PROPERTIES PREFIX "")
    add_custom_command(TARGET ${MODULE_N}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${MODULE_N}> ${CMAKE_SOURCE_DIR}/modules/bin/${MODULE_N}.module)
endforeach(MODULE ${MODULES})
