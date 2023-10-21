#include "Engine.hpp"
#include <iostream>


Engine::Engine() {
    // Initialize the library
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(640, 480, "Demo", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

void Engine::run() {
    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll for and process events
        glfwPollEvents();
    }
}

Engine:: ~Engine() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
