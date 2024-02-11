#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <kengine/vulkan/material/MaterialBinding.hpp>

#include <future>

size_t MaterialBindingConfig::hashCode() const noexcept {
    int hash = 5;
    hash = 73 * hash + descriptorSetIndex;
    hash = 73 * hash + bindingIndex;
    hash = 73 * hash + this->hash();
    return hash;
}

std::future<std::unique_ptr<MaterialBinding>> BufferBindingConfig::getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) {
    auto buf = bufferCache.get(bufferCacheKey);

    if (!buf)
        throw std::runtime_error("Buffer not found.");

    std::promise<std::unique_ptr<MaterialBinding>> promise;
    promise.set_value(std::make_unique<BufferBinding>(shared_from_this(), *buf));
    return promise.get_future();
}

int BufferBindingConfig::hash() const {
    int hash = 5;
    hash = 71 * hash + bufferCacheKey;
    return hash;
}

int ImageBindingConfig::hash() const {
    return textureConfig.hashCode();
}
