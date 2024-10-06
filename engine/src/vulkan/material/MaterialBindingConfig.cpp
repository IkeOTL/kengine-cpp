#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <kengine/vulkan/material/MaterialBinding.hpp>

#include <future>


std::unique_ptr<MaterialBinding> ImageBindingConfig::getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) {
    auto& texture = textureCache.get(textureConfig);
    return std::make_unique<ImageBinding>(shared_from_this(), texture);
}

// maybe change return type to AsyncCacheTask instead
std::unique_ptr<MaterialBinding> BufferBindingConfig::getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) {
    auto buf = bufferCache.get(bufferCacheKey);

    if (!buf)
        throw std::runtime_error("Buffer not found.");

    return std::make_unique<BufferBinding>(shared_from_this(), *buf);
}

std::unique_ptr<MaterialBinding> ImageArrayBindingConfig::getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) {
    auto count = textureConfigs.size();

    std::vector<Texture2d*> textures;
    textures.reserve(count);

    std::vector<AsyncCacheTask<Texture2d*>> tasks;
    tasks.reserve(count);

    for (auto i = 0; i < count; i++)
        tasks.push_back(textureCache.getAsync(textureConfigs[i]));

    for (auto i = 0; i < count; i++)
        textures.push_back(tasks[i].get());

    return std::make_unique<ImageArrayBinding>(shared_from_this(), textures);
}

size_t BufferBindingConfig::hash() const {
    int hash = 5;
    hash = 71 * hash + bufferCacheKey;
    return hash;
}

size_t ImageBindingConfig::hash() const {
    return textureConfig->hashCode();
}

size_t ImageArrayBindingConfig::hash() const {
    size_t hash = 29;
    for(auto& c : textureConfigs)
        hash = 61 * hash + c->hashCode();
    return hash;
}

size_t MaterialBindingConfig::hashCode() const noexcept {
    size_t hash = 5;
    hash = 73 * hash + descriptorSetIndex;
    hash = 73 * hash + bindingIndex;
    hash = 73 * hash + this->hash();
    return hash;
}