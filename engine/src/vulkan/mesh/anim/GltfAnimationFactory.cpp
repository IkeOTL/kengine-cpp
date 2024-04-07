#include <kengine/vulkan/mesh/anim/GltfAnimationFactory.hpp>
#include <kengine/vulkan/mesh/anim/Animation.hpp>
#include <kengine/vulkan/mesh/anim/AnimationConfig.hpp>
#include <kengine/vulkan/mesh/anim/BoneTrack.hpp>
#include <kengine/io/AssetIO.hpp>
#include <unordered_set>

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

    // find all joint indices for first skin
    std::unordered_set<uint32_t> uniqueBoneNodeIndices;
    if (!model.skins.empty())
        for (const auto jointIndex : model.skins[0].joints)
            uniqueBoneNodeIndices.insert(jointIndex);

    // nodeId to boneId
    auto currentBoneId = 0;
    std::unordered_map<uint32_t, uint32_t> boneMap;
    for (size_t i = 0; i < model.nodes.size(); i++) {
        auto& node = model.nodes[i];

        // normal node, aka not a joint
        if (uniqueBoneNodeIndices.find(i) == uniqueBoneNodeIndices.end())
            continue;

        boneMap[i] = currentBoneId++;
    }

    // map samplers so we can load 
    struct BoneChannelSamplerMapping {
        std::vector<float> translationTime;
        std::vector<glm::vec3> translation;
        std::vector<float> rotationTime;
        std::vector<glm::quat> rotation;
        std::vector<float> scaleTime;
        std::vector<glm::vec3> scale;
        float lastFrameTime;
    };

    std::unordered_map<uint32_t, BoneChannelSamplerMapping> boneChannelSamplers;

    for (auto& ch : anim->channels) {
        auto boneId = boneMap[ch.target_node];
        auto& mapping = boneChannelSamplers[boneId];
        auto& sampler = anim->samplers[ch.sampler];

        if (ch.target_path == "translation") {
            // load times
            {
                const auto& accessor = model.accessors[sampler.input];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const auto* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                // todo: handle doubles
                if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw std::runtime_error("Animation data is expect to be floats.");

                mapping.translationTime.resize(accessor.count);
                memcpy(mapping.translationTime.data(), data, accessor.count * sizeof(float));

                auto max = *std::max_element(mapping.translationTime.begin(), mapping.translationTime.end());
                mapping.lastFrameTime = mapping.lastFrameTime > max ? mapping.lastFrameTime : max;
            }

            // load translations
            {
                const auto& accessor = model.accessors[sampler.output];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const auto* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                // todo: handle doubles
                if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw std::runtime_error("Animation data is expect to be floats.");

                const auto stride = bufferView.byteStride ? bufferView.byteStride : sizeof(glm::vec3);

                mapping.translation.resize(accessor.count);
                for (auto i = 0; i < accessor.count; i++)
                    memcpy(&mapping.translation[i], data + (i * stride), sizeof(glm::vec3));
            }
        }
        else if (ch.target_path == "rotation") {
            // load times
            {
                const auto& accessor = model.accessors[sampler.input];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const auto* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                // todo: handle doubles
                if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw std::runtime_error("Animation data is expect to be floats.");

                mapping.rotationTime.resize(accessor.count);
                memcpy(mapping.rotationTime.data(), data, accessor.count * sizeof(float));

                auto max = *std::max_element(mapping.rotationTime.begin(), mapping.rotationTime.end());
                mapping.lastFrameTime = mapping.lastFrameTime > max ? mapping.lastFrameTime : max;
            }

            // load rotations
            {
                const auto& accessor = model.accessors[sampler.output];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const auto* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                // todo: handle doubles
                if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw std::runtime_error("Animation data is expect to be floats.");

                const auto stride = bufferView.byteStride ? bufferView.byteStride : sizeof(glm::quat);

                mapping.rotation.resize(accessor.count);
                for (auto i = 0; i < accessor.count; i++)
                    memcpy(&mapping.rotation[i], data + (i * stride), sizeof(glm::quat));
            }
        }
        else if (ch.target_path == "scale") {
            // load times
            {
                const auto& accessor = model.accessors[sampler.input];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const auto* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                // todo: handle doubles
                if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw std::runtime_error("Animation data is expect to be floats.");

                mapping.scaleTime.resize(accessor.count);
                memcpy(mapping.scaleTime.data(), data, accessor.count * sizeof(float));

                auto max = *std::max_element(mapping.scaleTime.begin(), mapping.scaleTime.end());
                mapping.lastFrameTime = mapping.lastFrameTime > max ? mapping.lastFrameTime : max;
            }

            // load scales
            {
                const auto& accessor = model.accessors[sampler.output];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const auto* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                // todo: handle doubles
                if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw std::runtime_error("Animation data is expect to be floats.");

                const auto stride = bufferView.byteStride ? bufferView.byteStride : sizeof(glm::quat);

                mapping.scale.resize(accessor.count);
                for (auto i = 0; i < accessor.count; i++)
                    memcpy(&mapping.scale[i], data + (i * stride), sizeof(glm::quat));
            }
        }
    }

    std::unordered_map<uint32_t, std::unique_ptr<BoneTrack>> tracks;

    return nullptr;
}