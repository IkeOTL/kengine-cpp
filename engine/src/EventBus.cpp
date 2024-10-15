#include <kengine/EventBus.hpp>
#include <kengine/BufferPool.hpp>

void EventBus::dispatch(Event* evt) {
    if (evt->recipient) {
        std::lock_guard<std::mutex> lock(busMtx);
        auto recipientIt = handlers.find(evt->recipient);

        // recipient is no longer registered in the event bus
        if (recipientIt != handlers.end())
            recipientIt->second(*world, *evt);
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

            subsIt->second(*world, *evt);
            ++evtSubIdIt;
        }
    }

    if (evt->returnReceiptStatus == RETURN_RECEIPT_NEEDED) {
        evt->recipient = evt->sender;
        evt->sender = 0;
        evt->returnReceiptStatus = RETURN_RECEIPT_SENT;
        dispatch(evt);
        return;
    }

    // Release the telegram to the pool
    eventPool.release(evt);
}

void EventBus::unregisterHandler(HandlerId subId) {
    std::lock_guard<std::mutex> lock(busMtx);
    auto it = handlers.find(subId);
    if (it != handlers.end())
        handlers.erase(it);
}

void EventBus::subscribe(const EventOpcode opcode, const HandlerId subscriberId) {
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

void EventBus::unsubscribe(const EventOpcode opcode, const HandlerId subscriberId) {
    std::lock_guard<std::mutex> lock(busMtx);
    auto it = subscriptions.find(opcode);

    if (it == subscriptions.end())
        return;

    auto& evtSubs = it->second;

    auto subIt = std::find_if(evtSubs.begin(), evtSubs.end(),
        [subscriberId](HandlerId id) {
            return id == subscriberId;
        });

    if (subIt != evtSubs.end())
        evtSubs.erase(subIt);

    if (evtSubs.empty())
        subscriptions.erase(it);
}

void EventBus::publish(const EventOpcode opcode, const HandlerId recipientId, const HandlerId onFulfilledId, float delaySeconds, ByteBuf data) {
    assert(delaySeconds >= 0);

    auto* evt = eventPool.rent();
    evt->opcode = opcode;
    evt->recipient = recipientId;
    evt->onFulfilled = onFulfilledId;
    evt->timestamp = sceneTime.getSceneTime() + delaySeconds;
   
    evt->data = data;

    if (delaySeconds > 0) {
        std::lock_guard<std::mutex> lock(busMtx);
        queue.push(evt);
        return;
    }
}

void EventBus::process() {
    if (queue.empty())
        return;

    auto curTime = sceneTime.getSceneTime();

    while (queue.size()) {
        auto* evt = queue.top();

        if (evt->timestamp > curTime)
            break;

        publish(evt);

        queue.pop();
    }
}


/// EventPool
Event* EventPool::rent() {

    void* mem = nullptr;
    { // lock main allocator
        std::lock_guard<std::mutex> lock(poolMtx);
        mem = telegramAllocator.allocate(sizeof(Event));
    }

    if (!mem)
        throw std::bad_alloc();

    memset(mem, 0, sizeof(Event));
    auto* evt = new (mem) Event();

    auto dataBufSize = EventDataBufSize::NONE;

    if (bufSize > 64) {
        std::lock_guard<std::mutex> lock(dataMtx);
        dataBufSize = EventDataBufSize::LARGE;
        evt->data = largeAllocator.allocate(128);
    }
    else if (bufSize > 32) {
        std::lock_guard<std::mutex> lock(dataMtx);
        dataBufSize = EventDataBufSize::MEDIUM;
        evt->data = mediumAllocator.allocate(64);
    }
    else if (bufSize > 16) {
        std::lock_guard<std::mutex> lock(dataMtx);
        dataBufSize = EventDataBufSize::SMALL;
        evt->data = smallAllocator.allocate(32);
    }
    else if (bufSize > 0) {
        std::lock_guard<std::mutex> lock(dataMtx);
        dataBufSize = EventDataBufSize::TINY;
        evt->data = tinyAllocator.allocate(16);
    }
    else {
        evt->dataBufSize = EventDataBufSize::NONE;
        evt->data = nullptr;
    }


    if (evt->data)
        memset(evt->data, 0, bufSize);

    evt->dataBufSize = dataBufSize;

    return evt;
}

void EventPool::release(Event* evt) {
    if (!evt)
        return;

    evt->~Event();

    // deallocate for data
    switch (evt->dataBufSize) {
    case EventDataBufSize::TINY: {
        std::lock_guard<std::mutex> lock(dataMtx);
        tinyAllocator.deallocate(evt->data, 16);
        break;
    }
    case EventDataBufSize::SMALL: {
        std::lock_guard<std::mutex> lock(dataMtx);
        smallAllocator.deallocate(evt->data, 32);
        break;
    }
    case EventDataBufSize::MEDIUM: {
        std::lock_guard<std::mutex> lock(dataMtx);
        mediumAllocator.deallocate(evt->data, 64);
        break;
    }
    case EventDataBufSize::LARGE: {
        std::lock_guard<std::mutex> lock(dataMtx);
        largeAllocator.deallocate(evt->data, 128);
        break;
    }
    }

    { // lock main allocator
        std::lock_guard<std::mutex> lock(poolMtx);
        telegramAllocator.deallocate(evt, sizeof(Event));
    }
}