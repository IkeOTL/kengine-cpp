add_library(kengine_engine STATIC)

#set(BUILD_SHARED_LIBS ON CACHE BOOL "Toggle static or shared" FORCE)

target_compile_definitions(kengine_engine PUBLIC
    NOMINMAX
    KE_DEBUG_RENDER # enable debug render code
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    TINYGLTF_NO_STB_IMAGE
    TINYGLTF_NO_STB_IMAGE_WRITE
    TRACY_ENABLE # need to be conditional
)

#include(CheckCXXCompilerFlag)
#check_cxx_compiler_flag(-fsanitize=address HAS_ASAN)
#if (HAS_ASAN)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")
#endif()

set(HEADERS
    include/public/kengine/util/Random.hpp
    include/public/kengine/util/MapUtils.hpp
    include/public/kengine/util/VecUtils.hpp
    include/public/kengine/util/MatUtils.hpp

    include/public/kengine/input/InputManager.hpp
    include/public/kengine/input/EventListener.hpp
    include/public/kengine/input/InputEventAdapter.hpp

    include/public/kengine/ecs/BaseSystem.hpp
    include/public/kengine/ecs/EcsSystem.hpp
    include/public/kengine/ecs/World.hpp

    include/public/kengine/terrain/TileTerrain.hpp
    include/public/kengine/terrain/TileTerrainChunk.hpp
    include/public/kengine/terrain/TileSheet.hpp

    include/public/kengine/io/AssetIO.hpp
    include/public/kengine/io/MemoryMappedFile.hpp

    include/public/kengine/EngineConfig.hpp
    include/public/kengine/Logger.hpp
    include/public/kengine/Profiler.hpp
    include/public/kengine/DebugContext.hpp
    include/public/kengine/Math.hpp
    include/public/kengine/Window.hpp
    include/public/kengine/ExecutorService.hpp
    include/public/kengine/StateMachine.hpp
    include/public/kengine/Hashable.hpp
    include/public/kengine/Transform.hpp
    include/public/kengine/Spatial.hpp
    include/public/kengine/SceneGraph.hpp
    include/public/kengine/Bounds.hpp
    include/public/kengine/SpatialGrid.hpp
    include/public/kengine/SpatialPartitioningManager.hpp
    include/public/kengine/FrustumIntersection.hpp
    
    include/public/kengine/vulkan/ImGuiKEContext.hpp
    include/public/kengine/vulkan/VulkanContext.hpp
    include/public/kengine/vulkan/RenderContext.hpp
    include/public/kengine/vulkan/CullContext.hpp
    include/public/kengine/vulkan/ShadowContext.hpp
    include/public/kengine/vulkan/ColorFormatAndSpace.hpp
    include/public/kengine/vulkan/QueueFamilies.hpp
    include/public/kengine/vulkan/VulkanQueue.hpp
    include/public/kengine/vulkan/VulkanInclude.hpp
    include/public/kengine/vulkan/VmaInclude.hpp
    include/public/kengine/vulkan/GpuBuffer.hpp
    include/public/kengine/vulkan/GpuImage.hpp
    include/public/kengine/vulkan/Swapchain.hpp
    include/public/kengine/vulkan/CommandBuffer.hpp
    include/public/kengine/vulkan/GpuBufferCache.hpp
    include/public/kengine/vulkan/CommandPool.hpp
    include/public/kengine/vulkan/QueueOwnerTransfer.hpp
    include/public/kengine/vulkan/Camera.hpp
    include/public/kengine/vulkan/CameraController.hpp
    include/public/kengine/vulkan/SceneData.hpp
    include/public/kengine/vulkan/ShadowCascade.hpp
    include/public/kengine/vulkan/DrawObjectBuffer.hpp
    include/public/kengine/vulkan/IndirectDrawBatch.hpp
    include/public/kengine/vulkan/MaterialBindManager.hpp
    include/public/kengine/vulkan/AsyncAssetCache.hpp
    include/public/kengine/vulkan/SamplerCache.hpp
    include/public/kengine/vulkan/LightsManager.hpp
    include/public/kengine/vulkan/SkeletonManager.hpp

    include/public/kengine/vulkan/mesh/AsyncModelCache.hpp
    include/public/kengine/vulkan/mesh/ModelConfig.hpp
    include/public/kengine/vulkan/mesh/Model.hpp
    include/public/kengine/vulkan/mesh/Mesh.hpp
    include/public/kengine/vulkan/mesh/MeshBuilder.hpp
    include/public/kengine/vulkan/mesh/ModelFactory.hpp
    include/public/kengine/vulkan/mesh/GltfModelFactory.hpp
    include/public/kengine/vulkan/mesh/Vertex.hpp
    include/public/kengine/vulkan/mesh/ModelNode.hpp

    include/public/kengine/vulkan/mesh/anim/AsyncAnimationCache.hpp
    include/public/kengine/vulkan/mesh/anim/AnimationFactory.hpp
    include/public/kengine/vulkan/mesh/anim/GltfAnimationFactory.hpp
    include/public/kengine/vulkan/mesh/anim/AnimationConfig.hpp
    include/public/kengine/vulkan/mesh/anim/Animation.hpp
    include/public/kengine/vulkan/mesh/anim/Skeleton.hpp
    include/public/kengine/vulkan/mesh/anim/BoneTrack.hpp

    include/public/kengine/vulkan/texture/AsyncTextureCache.hpp
    include/public/kengine/vulkan/texture/TextureFactory.hpp
    include/public/kengine/vulkan/texture/TextureConfig.hpp
    include/public/kengine/vulkan/texture/Texture2d.hpp

    include/public/kengine/vulkan/material/AsyncMaterialCache.hpp
    include/public/kengine/vulkan/material/Material.hpp
    include/public/kengine/vulkan/material/MaterialConfig.hpp
    include/public/kengine/vulkan/material/MaterialBindingConfig.hpp
    include/public/kengine/vulkan/material/MaterialBinding.hpp
    include/public/kengine/vulkan/material/PbrMaterialConfig.hpp

    include/public/kengine/vulkan/pipelines/Pipeline.hpp
    include/public/kengine/vulkan/pipelines/PipelineCache.hpp
    include/public/kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp
    include/public/kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp
    include/public/kengine/vulkan/pipelines/PreDrawCullingPipeline.hpp
    include/public/kengine/vulkan/pipelines/DrawCullingPipeline.hpp
    include/public/kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp
    include/public/kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp
    include/public/kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp
    include/public/kengine/vulkan/pipelines/DebugDeferredOffscreenPbrPipeline.hpp

    include/public/kengine/vulkan/descriptor/DescriptorSetLayout.hpp
    include/public/kengine/vulkan/descriptor/DescriptorSetPool.hpp
    include/public/kengine/vulkan/descriptor/DescriptorSetAllocator.hpp

    include/public/kengine/vulkan/renderpass/RenderPass.hpp
    include/public/kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp
    include/public/kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp
)

set(SOURCES
    src/Window.cpp
    src/Transform.cpp
    src/Spatial.cpp
    src/SceneGraph.cpp
    src/Bounds.cpp
    src/SpatialGrid.cpp
    src/FrustumIntersection.cpp
    src/DebugContext.cpp
    
    src/io/FlieSystemAssetIO.cpp
    src/io/MemoryMappedFile.cpp

    src/input/InputManager.cpp
    src/input/EventListener.cpp
    src/input/InputEventAdapter.cpp

    src/terrain/DualGridTileTerrain.cpp
    src/terrain/TileTerrainChunk.cpp

    src/ecs/World.cpp
    
    src/vulkan/tinygltf_impl.cpp
    src/vulkan/ImGuiKEContext.cpp
    src/vulkan/VulkanContext.cpp
    src/vulkan/RenderContext.cpp
    src/vulkan/CullContext.cpp
    src/vulkan/ShadowContext.cpp
    src/vulkan/ColorFormatAndSpace.cpp
    src/vulkan/QueueFamilies.cpp
    src/vulkan/VulkanQueue.cpp
    src/vulkan/GpuBuffer.cpp
    src/vulkan/Swapchain.cpp
    src/vulkan/GpuBufferCache.cpp
    src/vulkan/CommandPool.cpp
    src/vulkan/QueueOwnerTransfer.cpp
    src/vulkan/Camera.cpp
    src/vulkan/CameraController.cpp
    src/vulkan/SceneData.cpp
    src/vulkan/ShadowCascade.cpp
    src/vulkan/DrawObjectBuffer.cpp
    src/vulkan/IndirectDrawBatch.cpp
    src/vulkan/MaterialBindManager.cpp
    src/vulkan/AsyncAssetCache.cpp
    src/vulkan/SamplerCache.cpp
    src/vulkan/LightsManager.cpp
    src/vulkan/SkeletonManager.cpp

    src/vulkan/texture/AsyncTextureCache.cpp
    src/vulkan/texture/TextureFactory.cpp
    src/vulkan/texture/TextureConfig.cpp
    src/vulkan/texture/Texture2d.cpp

    src/vulkan/mesh/AsyncModelCache.cpp
    src/vulkan/mesh/ModelConfig.cpp
    src/vulkan/mesh/Mesh.cpp
    src/vulkan/mesh/Model.cpp
    src/vulkan/mesh/GltfModelFactory.cpp

    src/vulkan/mesh/anim/GltfAnimationFactory.cpp
    src/vulkan/mesh/anim/AsyncAnimationCache.cpp
    src/vulkan/mesh/anim/AnimationConfig.cpp
    src/vulkan/mesh/anim/Animation.cpp
    src/vulkan/mesh/anim/BoneTrack.cpp
    src/vulkan/mesh/anim/Skeleton.cpp

    src/vulkan/material/AsyncMaterialCache.cpp
    src/vulkan/material/Material.cpp
    src/vulkan/material/MaterialConfig.cpp
    src/vulkan/material/MaterialBindingConfig.cpp
    src/vulkan/material/MaterialBinding.cpp
    src/vulkan/material/PbrMaterialConfig.cpp

    src/vulkan/pipelines/Pipeline.cpp
    src/vulkan/pipelines/PipelineCache.cpp
    src/vulkan/pipelines/DeferredOffscreenPbrPipeline.cpp
    src/vulkan/pipelines/SkinnedOffscreenPbrPipeline.cpp
    src/vulkan/pipelines/PreDrawCullingPipeline.cpp
    src/vulkan/pipelines/DrawCullingPipeline.cpp
    src/vulkan/pipelines/CascadeShadowMapPipeline.cpp
    src/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.cpp
    src/vulkan/pipelines/DeferredCompositionPbrPipeline.cpp
    src/vulkan/pipelines/DebugDeferredOffscreenPbrPipeline.cpp

    src/vulkan/descriptor/DescriptorSetLayout.cpp
    src/vulkan/descriptor/DescriptorSetPool.cpp
    src/vulkan/descriptor/DescriptorSetAllocator.cpp

    src/vulkan/renderpass/RenderPass.cpp
    src/vulkan/renderpass/DeferredPbrRenderPass.cpp
    src/vulkan/renderpass/CascadeShadowMapRenderPass.cpp
)

####### Dependencies
include(FetchContent)

# download spdlog source
set(SPDLOG_ENABLE_PCH ON CACHE BOOL "Enable precompiled headers for spdlog" FORCE)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)
set_target_properties(spdlog PROPERTIES FOLDER "thirdparty")

# download tracy source
FetchContent_Declare(
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG v0.11.1
)
FetchContent_MakeAvailable(tracy)
set_target_properties(TracyClient PROPERTIES FOLDER "thirdparty")

# download imgui source
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.1
)
FetchContent_MakeAvailable(imgui)

set(IMGUI
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)

# download glfw source
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)
set_target_properties(glfw PROPERTIES FOLDER "thirdparty/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "thirdparty/GLFW3")

# download JoltPhysics source
#set(JPH_PROFILE_ENABLED ON CACHE BOOL "Enable internal profiling")
#set(JPH_DEBUG_RENDERER ON CACHE BOOL "Enable debug rendering")
#set(JPH_DOUBLE_PRECISION OFF CACHE BOOL "Use double precision for large worlds")
set(JPH_CROSS_PLATFORM_DETERMINISTIC ON CACHE BOOL "Make simulation deterministic across different platforms")
set(PHYSICS_REPO_ROOT ${CMAKE_BINARY_DIR}/_deps/joltphysics-src)
FetchContent_Declare(
    JoltPhysics
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG v5.1.0
    SOURCE_DIR ${PHYSICS_REPO_ROOT}
)
FetchContent_MakeAvailable(JoltPhysics)
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)
set_target_properties(Jolt PROPERTIES FOLDER "thirdparty")

# download msgpack-c source
set(MSGPACK_CXX20 ON CACHE BOOL "Enforce use C++20")
set(MSGPACK_USE_BOOST OFF CACHE BOOL "Remove Boost dependency")
set(MSGPACK_BUILD_DOCS OFF CACHE BOOL "Dont build docs")
FetchContent_Declare(
    msgpack
    GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
    GIT_TAG cpp-6.1.0
)
FetchContent_MakeAvailable(msgpack)

# Gave up. Going to use vcpkg for GameNetworkingSockets
# # zlib needs to be installed on system for now. apparently its an issue in CMAKE due to maintainer: https://discourse.cmake.org/t/how-to-set-zlib-root/9765
# # download zlib source
# # download protobuf source depends on zib
# # download GameNetworkingSockets source


######### 

target_sources(kengine_engine 
    PRIVATE
        ${HEADERS}
        ${SOURCES}
        ${IMGUI}
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    #PREFIX "include"
    FILES ${HEADERS}
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${SOURCES}
)

source_group(
    TREE ${CMAKE_SOURCE_DIR}
    PREFIX "thirdparty"
    FILES ${IMGUI}
)

# imgui
target_include_directories(kengine_engine PUBLIC ${imgui_SOURCE_DIR})

target_include_directories(kengine_engine PUBLIC include/public)
target_include_directories(kengine_engine PRIVATE include/private)
target_include_directories(kengine_engine PRIVATE include/thirdparty)

# find packages tobe installed on system or through vcpkg
find_package(Threads REQUIRED)
find_package(Vulkan 1.3 REQUIRED) # expected to also install VMA and GLM
find_package(GameNetworkingSockets CONFIG REQUIRED)

# is this even needed?
#target_precompile_headers(kengine_engine PRIVATE <spdlog/spdlog.h>)

target_link_libraries(kengine_engine PUBLIC
    spdlog::spdlog
    Tracy::TracyClient
    Threads::Threads
    Vulkan::Vulkan
    glfw
    msgpack-cxx
    #Jolt ## commented out to work on it when i need it, and ensure its linking correctly
    GameNetworkingSockets::shared
)