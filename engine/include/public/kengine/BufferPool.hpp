#pragma once
#include <EASTL/fixed_allocator.h>
#include <mutex>
#include "Logger.hpp"
#include <memory_resource>

namespace ke {
    enum LeasedBufferSize {
        NONE,
        TINY = 16,
        SMALL = 32,
        MEDIUM = 64,
        LARGE = 128,
        XLARGE = 256,
        XXLARGE = 512
    };

    struct ByteBuf
    {
        LeasedBufferSize bufSize;
        void* data;
    };

    class BufferPool
    {
    private:
        std::pmr::pool_options poolOptions;

        // switch to mutlple pools for contention reasons so we only lock certain pools at a time
        std::pmr::unsynchronized_pool_resource pool;

        std::mutex mtx;

    public:
        BufferPool()
            : poolOptions({ 100, 512 }), pool(poolOptions) {}

        ~BufferPool() = default;

        ByteBuf lease(uint32_t bufSize) {
            ByteBuf buf{};
            buf.bufSize = LeasedBufferSize::NONE;

            if (bufSize > LeasedBufferSize::XXLARGE) {
                KE_LOG_ERROR("Max buf size supported is 512.");
                return buf;
            }

            if (bufSize <= LeasedBufferSize::TINY)
                buf.bufSize = LeasedBufferSize::TINY;
            if (bufSize <= LeasedBufferSize::SMALL)
                buf.bufSize = LeasedBufferSize::SMALL;
            else if (bufSize <= LeasedBufferSize::MEDIUM)
                buf.bufSize = LeasedBufferSize::MEDIUM;
            else if (bufSize <= LeasedBufferSize::LARGE)
                buf.bufSize = LeasedBufferSize::LARGE;
            else if (bufSize <= LeasedBufferSize::XLARGE)
                buf.bufSize = LeasedBufferSize::XLARGE;
            else if (bufSize <= LeasedBufferSize::XXLARGE)
                buf.bufSize = LeasedBufferSize::XXLARGE;

            {
                std::lock_guard<std::mutex> lock(mtx);
                buf.data = pool.allocate(buf.bufSize);
            }

            if (buf.data)
                memset(buf.data, 0, bufSize);

            return buf;
        }

        void release(ByteBuf* buf) {
            if (!buf)
                return;

            {
                std::lock_guard<std::mutex> lock(mtx);
                pool.deallocate(buf->data, buf->bufSize);
            }
        }
    };
} // namespace ke