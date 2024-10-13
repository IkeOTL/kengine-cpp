add_executable(basic)

set(HEADERS
    src/Game.hpp
    src/BasicGameTest.hpp
    src/MainGameState.hpp
    src/BasicCameraController.hpp

    # systems
    src/RenderSystem.hpp
    src/CameraSystem.hpp
    src/RenderablePreviousTransformSystem.hpp
    src/SpatialGridUpdateSystem.hpp
    src/AnimationSystem.hpp

    # components
    src/components/Components.hpp
    src/components/Model.hpp
    src/components/Material.hpp
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

    src/components/Components.cpp
)

set_target_properties(basic PROPERTIES FOLDER "examples")

target_sources(basic 
    PRIVATE
        ${HEADERS}
        ${SOURCES}
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${HEADERS} ${SOURCES}
)

target_include_directories(basic PRIVATE src)

target_link_libraries(basic PRIVATE kengine_engine)

