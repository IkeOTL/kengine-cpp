#pragma once
#include "VmaInclude.hpp"

namespace ke {
    class GpuBuffer {
    private:
        const uint32_t id;
        const VmaAllocator vmaAllocator;
        const VmaAllocation vmaAllocation;
        const bool hostCoherent;
        void* mappedBuffer = nullptr;

    public:
        const VkBuffer vkBuffer;

        GpuBuffer(
            uint32_t id,
            VmaAllocator vmaAllocator,
            VkBuffer vkBuffer,
            VmaAllocation vmaAllocation,
            bool hostCoherent);

        ~GpuBuffer();

        VkBuffer getVkBuffer() const {
            return vkBuffer;
        }

        void* data();
        void* map();
        void unmap();
        void flush(unsigned long offset, unsigned long size);

        class ScopedMap {
            GpuBuffer& buf;

        public:
            ScopedMap(GpuBuffer& buffer)
                : buf(buffer) {
                buf.map();
            }

            ~ScopedMap() {
                buf.unmap();
            }
        };
    };
} // namespace ke