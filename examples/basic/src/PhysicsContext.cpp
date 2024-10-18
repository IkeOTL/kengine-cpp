
#include "PhysicsContext.hpp"


void PhysicsContext::init() {
    // todo: implement our own allocator
    JPH::RegisterDefaultAllocator();

    tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(32 * 1024 * 1024);

    //need to implement our own job system
    jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    physicsSystem = std::make_unique<JPH::PhysicsSystem>();
    physicsSystem->Init(cNumBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broadPhaseLayerInterface, objectVsBroadPhaseLayerFilter, objectVsObjectLayerFilter);
    physicsSystem->SetPhysicsSettings(physicsSettings);

    physicsSystem->SetGravity(JPH::Vec3(0, -10, 0));
}

void PhysicsContext::step(float delta) {
    physicsSystem->Update(delta, 1, tempAllocator.get(), jobSystem.get());
}