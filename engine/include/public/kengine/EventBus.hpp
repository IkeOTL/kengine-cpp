#pragma once
#include <EASTL/shared_ptr.h>
#include <EASTL/fixed_allocator.h>
#include <EASTL/priority_queue.h>
#include <shared_mutex>
#include "Logger.hpp"
#include <EASTL/fixed_hash_map.h>
#include "ecs/World.hpp"
#include "ecs/BaseSystem.hpp"
#include <thirdparty/entt.hpp>

/// Inspired by https://github.com/libgdx/gdx-ai/

using EventOpcode = uint32_t;
using SubscriberId = uint32_t;

enum TelegramReceiptMode {
    RETURN_RECEIPT_UNNEEDED, RETURN_RECEIPT_NEEDED, RETURN_RECEIPT_SENT
};

enum TelegramDataBufSize {
    NONE, TINY, SMALL, MEDIUM, LARGE
};

struct Event {
public:
    SubscriberId sender;
    SubscriberId recipient;
    EventOpcode opcode = 0;
    float timestamp = 0;
    TelegramReceiptMode returnReceiptStatus = TelegramReceiptMode::RETURN_RECEIPT_UNNEEDED;
    TelegramDataBufSize dataBufSize = TelegramDataBufSize::NONE;
    void* data = nullptr;

    bool operator==(const Event& other) const {
        if (this->sender != other.sender)
            return false;
        if (this->recipient != other.recipient)
            return false;
        if (opcode != other.opcode)
            return false;
        return std::bit_cast<uint32_t>(timestamp) == std::bit_cast<uint32_t>(other.timestamp);
    }
};

class EventPool
{
private:
    unsigned char telegramBuf[1000 * sizeof(Event)];
    eastl::fixed_allocator_with_overflow telegramAllocator;

    unsigned char tinyDatabuf[500 * 16];
    eastl::fixed_allocator tinyAllocator;

    unsigned char smallDatabuf[500 * 32];
    eastl::fixed_allocator smallAllocator;

    unsigned char mediumDataBuf[250 * 64];
    eastl::fixed_allocator mediumAllocator;

    unsigned char largeDataBuf[150 * 128];
    eastl::fixed_allocator largeAllocator;

    std::mutex poolMtx;
    std::mutex dataMtx;

public:
    EventPool()
    {
        telegramAllocator.init(telegramBuf, sizeof(telegramBuf), sizeof(Event), alignof(Event));
        tinyAllocator.init(tinyDatabuf, sizeof(tinyDatabuf), 16, 8);
        smallAllocator.init(smallDatabuf, sizeof(smallDatabuf), 32, 8);
        mediumAllocator.init(mediumDataBuf, sizeof(mediumDataBuf), 64, 8);
        largeAllocator.init(largeDataBuf, sizeof(largeDataBuf), 128, 8);
    }

    ~EventPool() = default;

    Event* rent(uint32_t bufSize);
    void release(Event* evt);
};

class EventBus : public  WorldService {
private:
    struct EventComparator {
        bool operator()(const Event* a, const Event* b) const {
            if (*a == *b)
                return false;
            return a->timestamp > b->timestamp;
        }
    };

    //using EventSubscription = entt::delegate<void(const void*, World&, const Event&)>;
    using EventSubscription = std::function<void(World& world, const Event&)>;

    std::atomic<uint32_t> runningId = 0;
    eastl::priority_queue<Event*, eastl::vector<Event*>, EventComparator> queue;
    eastl::hash_map<SubscriberId, EventSubscription> subscribers;
    eastl::hash_map<EventOpcode, eastl::vector<SubscriberId>> subscriptions;
    EventPool eventPool;
    std::mutex busMtx;

public:
    template <typename Callable>
    SubscriberId registerSubscriber(Callable&& func) {
        std::lock_guard<std::mutex> lock(busMtx);

        static_assert(std::is_invocable_r_v<void, Callable, World&, const Event&>,
            "Subscriber must be callable with (World&, const Event&)");

        auto id = runningId++;

        //subscribers[id].connect(std::forward<Callable>(func));
        subscribers[id] = std::move(func);
        return id;
    }

    void unregisterSubscriber(SubscriberId subId) {
        std::lock_guard<std::mutex> lock(busMtx);
        // tink about mutex usage in this class
    }

    Event* rentEvent(const EventOpcode opcode, const uint32_t bufSize);
    void subscribe(const EventOpcode opcode, const SubscriberId subscriberId);
    void unsubscribe(const EventOpcode opcode, const SubscriberId subscriberId);
    void publish(Event* evt);
    void enqueue(Event* evt);
};