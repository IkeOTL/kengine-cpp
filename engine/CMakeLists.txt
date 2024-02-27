add_library(engine STATIC)

#include(CheckCXXCompilerFlag)
#check_cxx_compiler_flag(-fsanitize=address HAS_ASAN)
#if (HAS_ASAN)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")
#endif()

set(HEADERS
    include/public/kengine/util/MapUtils.hpp
    include/public/kengine/util/VecUtils.hpp

    include/public/kengine/io/AssetIO.hpp
    include/public/kengine/io/MemoryMappedFile.hpp

    include/public/kengine/Engine.hpp
    include/public/kengine/Math.hpp
    include/public/kengine/Window.hpp
    include/public/kengine/ExecutorService.hpp
    include/public/kengine/StateMachine.hpp
    include/public/kengine/Hashable.hpp
    include/public/kengine/Transform.hpp
    include/public/kengine/Spatial.hpp
    
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

    include/public/kengine/vulkan/mesh/Mesh.hpp
    include/public/kengine/vulkan/mesh/MeshBuilder.hpp
    include/public/kengine/vulkan/mesh/ModelFactory.hpp
    include/public/kengine/vulkan/mesh/GltfModelFactory.hpp
    include/public/kengine/vulkan/mesh/Vertex.hpp
    include/public/kengine/vulkan/mesh/ModelNode.hpp
    include/public/kengine/vulkan/mesh/Model.hpp

    include/public/kengine/vulkan/mesh/anim/Skeleton.hpp

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
    include/public/kengine/vulkan/pipelines/DrawCullingPipeline.hpp
    include/public/kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp
    include/public/kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp
    include/public/kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp

    include/public/kengine/vulkan/descriptor/DescriptorSetLayout.hpp
    include/public/kengine/vulkan/descriptor/DescriptorSetPool.hpp
    include/public/kengine/vulkan/descriptor/DescriptorSetAllocator.hpp

    include/public/kengine/vulkan/renderpass/RenderPass.hpp
    include/public/kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp
    include/public/kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp
)

set(SOURCES
    src/Engine.cpp
    src/Window.cpp
    src/Transform.cpp
    src/Spatial.cpp
    
    src/io/FlieSystemAssetIO.cpp
    src/io/MemoryMappedFile.cpp

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

    src/vulkan/texture/AsyncTextureCache.cpp
    src/vulkan/texture/TextureFactory.cpp
    src/vulkan/texture/TextureConfig.cpp
    src/vulkan/texture/Texture2d.cpp

    src/vulkan/mesh/Mesh.cpp
    src/vulkan/mesh/MeshBuilder.cpp
    src/vulkan/mesh/Model.cpp
    src/vulkan/mesh/GltfModelFactory.cpp

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
    src/vulkan/pipelines/DrawCullingPipeline.cpp
    src/vulkan/pipelines/CascadeShadowMapPipeline.cpp
    src/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.cpp
    src/vulkan/pipelines/DeferredCompositionPbrPipeline.cpp

    src/vulkan/descriptor/DescriptorSetLayout.cpp
    src/vulkan/descriptor/DescriptorSetPool.cpp
    src/vulkan/descriptor/DescriptorSetAllocator.cpp

    src/vulkan/renderpass/RenderPass.cpp
    src/vulkan/renderpass/DeferredPbrRenderPass.cpp
    src/vulkan/renderpass/CascadeShadowMapRenderPass.cpp
)

target_sources(engine 
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

find_package(Threads REQUIRED)
find_package(Vulkan 1.3 REQUIRED) # expected to also install VMA and GLM
find_package(glfw3 3.3 REQUIRED)

target_link_libraries(engine PUBLIC Threads::Threads)
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
target_link_libraries(engine PUBLIC glfw)