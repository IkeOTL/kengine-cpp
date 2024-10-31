#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>

#include <cstdint>
#include <thread>
#include <memory>
#include <cassert>
#include <EASTL/unordered_map.h>
#include <kengine/Logger.hpp>

/// Layer that objects can be in, determines which other objects it can collide with
namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer PLAYER = 2;
    static constexpr JPH::ObjectLayer TERRAIN = 3;
    static constexpr JPH::ObjectLayer SENSOR = 4;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 5;
}; // namespace Layers

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
        switch (inObject1) {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING;
            case Layers::MOVING:
                return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING
                    || inObject2 == Layers::TERRAIN || inObject2 == Layers::SENSOR;
            case Layers::TERRAIN:
                return inObject2 == Layers::MOVING;
            case Layers::PLAYER:
                return inObject2 == Layers::MOVING || inObject2 == Layers::NON_MOVING;
            default:
                JPH_ASSERT(false);
                return false;
        }
    }
};

namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr JPH::BroadPhaseLayer PLAYER(2);
    static constexpr JPH::BroadPhaseLayer TERRAIN(3);
    static constexpr JPH::BroadPhaseLayer SENSOR(4);
    static constexpr JPH::BroadPhaseLayer UNUSED(5);
    static constexpr JPH::uint NUM_LAYERS(5);
}; // namespace BroadPhaseLayers

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    BPLayerInterfaceImpl() {
        objectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        objectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        objectToBroadPhase[Layers::PLAYER] = BroadPhaseLayers::PLAYER;
        objectToBroadPhase[Layers::TERRAIN] = BroadPhaseLayers::TERRAIN;
        objectToBroadPhase[Layers::SENSOR] = BroadPhaseLayers::SENSOR;
    }

    virtual JPH::uint GetNumBroadPhaseLayers() const override {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
        assert(inLayer < Layers::NUM_LAYERS);
        return objectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
        switch ((JPH::BroadPhaseLayer::Type)inLayer) {
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                return "MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::PLAYER:
                return "PLAYER";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::TERRAIN:
                return "TERRAIN";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::SENSOR:
                return "SENSOR";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::UNUSED:
                return "UNUSED";
            default:
                assert(false);
                return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
    JPH::BroadPhaseLayer objectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
        switch (inLayer1) {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING
                    || inLayer2 == BroadPhaseLayers::TERRAIN || inLayer2 == BroadPhaseLayers::SENSOR;
            case Layers::TERRAIN:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::PLAYER:
                return inLayer2 == BroadPhaseLayers::MOVING || inLayer2 == BroadPhaseLayers::NON_MOVING;
            default:
                assert(false);
                return false;
        }
    }
};

class MyContactListener : public JPH::ContactListener {
public:
    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override {
        //   KE_LOG_INFO("Contact validate callback");
        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override {
        //   KE_LOG_INFO("A contact was added");
    }

    virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override {
        //   KE_LOG_INFO("A contact was persisted");
    }

    virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override {
        //    KE_LOG_INFO("A contact was removed");
    }
};

class MyBodyActivationListener : public JPH::BodyActivationListener {
public:
    virtual void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override {
        //  KE_LOG_INFO("A body got activated");
    }

    virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override {
        //    KE_LOG_INFO("A body went to sleep");
    }
};

class PhysicsContext {
private:
    JPH::PhysicsSettings physicsSettings;

    std::unique_ptr<JPH::Factory> factory = nullptr;
    std::unique_ptr<JPH::TempAllocator> tempAllocator = nullptr;
    std::unique_ptr<JPH::JobSystem> jobSystem = nullptr;
    std::unique_ptr<JPH::PhysicsSystem> physicsSystem = nullptr;
    BPLayerInterfaceImpl broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl objectVsObjectLayerFilter;

    MyContactListener contactListener;
    MyBodyActivationListener activationListener;

    bool paused = false;

    static constexpr JPH::uint cNumBodies = 10240;
    static constexpr JPH::uint cNumBodyMutexes = 0; // Autodetect
    static constexpr JPH::uint cMaxBodyPairs = 65536;
    static constexpr JPH::uint cMaxContactConstraints = 20480;

public:
    PhysicsContext() = default;
    ~PhysicsContext() = default;

    inline static std::unique_ptr<PhysicsContext> create() {
        return std::make_unique<PhysicsContext>();
    }

    void setPaused(bool b) {
        paused = b;
    }

    bool isPaused() const {
        return paused;
    }

    JPH::PhysicsSystem& getPhysics() {
        return *physicsSystem;
    }

    JPH::TempAllocator& getTempAllocator() {
        return *tempAllocator;
    }

    void init();
    void step(float delta);
};