#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>


class PipelineCache {
private:
    std::unordered_map<std::type_index, std::unique_ptr<Pipeline>> pipelines;

public:
    const static DescriptorSetLayoutConfig globalLayout;

    void addPipeline(std::unique_ptr<Pipeline>&& pipeline) {
        pipelines[std::type_index(typeid(*pipeline))] = std::move(pipeline);
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

    Pipeline& getPipeline(const std::type_index& typeIdx) {
        auto it = pipelines.find(typeIdx);
        if (it == pipelines.end())
            throw std::runtime_error("Pipeline not found.");

        return *(it->second);
    }
};