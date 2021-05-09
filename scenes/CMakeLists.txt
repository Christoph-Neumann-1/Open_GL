file(MAKE_DIRECTORY bin)

file(GLOB_RECURSE SCENES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

set(S_PCH 0)

foreach(SCENE ${SCENES})
    string(REPLACE ".cpp" "" SCENE_N2 ${SCENE})
    string(REPLACE "/" "__" SCENE_N ${SCENE_N2})
    string(REPLACE "." "__" SCENE_N ${SCENE_N})
    add_library(s_${SCENE_N} SHARED ${SCENE})
    target_link_libraries(s_${SCENE_N} glad GL glfw IMGUI OGL assimp pthread)
    add_dependencies(s_${SCENE_N} OGL)
    set_target_properties(s_${SCENE_N} PROPERTIES PREFIX "")
    add_custom_command(TARGET s_${SCENE_N}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:s_${SCENE_N}> ${CMAKE_SOURCE_DIR}/scenes/bin/${SCENE_N2}.scene)
    
    set_property(
    TARGET s_${SCENE_N}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_SOURCE_DIR}/scenes/bin/${SCENE_N2}.scene
    )

    if(${S_PCH} EQUAL 0)
    target_precompile_headers(s_${SCENE_N} PRIVATE pch.hpp)
    set(S_PCH s_${SCENE_N})
    else()
    target_compile_definitions(s_${SCENE_N} PRIVATE ${S_PCH}_EXPORTS) #Stops gcc complains
    target_precompile_headers(s_${SCENE_N} REUSE_FROM ${S_PCH})
    endif(${S_PCH} EQUAL 0)
                   
endforeach(SCENE ${SCENES})
