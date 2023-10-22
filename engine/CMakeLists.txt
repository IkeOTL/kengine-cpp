add_library(engine)

target_sources(engine 
    PRIVATE
        src/Engine.cpp
        src/Window.cpp
        src/VulkanContext.cpp

        include/public/Engine.hpp
        include/public/Window.hpp
        include/public/VulkanContext.hpp
        include/public/ColorFormatAndSpace.hpp

        include/public/ExecutorService.hpp
        include/public/ColorFormatAndSpace.hpp
)

target_include_directories(engine PUBLIC include/public)
target_include_directories(engine PRIVATE include/private)

find_package(Vulkan 1.3 REQUIRED)
find_package(glfw3 3.3 REQUIRED HINTS "C:/Program Files/GLFW/lib/cmake/glfw3")
find_package(glm REQUIRED HINTS "D:/git/glm/cmake/glm")

target_link_libraries(engine PUBLIC Vulkan::Vulkan)
target_link_libraries(engine PUBLIC glfw)
target_link_libraries(engine PUBLIC glm::glm)