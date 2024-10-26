add_executable(basic)

set(HEADERS
    src/BasicGameTest.hpp
    src/MainGameState.hpp
    src/BasicCameraController.hpp

    src/PhysicsContext.hpp

    src/MyPlayerContext.hpp
    src/PlayerMovementManager.hpp

    # systems
    src/MyPlayerSystem.hpp
    src/RenderSystem.hpp
    src/CameraSystem.hpp
    src/RenderablePreviousTransformSystem.hpp
    src/SpatialGridUpdateSystem.hpp
    src/AnimationSystem.hpp
    src/KinematicPlayerSystem.hpp
    src/PhysicsSystem.hpp
    src/PhysicsSyncSystem.hpp
    src/PlayerCameraSystem.hpp

    # components
    src/components/Components.hpp
    src/components/Model.hpp
    src/components/Material.hpp
    src/components/Physics.hpp
)

set(SOURCES
    src/Main.cpp
    src/BasicGameTest.cpp
    src/MainGameState.cpp
    src/BasicCameraController.cpp

    src/PhysicsContext.cpp

    src/MyPlayerContext.cpp
    src/PlayerMovementManager.cpp

# systems
    src/RenderSystem.cpp
    src/CameraSystem.cpp
    src/RenderablePreviousTransformSystem.cpp
    src/SpatialGridUpdateSystem.cpp
    src/AnimationSystem.cpp
    src/KinematicPlayerSystem.cpp
    src/PhysicsSystem.cpp
    src/PhysicsSyncSystem.cpp
    src/PlayerCameraSystem.cpp

    src/components/Components.cpp
)

target_compile_definitions(kengine_engine PUBLIC KE_ACTIVE_LOG_LEVEL=KE_LOG_LEVEL_DEBUG)
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

