add_library(engine STATIC)

set(HEADERS
    include/public/kengine/Engine.hpp
    include/public/kengine/Math.hpp
    include/public/kengine/Window.hpp
    include/public/kengine/ExecutorService.hpp
    include/public/kengine/StateMachine.hpp
    
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

    include/public/kengine/vulkan/pipelines/Pipeline.hpp
    include/public/kengine/vulkan/pipelines/PipelineCache.hpp
    include/public/kengine/vulkan/pipelines/DrawCullingPipeline.hpp
    include/public/kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp

    include/public/kengine/vulkan/descriptor/DescriptorSetLayout.hpp
    include/public/kengine/vulkan/descriptor/DescriptorSetPool.hpp
    include/public/kengine/vulkan/descriptor/DescriptorSetAllocator.hpp

    include/public/kengine/vulkan/renderpass/RenderPass.hpp
    include/public/kengine/vulkan/renderpass/RenderTarget.hpp
    include/public/kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp
)

set(SOURCES
    src/Engine.cpp
    src/Window.cpp

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

    src/vulkan/pipelines/Pipeline.cpp
    src/vulkan/pipelines/PipelineCache.cpp
    src/vulkan/pipelines/DrawCullingPipeline.cpp
    src/vulkan/pipelines/CascadeShadowMapPipeline.cpp

    src/vulkan/descriptor/DescriptorSetLayout.cpp
    src/vulkan/descriptor/DescriptorSetPool.cpp
    src/vulkan/descriptor/DescriptorSetAllocator.cpp

    src/vulkan/renderpass/RenderPass.cpp
    src/vulkan/renderpass/RenderTarget.cpp
    src/vulkan/renderpass/DeferredPbrRenderPass.cpp
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

find_package(Threads REQUIRED)
find_package(Vulkan 1.3 REQUIRED) # expected to also install VMA and GLM
find_package(glfw3 3.3 REQUIRED)

target_link_libraries(engine PUBLIC Threads::Threads)
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
target_link_libraries(engine PUBLIC glfw)