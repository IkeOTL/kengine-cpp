add_subdirectory(basic)


# compile a shaders to SPIR-V
function(compile_shader SHADER_SRC SHADER_DST)
    find_program(GLSLC_EXECUTABLE NAMES glslc HINTS ${Vulkan_GLSLC_EXECUTABLE})
    if(NOT GLSLC_EXECUTABLE)
        message(FATAL_ERROR "glslc not found!")
    endif()

    add_custom_command(
        OUTPUT ${SHADER_DST}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER_SRC} -o ${SHADER_DST}
        DEPENDS ${SHADER_SRC}
        IMPLICIT_DEPENDS CXX ${SHADER_SRC}
        COMMENT "Compiling ${SHADER_SRC} to SPIR-V"
    )
endfunction()

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/src")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/src")

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

file(GLOB SHADERS
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.comp"
)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_SPIRV "${SHADER_BINARY_DIR}/${SHADER_NAME}.spv")
    compile_shader(${SHADER} ${SHADER_SPIRV})
    list(APPEND SPIRV_BINARY_FILES ${SHADER_SPIRV})
endforeach()

add_custom_target(kengine_shaders
    ALL DEPENDS ${SPIRV_BINARY_FILES}
    SOURCES ${SHADERS}
)
add_dependencies(basic kengine_shaders)