#include <kengine/vulkan/material/Material.hpp>

MaterialBinding& Material::getBinding(int descSetIdx, int bindingIdx) {
    auto it = materialBindings.find(descSetIdx);

    if (it == materialBindings.end())
        throw std::runtime_error("Material binding not found.");

    return *(it->second[bindingIdx]);
}

void Material::addBinding(std::unique_ptr<MaterialBinding>&& binding) {
    materialBindings[binding->getDescriptorSetIndex()].push_back(binding);
}