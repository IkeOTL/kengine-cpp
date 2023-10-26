#pragma once
#include <vulkan/vulkan_core.h>
#include <type_traits>
#include <memory>

class RenderTarget;

template <typename RP>
class RenderPass {
    static_assert(std::is_base_of<RenderTarget, RP>::value, "Generic must derive from RenderTarget");

private:
    virtual std::unique_ptr<RenderTarget> createRenderTarget() = 0;

public:

};