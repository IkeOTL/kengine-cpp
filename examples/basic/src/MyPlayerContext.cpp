
#include "MyPlayerContext.hpp"
#include <glm/trigonometric.hpp>
#include <jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <jolt/Physics/Collision/BackFaceMode.h>
//#include <glm/>

std::unique_ptr<MyPlayerContext> MyPlayerContext::create(PhysicsContext& physicsContext) {
    auto playerCtx = std::make_unique<MyPlayerContext>(physicsContext);

    // setup physics
    {
        auto characterRadius = 0.75 / 2.0f;
        auto characterHeight = 2.0f;
        auto shape = JPH::RotatedTranslatedShapeSettings(
            JPH::Vec3(0, characterHeight, 0),
            JPH::Quat::sIdentity(),
            new JPH::CapsuleShape(0.5f * (characterHeight - characterRadius * 2), characterRadius)
        ).Create().Get();

        auto settings = std::make_unique<JPH::CharacterVirtualSettings>();

        settings->mMaxSlopeAngle = glm::radians(45.0f);
        settings->mMaxStrength = 1000.0f;
        settings->mShape = shape;
        settings->mBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;
        settings->mCharacterPadding = 0.02f;
        settings->mPenetrationRecoverySpeed = 1.0f;
        settings->mPredictiveContactDistance = 0.1f;
        settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -characterRadius); // Accept contacts that touch the lower sphere of the capsule
        settings->mEnhancedInternalEdgeRemoval = false;
        settings->mInnerBodyShape = nullptr;
        settings->mInnerBodyLayer = Layers::MOVING;
        auto physicsCharacter = std::make_unique<JPH::CharacterVirtual>(settings.get(), JPH::RVec3(5, 4, 5), JPH::Quat::sIdentity(), 0, &physicsContext.getPhysics());

        playerCtx->setPhysicsCharacter(std::move(physicsCharacter), std::move(settings));
    }

    return playerCtx;
}



void MyPlayerContext::setPhysicsCharacter(std::unique_ptr<JPH::CharacterVirtual>&& p, std::unique_ptr<JPH::CharacterVirtualSettings>&& s) {
    physicsCharacter = std::move(p);
    physicsCharacterSettings = std::move(s);
}