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

    auto assetData = assetIo.load(config.getAnimationSet());

    auto ret = gltfLoader.LoadBinaryFromMemory(&model, &err, &warn, assetData->data(), assetData->length());

    tinygltf::Animation* anim = nullptr;
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
    auto boneCount = 0;
    std::unordered_map<uint32_t, std::string> boneNames;
    std::unordered_map<uint32_t, uint32_t> boneMap;
    for (size_t i = 0; i < model.nodes.size(); i++) {
        auto& node = model.nodes[i];

        // normal node, aka not a joint
        if (uniqueBoneNodeIndices.find(i) == uniqueBoneNodeIndices.end())
            continue;

        auto boneId = boneCount++;
        boneMap[i] = boneId;
        boneNames[boneId] = node.name;
    }

    // map samplers so we can load 
    struct BoneChannels {
        std::vector<float> translationTime;
        std::vector<glm::vec3> translation;
        std::vector<float> rotationTime;
        std::vector<glm::quat> rotation;
        std::vector<float> scaleTime;
        std::vector<glm::vec3> scale;
        float lastFrameTime = 0;
    };

    std::unordered_map<uint32_t, BoneChannels> boneChannels;

    for (auto& ch : anim->channels) {
        auto boneId = boneMap[ch.target_node];
        auto& mapping = boneChannels[boneId];
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
                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

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

                const auto stride = bufferView.byteStride ? bufferView.byteStride : sizeof(glm::vec3);

                mapping.scale.resize(accessor.count);
                for (auto i = 0; i < accessor.count; i++)
                    memcpy(&mapping.scale[i], data + (i * stride), sizeof(glm::vec3));
            }
        }
    }

    const int ticksPerSecond = 30;
    std::vector<BoneTrack> tracks;
    tracks.reserve(boneCount);
    // sample
    {
        for (int boneId = 0; boneId < boneChannels.size(); boneId++) {
            std::vector<float> times;
            std::vector<glm::vec3> translation;
            std::vector<glm::quat> rotation;
            std::vector<glm::vec3> scale;

            auto& trackData = boneChannels[boneId];

            // fill times
            {
                float trackDuration = trackData.lastFrameTime;
                int numSamples = static_cast<int>(ticksPerSecond) + static_cast<int>(trackDuration * ticksPerSecond);

                times.reserve(numSamples);

                for (auto s = 0; s < numSamples; ++s)
                    times.push_back(trackDuration * (static_cast<float>(s) / static_cast<float>(numSamples - 1)));
            }

            // sample translation
            {
                auto& trackTimes = trackData.translationTime;
                float trackDuration = trackTimes[trackTimes.size() - 1] - trackTimes[0];
                int numSamples = static_cast<int>(ticksPerSecond) + static_cast<int>(trackDuration * ticksPerSecond);

                translation.reserve(numSamples);

                int numFrames = trackData.translation.size();
                for (auto s = 0; s < numSamples; ++s) {
                    float t = static_cast<float>(s) / static_cast<float>(numSamples - 1);
                    float time = t * trackDuration + trackTimes[0];

                    int frameIndex = 0;
                    for (auto j = numFrames - 1; j >= 0; --j) {
                        if (time >= trackTimes[j]) {
                            frameIndex = j;
                            if (frameIndex >= numFrames - 2)
                                frameIndex = numFrames - 2;
                            break;
                        }
                    }

                    translation.push_back(glm::mix(trackData.translation[frameIndex], trackData.translation[frameIndex + 1], t));
                }
            }

            // sample rotation
            {
                auto& trackTimes = trackData.rotationTime;
                float trackDuration = trackTimes[trackTimes.size() - 1] - trackTimes[0];
                int numSamples = static_cast<int>(ticksPerSecond) + static_cast<int>(trackDuration * ticksPerSecond);

                rotation.reserve(numSamples);

                int numFrames = trackData.rotation.size();
                for (auto s = 0; s < numSamples; ++s) {
                    float t = static_cast<float>(s) / static_cast<float>(numSamples - 1);
                    float time = t * trackDuration + trackTimes[0];

                    int frameIndex = 0;
                    for (auto j = numFrames - 1; j >= 0; --j) {
                        if (time >= trackTimes[j]) {
                            frameIndex = j;
                            if (frameIndex >= numFrames - 2)
                                frameIndex = numFrames - 2;
                            break;
                        }
                    }

                    rotation.push_back(glm::mix(trackData.rotation[frameIndex], trackData.rotation[frameIndex + 1], t));
                }
            }

            // sample scale
            {
                auto& trackTimes = trackData.scaleTime;
                float trackDuration = trackTimes[trackTimes.size() - 1] - trackTimes[0];
                int numSamples = static_cast<int>(ticksPerSecond) + static_cast<int>(trackDuration * ticksPerSecond);

                scale.reserve(numSamples);

                int numFrames = trackData.scale.size();
                for (auto s = 0; s < numSamples; ++s) {
                    float t = static_cast<float>(s) / static_cast<float>(numSamples - 1);
                    float time = t * trackDuration + trackTimes[0];

                    int frameIndex = 0;
                    for (auto j = numFrames - 1; j >= 0; --j) {
                        if (time >= trackTimes[j]) {
                            frameIndex = j;
                            if (frameIndex >= numFrames - 2)
                                frameIndex = numFrames - 2;
                            break;
                        }
                    }

                    scale.push_back(glm::mix(trackData.scale[frameIndex], trackData.scale[frameIndex + 1], t));
                }
            }

            BoneTrack boneTrack(boneId, boneNames[boneId], std::move(times), std::move(translation), std::move(rotation), std::move(scale));
            tracks.push_back(std::move(boneTrack));
        }
    }

    return std::make_unique<Animation>(config.getAnimationName(), ticksPerSecond, 0, std::move(tracks));
}