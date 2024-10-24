#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/vulkan/mesh/ModelConfig.hpp>

namespace ke {
    std::unique_ptr<Model> AsyncModelCache::create(std::shared_ptr<ModelConfig> keyObj) {
        return factory.loadModel(*keyObj);
    }
} // namespace ke