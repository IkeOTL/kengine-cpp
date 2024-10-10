#pragma once
#include <EASTL/shared_ptr.h>
#include <EASTL/fixed_allocator.h>
#include <shared_mutex>

/// Inspired by https://github.com/libgdx/gdx-ai/

enum TelegramReceiptMode {
    RETURN_RECEIPT_UNNEEDED,
    RETURN_RECEIPT_NEEDED,
    RETURN_RECEIPT_SENT
};

class Subscriber;

struct Event {
public:
    uint32_t messageType;
    eastl::weak_ptr<Event> sender;
    eastl::weak_ptr<Event> recipient;
    TelegramReceiptMode returnReceiptStatus;
    float timestamp;
    void* data;
};

class Subscriber {
public:
    bool handleMessage(eastl::shared_ptr<Event> msg);
};

constexpr uint32_t baseAlignment = 32;
constexpr uint32_t telegramBlockSize = (sizeof(Event) + baseAlignment - 1) & ~(baseAlignment - 1);

class EventPool
{
private:
    eastl::fixed_allocator allocator;
    unsigned char buf[1000 * telegramBlockSize];

public:
    EventPool()
    {
        allocator.init(buf, sizeof(buf), telegramBlockSize, telegramBlockSize);
    }

    ~EventPool() = default;

    Event* rent() {
        auto* mem = allocator.allocate(telegramBlockSize);

        if (!mem)
            throw std::bad_alloc();

        auto* evt =  new (mem) Event();

        return evt;
    }

    void release(Event* event) {
        if (!event)
            return;

        event->~Event();
        allocator.deallocate(event, sizeof(Event));
    }
};


class EventBus {
private:
    EventPool eventPool;
    std::shared_mutex mutex;

};