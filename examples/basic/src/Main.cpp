#include <steam/steamnetworkingsockets.h>

#include <glm/glm.hpp>
#include <kengine/EngineConfig.hpp>
#include <kengine/Logger.hpp>
#include <kengine/math.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>

#include "BasicGameTest.hpp"
#include "net/GameServer.hpp"
#include "net/GameClient.hpp"

void showMenu() {
    std::cout << "==== Demo Selection Menu ====\n";
    std::cout << "1. Basic Demo\n";
    std::cout << "2. Demo Server\n";
    std::cout << "3. Demo Client\n";
    std::cout << "0. Exit\n";
    std::cout << "Please enter the number of your choice: ";
}

int main() {
    KE_LOG_INFO("Example started.");

    // EngineConfig::getInstance().setDebugRenderingEnabled(true);
    ke::EngineConfig::getInstance().setAssetRoot("../../res/");

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
            case 2:
                std::cout << "Starting Demo Server...\n";
                {
                    ke::GameServer srv;
                    srv.connect();
                    while (true) {
                        srv.tick();
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
                break;
            case 3:
                std::cout << "Starting Demo Client...\n";
                {
                    ke::GameClient c;
                    c.connect();
                    while (true) {
                        c.tick();
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
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