
#include "MyPlayerContext.hpp"

std::unique_ptr<MyPlayerContext> MyPlayerContext::create(PhysicsContext& physicsContext) {
    auto settings = std::make_unique<JPH::CharacterVirtualSettings>();


    return std::make_unique<MyPlayerContext>(physicsContext);
}
