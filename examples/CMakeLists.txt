add_executable(kengine_example)

add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DTINYGLTF_NO_STB_IMAGE)
add_definitions(-DTINYGLTF_NO_STB_IMAGE_WRITE)

#include(CheckCXXCompilerFlag)
#check_cxx_compiler_flag(-fsanitize=address HAS_ASAN)
#if (HAS_ASAN)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")
#endif()

set(HEADERS
    include/public/kengine/game/Game.hpp
    include/public/kengine/game/BasicGameTest.hpp
    include/public/kengine/game/MainGameState.hpp
    include/public/kengine/game/BasicCameraController.hpp

# systems
    include/public/kengine/game/RenderSystem.hpp
    include/public/kengine/game/CameraSystem.hpp
    include/public/kengine/game/RenderablePreviousTransformSystem.hpp
    include/public/kengine/game/SpatialGridUpdateSystem.hpp
    include/public/kengine/game/AnimationSystem.hpp

# components
    include/public/kengine/game/components/Components.hpp
    include/public/kengine/game/components/Model.hpp
    include/public/kengine/game/components/Material.hpp

)

set(SOURCES
    src/Main.cpp
    src/BasicGameTest.cpp
    src/MainGameState.cpp
    src/BasicCameraController.cpp

# systems
    src/RenderSystem.cpp
    src/CameraSystem.cpp
    src/RenderablePreviousTransformSystem.cpp
    src/SpatialGridUpdateSystem.cpp
    src/AnimationSystem.cpp

    src/Components.cpp
)

target_sources(kengine_example 
    PRIVATE
        ${HEADERS}
        ${SOURCES}
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${HEADERS} ${SOURCES}
)

target_include_directories(kengine_engine PUBLIC include/public)
target_include_directories(kengine_engine PRIVATE include/private)
target_include_directories(kengine_engine PRIVATE include/thirdparty)

target_link_libraries(kengine_example PRIVATE kengine_engine)


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
add_dependencies(kengine_example kengine_shaders)