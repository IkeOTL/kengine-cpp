add_executable(example)

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
)

set(SOURCES
    src/Main.cpp
    src/BasicGameTest.cpp
    src/MainGameState.cpp
    src/BasicCameraController.cpp
)

target_sources(example 
    PRIVATE
        ${HEADERS}
        ${SOURCES}
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${HEADERS} ${SOURCES}
)

target_include_directories(engine PUBLIC include/public)
target_include_directories(engine PRIVATE include/private)
target_include_directories(engine PRIVATE include/thirdparty)

target_link_libraries(example PRIVATE engine)