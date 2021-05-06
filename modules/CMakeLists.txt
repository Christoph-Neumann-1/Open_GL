file(MAKE_DIRECTORY bin)

file(GLOB MODULES "*.cpp")

foreach(MODULE ${MODULES})
    string(REPLACE ".cpp" "" MODULE_N2 ${MODULE})
    get_filename_component(MODULE_N ${MODULE_N2} NAME)
    add_library("m_${MODULE_N}" SHARED ${MODULE} )
    target_link_libraries("m_${MODULE_N}" OGL)
    add_dependencies("m_${MODULE_N}" OGL)
    set_target_properties("m_${MODULE_N}" PROPERTIES PREFIX "")
    add_custom_command(TARGET "m_${MODULE_N}"
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:m_${MODULE_N}> ${CMAKE_SOURCE_DIR}/modules/bin/${MODULE_N}.module)
endforeach(MODULE ${MODULES})
