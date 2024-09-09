#include <kengine/vulkan/mesh/ModelConfig.hpp>

size_t ModelConfig::hashCode() const noexcept {
    std::hash<std::string> strHasher;

    size_t hash = 7;
    hash = 89 * hash + strHasher(modelKey);
    hash = 89 * hash + attributes;
    return hash;
}

bool ModelConfig::operator==(const ModelConfig& other) const {
    if (this == &other)
        return true;

    if (this->modelKey != other.modelKey)
        return false;

    if (this->attributes != other.attributes)
        return false;

    return this->hashCode() == other.hashCode();
}

bool ModelConfig::operator!=(const ModelConfig& other) const {
    return !(*this == other);
}