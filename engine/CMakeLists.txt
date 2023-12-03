add_library(engine)

set(HEADERS
    include/public/Engine.hpp
    include/public/Window.hpp
    include/public/ExecutorService.hpp
    
    include/public/vulkan/VulkanContext.hpp
    include/public/vulkan/ColorFormatAndSpace.hpp
    include/public/vulkan/QueueFamilies.hpp
    include/public/vulkan/VulkanQueue.hpp
    include/public/vulkan/VulkanInclude.hpp
    include/public/vulkan/VmaInclude.hpp
    include/public/vulkan/GpuBuffer.hpp
    include/public/vulkan/GpuImage.hpp
    include/public/vulkan/Swapchain.hpp

    include/public/vulkan/renderpass/RenderPass.hpp
    include/public/vulkan/renderpass/RenderTarget.hpp
    include/public/vulkan/renderpass/DeferredPbrRenderPass.hpp

    include/public/statemachine/StateMachine.hpp
)

set(SOURCES
    src/Engine.cpp
    src/Window.cpp

    src/vulkan/VulkanContext.cpp
    src/vulkan/ColorFormatAndSpace.cpp
    src/vulkan/QueueFamilies.cpp
    src/vulkan/VulkanQueue.cpp
    src/vulkan/GpuBuffer.cpp
    src/vulkan/Swapchain.cpp

    src/vulkan/renderpass/RenderPass.cpp
    src/vulkan/renderpass/RenderTarget.cpp
    src/vulkan/renderpass/DeferredPbrRenderPass.cpp
    
    src/statemachine/StateMachine.cpp
)

target_sources(engine 
    PUBLIC 
        ${HEADERS} 
        ${SOURCES}
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${HEADERS} ${SOURCES}
)

target_include_directories(engine PUBLIC include/public)
target_include_directories(engine PRIVATE include/private)
target_include_directories(engine PUBLIC D:/git/VulkanMemoryAllocator/include)

find_package(Threads REQUIRED)
find_package(Vulkan 1.3 REQUIRED)
find_package(glfw3 3.3 REQUIRED HINTS "C:/Program Files/GLFW/lib/cmake/glfw3")
find_package(glm REQUIRED HINTS "D:/git/glm/cmake/glm")

target_link_libraries(engine PUBLIC Threads::Threads)
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
target_link_libraries(engine PUBLIC glfw)
target_link_libraries(engine PUBLIC glm::glm)