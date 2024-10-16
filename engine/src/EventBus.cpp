#include <kengine/EventBus.hpp>

EventHandlerId EventBus::registerHandler(EventHandler&& func) {
    std::lock_guard<std::mutex> lock(busMtx);
    auto id = subcriberRunningId++;
    handlers[id] = std::move(func);
    return id;
}

Event* EventBus::leaseEvent() {
    void* mem = nullptr;
    {
        std::lock_guard<std::mutex> lock(poolMtx);
        mem = eventPool.allocate(sizeof(Event));
    }

    if (!mem)
        throw std::bad_alloc();

    memset(mem, 0, sizeof(Event));
    return new (mem) Event();
}

void EventBus::releaseEvent(Event* evt) {
    if (!evt)
        return;

    std::lock_guard<std::mutex> lock(poolMtx);
    eventPool.deallocate(evt, sizeof(Event));
}

void EventBus::dispatch(Event* evt) {
    if (evt->recipient) {
        std::lock_guard<std::mutex> lock(busMtx);
        auto recipientIt = handlers.find(evt->recipient);

        // recipient is no longer registered in the event bus
        if (recipientIt != handlers.end())
            recipientIt->second(*evt, *world);
        else
            // might leak data if `evt->data != nullptr`, and it never gets returned to pool or disposed
            KE_LOG_WARN(std::format("Event was dispatched to non-existent recipient. Might leak data."));
    }
    else { // target all subscribers of the opcode
        std::lock_guard<std::mutex> lock(busMtx);
        auto subscriptionsIt = subscriptions.find(evt->opcode);

        if (subscriptionsIt == subscriptions.end())
            return;

        auto& evtSubs = subscriptionsIt->second;

        // send to all subs
        for (auto* evtSubIdIt = evtSubs.begin(); evtSubIdIt != evtSubs.end(); ) {
            auto subsIt = handlers.find(*evtSubIdIt);

            // subscriber is not in the map, its been removed.
            // lets remove it from this opcode's subscriptions
            if (subsIt == handlers.end()) {
                evtSubIdIt = evtSubs.erase(evtSubIdIt);
                continue;
            }

            subsIt->second(*evt, *world);
            ++evtSubIdIt;
        }
    }

    if (evt->onFulfilled) {
        evt->recipient = evt->onFulfilled;
        evt->onFulfilled = 0;
        dispatch(evt);
        return;
    }

    // Release the telegram to the pool
    releaseEvent(evt);
}

void EventBus::unregisterHandler(EventHandlerId subId) {
    std::lock_guard<std::mutex> lock(busMtx);
    auto it = handlers.find(subId);
    if (it != handlers.end())
        handlers.erase(it);
}

void EventBus::subscribe(const EventOpcode opcode, const EventHandlerId subscriberId) {
    std::lock_guard<std::mutex> lock(busMtx);
    subscriptions[opcode].push_back(subscriberId);

    /*  if (providers != null) {
          for (int i = 0, n = providers.size; i < n; i++) {
              TelegramProvider provider = providers.get(i);
              Object info = provider.provideMessageInfo(msg, listener);
              if (info != null) {
                  Telegraph sender = ClassReflection.isInstance(Telegraph.class, provider) ? (Telegraph)provider : null;
                  dispatchMessage(0, sender, listener, msg, info, false);
              }
          }
      }*/
}

void EventBus::unsubscribe(const EventOpcode opcode, const EventHandlerId subscriberId) {
    std::lock_guard<std::mutex> lock(busMtx);
    auto it = subscriptions.find(opcode);

    if (it == subscriptions.end())
        return;

    auto& evtSubs = it->second;

    auto subIt = std::find_if(evtSubs.begin(), evtSubs.end(),
        [subscriberId](EventHandlerId id) {
            return id == subscriberId;
        });

    if (subIt != evtSubs.end())
        evtSubs.erase(subIt);

    if (evtSubs.empty())
        subscriptions.erase(it);
}

void EventBus::publish(const EventOpcode opcode, const EventHandlerId recipientId, const EventHandlerId onFulfilledId, float delaySeconds, ByteBuf* dataBuf) {
    assert(delaySeconds >= 0);

    auto* evt = leaseEvent();
    evt->opcode = opcode;
    evt->recipient = recipientId;
    evt->onFulfilled = onFulfilledId;
    evt->timestamp = sceneTime.getSceneTime();

    if (dataBuf)
        evt->dataBuf = *dataBuf;

    if (delaySeconds <= 0) {
        dispatch(evt);
        return;
    }

    evt->timestamp += delaySeconds;

    std::lock_guard<std::mutex> lock(busMtx);
    queue.push(evt);
}

void EventBus::publish(const EventOpcode opcode, const EventHandlerId recipientId, const EventHandlerId onFulfilledId, ByteBuf* dataBuf = nullptr) {
    publish(opcode, recipientId, onFulfilledId, 0, dataBuf);
}

void EventBus::publish(const EventOpcode opcode, const EventHandlerId recipientId, float delaySeconds, ByteBuf* dataBuf = nullptr) {
    publish(opcode, recipientId, 0, delaySeconds, dataBuf);
}

void EventBus::publish(const EventOpcode opcode, const EventHandlerId recipientId, ByteBuf* dataBuf = nullptr) {
    publish(opcode, recipientId, 0, 0, dataBuf);
}

void EventBus::publish(const EventOpcode opcode, float delaySeconds, ByteBuf* dataBuf = nullptr) {
    publish(opcode, 0, 0, delaySeconds, dataBuf);
}

void EventBus::publish(const EventOpcode opcode, ByteBuf* dataBuf = nullptr) {
    publish(opcode, 0, 0, 0, dataBuf);
}


void EventBus::process() {
    if (queue.empty())
        return;

    auto curTime = sceneTime.getSceneTime();

    while (queue.size()) {
        auto* evt = queue.top();

        if (evt->timestamp > curTime)
            break;

        dispatch(evt);

        queue.pop();
    }
}