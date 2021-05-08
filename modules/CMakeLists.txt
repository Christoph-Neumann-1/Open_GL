file(MAKE_DIRECTORY bin)

file(GLOB_RECURSE MODULES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

foreach(MODULE ${MODULES})
    string(REPLACE ".cpp" "" MODULE_N2 ${MODULE})
    string(REPLACE "/" "__" MODULE_N ${MODULE_N2})
    string(REPLACE "." "__" MODULE_N ${MODULE_N})

    add_library(m_${MODULE_N} SHARED ${MODULE} )
    target_link_libraries(m_${MODULE_N} OGL)
    add_dependencies(m_${MODULE_N} OGL)
    set_target_properties(m_${MODULE_N} PROPERTIES PREFIX "")
    add_custom_command(TARGET m_${MODULE_N}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:m_${MODULE_N}> ${CMAKE_SOURCE_DIR}/modules/bin/${MODULE_N2}.module)

    set_property(
    TARGET m_${MODULE_N}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_SOURCE_DIR}/modules/bin/${MODULE_N2}.module
    )

endforeach(MODULE ${MODULES})
