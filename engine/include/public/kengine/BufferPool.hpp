#pragma once
#include <EASTL/fixed_allocator.h>
#include <mutex>
#include "Logger.hpp"
#include <memory_resource>

enum BufferSize {
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
    BufferSize bufSize;
    void* data;
};

class BufferPool
{
private:
    std::pmr::pool_options poolOptions;
    std::pmr::unsynchronized_pool_resource pool;

    std::mutex mtx;

public:
    BufferPool()
        : poolOptions({ 1000, 512 }), pool(poolOptions) {}

    ~BufferPool() = default;

    ByteBuf lease(uint32_t bufSize) {
        ByteBuf buf{};
        buf.bufSize = BufferSize::NONE;

        if (bufSize > 512) {
            KE_LOG_ERROR("Max buf size supported is 512.");
            return buf;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (bufSize >= BufferSize::XXLARGE)
                buf.bufSize = BufferSize::XXLARGE;
            else if (bufSize >= BufferSize::XLARGE)
                buf.bufSize = BufferSize::XLARGE;
            else if (bufSize >= BufferSize::LARGE)
                buf.bufSize = BufferSize::LARGE;
            else if (bufSize >= BufferSize::MEDIUM)
                buf.bufSize = BufferSize::MEDIUM;
            else if (bufSize >= BufferSize::SMALL)
                buf.bufSize = BufferSize::SMALL;
            else if (bufSize > 0)
                buf.bufSize = BufferSize::TINY;

            buf.data = pool.allocate(buf.bufSize);
        }

        if (buf.data)
            memset(buf.data, 0, bufSize);

        return buf;
    }

    void release(ByteBuf buf) {
        if (!buf.data)
            return;

        {
            std::lock_guard<std::mutex> lock(mtx);
            pool.deallocate(buf.data, buf.bufSize);
        }
    }
};