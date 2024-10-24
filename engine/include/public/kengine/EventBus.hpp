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

namespace ke {
    /// Inspired by https://github.com/libgdx/gdx-ai/

    using EventOpcode = uint32_t;
    using EventHandlerId = uint32_t;

    enum EventReceiptMode {
        RETURN_RECEIPT_UNNEEDED, RETURN_RECEIPT_NEEDED, RETURN_RECEIPT_SENT
    };

    struct Event {
    public:
        EventOpcode opcode = 0;
        EventHandlerId recipient;
        EventHandlerId onFulfilled;
        float timestamp = 0;
        ByteBuf dataBuf;

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
        std::atomic<EventHandlerId> subcriberRunningId = 1;
        eastl::priority_queue<Event*, eastl::vector<Event*>, EventComparator> queue;
        eastl::hash_map<EventHandlerId, EventHandler> handlers;
        eastl::hash_map<EventOpcode, eastl::vector<EventHandlerId>> subscriptions;

        //  used std::pmr?
        std::byte eventBuf[1000 * sizeof(Event)]{};
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
            : sceneTime(st) {
            eventPool.init(eventBuf, sizeof(eventBuf), sizeof(Event), alignof(Event));
        }

        /// <summary>
        /// helper func for creating the event bus in a smart ptr
        /// </summary>
        inline static std::unique_ptr<EventBus> create(SceneTime& st) {
            return std::make_unique<EventBus>(st);
        }

        EventHandlerId registerHandler(EventHandler&& func);
        void unregisterHandler(EventHandlerId subId);
        void subscribe(const EventOpcode opcode, const EventHandlerId handlerId);
        void unsubscribe(const EventOpcode opcode, const EventHandlerId handlerId);
        void publish(const EventOpcode opcode, const EventHandlerId recipientId, const EventHandlerId onFulfilledId, float delaySeconds, ByteBuf* dataBuf = nullptr);
        void publish(const EventOpcode opcode, const EventHandlerId recipientId, const EventHandlerId onFulfilledId, ByteBuf* dataBuf = nullptr);
        void publish(const EventOpcode opcode, const EventHandlerId recipientId, float delaySeconds, ByteBuf* dataBuf = nullptr);
        void publish(const EventOpcode opcode, const EventHandlerId recipientId, ByteBuf* dataBuf = nullptr);
        void publish(const EventOpcode opcode, float delaySeconds, ByteBuf* dataBuf = nullptr);
        void publish(const EventOpcode opcode, ByteBuf* dataBuf = nullptr);
        void process();
    };
} // namespace ke