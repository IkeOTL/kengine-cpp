#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>

#include <cstdint>
#include <thread>
#include <memory>
#include <cassert>

/// Layer that objects can be in, determines which other objects it can collide with
namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer DEBRIS = 2; // Example: Debris collides only with NON_MOVING
    static constexpr JPH::ObjectLayer SENSOR = 3; // Sensors only collide with MOVING objects
    static constexpr JPH::ObjectLayer NUM_LAYERS = 4;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
        switch (inObject1)
        {
        case Layers::NON_MOVING:
            return inObject2 == Layers::MOVING || inObject2 == Layers::DEBRIS;
        case Layers::MOVING:
            return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING || inObject2 == Layers::SENSOR;
        case Layers::DEBRIS:
            return inObject2 == Layers::NON_MOVING;
        case Layers::SENSOR:
            return inObject2 == Layers::MOVING;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }
};

namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr JPH::BroadPhaseLayer DEBRIS(2);
    static constexpr JPH::BroadPhaseLayer SENSOR(3);
    static constexpr JPH::BroadPhaseLayer UNUSED(4);
    static constexpr JPH::uint NUM_LAYERS(5);
};

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl() {
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[Layers::DEBRIS] = BroadPhaseLayers::DEBRIS;
        mObjectToBroadPhase[Layers::SENSOR] = BroadPhaseLayers::SENSOR;
    }

    virtual JPH::uint GetNumBroadPhaseLayers() const override {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        assert(inLayer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        switch ((JPH::BroadPhaseLayer::Type)inLayer)
        {
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
            return "NON_MOVING";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
            return "MOVING";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::DEBRIS:
            return "DEBRIS";
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
    JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
        case Layers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::MOVING || inLayer2 == BroadPhaseLayers::DEBRIS;
        case Layers::MOVING:
            return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING || inLayer2 == BroadPhaseLayers::SENSOR;
        case Layers::DEBRIS:
            return inLayer2 == BroadPhaseLayers::NON_MOVING;
        case Layers::SENSOR:
            return inLayer2 == BroadPhaseLayers::MOVING;
        default:
            assert(false);
            return false;
        }
    }
};

class PhysicsContext {
private:
    JPH::PhysicsSettings physicsSettings;
    std::unique_ptr<JPH::TempAllocator> tempAllocator = nullptr;
    std::unique_ptr<JPH::JobSystem> jobSystem = nullptr;
    std::unique_ptr<JPH::PhysicsSystem> physicsSystem = nullptr;
    BPLayerInterfaceImpl broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl objectVsObjectLayerFilter;
    //ContactListenerImpl* mContactListener = nullptr;

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

    void init();
    void step(float delta);
};