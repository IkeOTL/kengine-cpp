#define TINYGLTF_IMPLEMENTATION

#include <kengine/vulkan/mesh/anim/GltfAnimationFactory.hpp>
#include <kengine/vulkan/mesh/anim/AnimationConfig.hpp>

thread_local tinygltf::TinyGLTF GltfAnimationFactory::gltfLoader{};

std::unique_ptr<Animation> GltfAnimationFactory::loadAnimation(const AnimationConfig& config) {

}