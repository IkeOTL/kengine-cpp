#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>


class PipelineCache {
private:
    std::unordered_map<std::type_index, std::unique_ptr<Pipeline>> pipelines{};

public:
    inline static const DescriptorSetLayoutConfig globalLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT }
    };

    inline static const std::unique_ptr<PipelineCache> create() {
        return std::make_unique<PipelineCache>();
    };

    template <typename T>
    T& createPipeline() {
        auto pipeline = std::make_unique<T>();
        auto& outPipeline = pipelines[std::type_index(typeid(T))] = std::move(pipeline);
        return *static_cast<T*>(outPipeline.get());
    }

    template <typename T>
    T& getPipeline() {
        auto it = pipelines.find(std::type_index(typeid(T)));
        if (it == pipelines.end())
            throw std::runtime_error("Pipeline not found.");

        auto pipeline = static_cast<T*>(it->second.get());
        if (!pipeline)
            throw std::runtime_error("Pipeline failed to be cast.");

        return *pipeline;
    }

    Pipeline& getPipeline(std::type_index typeIdx) {
        auto it = pipelines.find(typeIdx);
        if (it == pipelines.end())
            throw std::runtime_error("Pipeline not found.");

        return *(it->second);
    }
};