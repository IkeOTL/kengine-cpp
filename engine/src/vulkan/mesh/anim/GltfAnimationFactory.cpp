#define TINYGLTF_IMPLEMENTATION

#include <kengine/vulkan/mesh/anim/GltfAnimationFactory.hpp>
#include <kengine/vulkan/mesh/anim/AnimationConfig.hpp>
#include <kengine/io/AssetIO.hpp>

thread_local tinygltf::TinyGLTF GltfAnimationFactory::gltfLoader{};

std::unique_ptr<Animation> GltfAnimationFactory::loadAnimation(const AnimationConfig& config) {
    tinygltf::Model model;
    std::string err, warn;

    auto assetData = assetIo.loadBuffer(config.getAnimationSet());

    auto ret = gltfLoader.LoadBinaryFromMemory(&model, &err, &warn, assetData->data(), assetData->length());

    tinygltf::Animation* anim;
    for (auto& a : model.animations)
        if (a.name == config.getAnimationName()) {
            anim = &a;
            break;
        }

    if (!anim)
        throw std::runtime_error("Animation not found. " + config.getAnimationName());


}