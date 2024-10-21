#include "BasicGameTest.hpp"

#include <glm/glm.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/math.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <steam/steamnetworkingsockets.h>
#include <kengine/Logger.hpp>
#include <kengine/EngineConfig.hpp>

void showMenu() {
    std::cout << "==== Demo Selection Menu ====\n";
    std::cout << "1. Basic Demo\n";
    std::cout << "0. Exit\n";
    std::cout << "Please enter the number of your choice: ";
}

int main() {
    KE_LOG_INFO("Example started.");

    //EngineConfig::getInstance().setDebugRenderingEnabled(true);
    EngineConfig::getInstance().setAssetRoot("../res/");

    bool running = true;
    while (running) {
        showMenu();

        int choice;
        std::cin >> choice;

        switch (choice) {
        case 1:
            std::cout << "Starting Basic Demo...\n";
            {
                BasicGameTest game;
                game.run();
            }
            break;

        case 0:
            std::cout << "Exiting program...\n";
            running = false;
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }

        std::cout << "\n";
    }

    return 0;
}