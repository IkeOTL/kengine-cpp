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
#include "Game.hpp"
#include "BufferPool.hpp"

/// Inspired by https://github.com/libgdx/gdx-ai/

using EventOpcode = uint32_t;
using HandlerId = uint32_t;

enum EventReceiptMode {
    RETURN_RECEIPT_UNNEEDED, RETURN_RECEIPT_NEEDED, RETURN_RECEIPT_SENT
};

struct Event {
public:
    EventOpcode opcode = 0;
    HandlerId recipient;
    HandlerId onFulfilled;
    float timestamp = 0;
    ByteBuf data;

    bool operator==(const Event& other) const {
        if (this->recipient != other.recipient)
            return false;
        if (this->onFulfilled != other.onFulfilled)
            return false;
        if (opcode != other.opcode)
            return false;
        return std::bit_cast<uint32_t>(timestamp) == std::bit_cast<uint32_t>(other.timestamp);
    }
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
    using EventHandler = std::function<void(const Event&, World&)>;

    /// <summary>
    /// SubscriberId = 0 is reserved for reference no/null/void subscriber
    /// </summary>
    std::atomic<HandlerId> subcriberRunningId = 1;
    eastl::priority_queue<Event*, eastl::vector<Event*>, EventComparator> queue;
    eastl::hash_map<HandlerId, EventHandler> handlers;
    eastl::hash_map<EventOpcode, eastl::vector<HandlerId>> subscriptions;
    
    //  used std::pmr?
    std::byte telegramBuf[1000 * sizeof(Event)];
    eastl::fixed_allocator_with_overflow eventPool;

    std::mutex poolMtx;
    std::mutex busMtx;

    // to avoud calls to world for the registered time service
    SceneTime& sceneTime;

    Event* leaseEvent();
    void releaseEvent(Event* evt);

    void dispatch(Event* evt);

public:
    EventBus(SceneTime& st)
        : sceneTime(st) {}

    template <typename Callable>
    HandlerId registerHandler(Callable&& func) {
        std::lock_guard<std::mutex> lock(busMtx);

        static_assert(std::is_invocable_r_v<void, Callable, const Event&, World&>,
            "Subscriber must be callable with (const HandlerId, const Event&, World&)");

        auto id = subcriberRunningId++;

        //subscribers[id].connect(std::forward<Callable>(func));
        handlers[id] = std::move(func);
        return id;
    }

    void unregisterHandler(HandlerId subId);

    void subscribe(const EventOpcode opcode, const HandlerId handlerId);
    void unsubscribe(const EventOpcode opcode, const HandlerId handlerId);
    void publish(const EventOpcode opcode, const HandlerId recipientId, const HandlerId onFulfilledId, float delaySeconds, ByteBuf* data);
    void process();
};