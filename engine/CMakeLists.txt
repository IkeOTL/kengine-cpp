add_library(engine)

target_sources(engine 
    PUBLIC
        src/Engine.cpp
        src/Window.cpp
        src/VulkanContext.cpp
        src/ColorFormatAndSpace.cpp
        src/QueueFamilies.cpp
        src/VulkanQueue.cpp
        src/GpuBuffer.cpp
        src/Swapchain.cpp

        src/renderpass/RenderPass.cpp
        src/renderpass/RenderTarget.cpp
        src/renderpass/DeferredPbrRenderPass.cpp

        include/public/Engine.hpp
        include/public/Window.hpp
        include/public/VulkanContext.hpp
        include/public/ColorFormatAndSpace.hpp
        include/public/QueueFamilies.hpp
        include/public/VulkanQueue.hpp
        include/public/VulkanInclude.hpp
        include/public/VmaInclude.hpp
        include/public/GpuBuffer.hpp
        include/public/GpuImage.hpp
        include/public/Swapchain.hpp
        include/public/ExecutorService.hpp

        include/public/renderpass/RenderPass.hpp
        include/public/renderpass/RenderTarget.hpp
        include/public/renderpass/DeferredPbrRenderPass.hpp
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