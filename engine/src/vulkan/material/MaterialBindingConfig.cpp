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

int BufferBindingConfig::hash() const {
    int hash = 5;
    hash = 71 * hash + bufferCacheKey;
    return hash;
}

int ImageBindingConfig::hash() const {
    return textureConfig->hashCode();
}

size_t MaterialBindingConfig::hashCode() const noexcept {
    int hash = 5;
    hash = 73 * hash + descriptorSetIndex;
    hash = 73 * hash + bindingIndex;
    hash = 73 * hash + this->hash();
    return hash;
}