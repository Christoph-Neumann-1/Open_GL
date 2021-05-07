file(MAKE_DIRECTORY bin)

file(GLOB SCENES "*.cpp")

set(S_PCH 0)

foreach(SCENE ${SCENES})
    string(REPLACE ".cpp" "" SCENE_N2 ${SCENE})
    get_filename_component(SCENE_N ${SCENE_N2} NAME)
    add_library(s_${SCENE_N} SHARED ${SCENE})
    target_link_libraries(s_${SCENE_N} glad GL glfw IMGUI OGL assimp pthread)
    add_dependencies(s_${SCENE_N} OGL)
    set_target_properties(s_${SCENE_N} PROPERTIES PREFIX "")
    add_custom_command(TARGET s_${SCENE_N}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:s_${SCENE_N}> ${CMAKE_SOURCE_DIR}/scenes/bin/${SCENE_N}.scene)
    
    if(${S_PCH} EQUAL 0)
    target_precompile_headers(s_${SCENE_N} PRIVATE pch.hpp)
    set(S_PCH s_${SCENE_N})
    else()
    target_compile_definitions(s_${SCENE_N} PRIVATE s_3d_EXPORTS)
    target_precompile_headers(s_${SCENE_N} REUSE_FROM ${S_PCH})
    endif(${S_PCH} EQUAL 0)
                   
endforeach(SCENE ${SCENES})
