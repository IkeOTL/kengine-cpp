#include <kengine/EventBus.hpp>


Event* EventBus::rentEvent(const EventOpcode opcode, const uint32_t bufSize = 0) {
    auto* evt = eventPool.rent(bufSize);

    return evt;
}

void EventBus::subscribe(const EventOpcode opcode, const SubscriberId subscriberId) {
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

void EventBus::unsubscribe(const EventOpcode opcode, const SubscriberId subscriberId) {
    std::lock_guard<std::mutex> lock(busMtx);
    auto it = subscriptions.find(opcode);

    if (it == subscriptions.end())
        return;

    auto& subscribers = it->second;

    for (auto* subIt = subscribers.begin(); subIt != subscribers.end(); ++subIt) {
        if (*subIt == subscriberId) {
            subscribers.erase(subIt);
            break;
        }
    }

    if (subscribers.empty())
        subscriptions.erase(it);
}

void EventBus::publish(Event* evt) {
    std::lock_guard<std::mutex> lock(busMtx);
    auto it = subscriptions.find(evt->opcode);

    if (it == subscriptions.end())
        return;

    auto& subs = it->second;

    for (const auto& subId : subs)
        subscribers[subId](*world, *evt);
}

void EventBus::enqueue(Event* evt) {
    std::lock_guard<std::mutex> lock(busMtx);
    queue.push(evt);
}

Event* EventPool::rent(uint32_t bufSize = 0) {
    if (bufSize > 128) {
        KE_LOG_ERROR("Max buf size supported is 128.");
        return nullptr;
    }

    void* mem = nullptr;
    { // lock main allocator
        std::lock_guard<std::mutex> lock(poolMtx);
        mem = telegramAllocator.allocate(sizeof(Event));
    }

    if (!mem)
        throw std::bad_alloc();

    memset(mem, 0, sizeof(Event));
    auto* evt = new (mem) Event();

    auto dataBufSize = TelegramDataBufSize::NONE;

    if (bufSize > 64) {
        std::lock_guard<std::mutex> lock(dataMtx);
        dataBufSize = TelegramDataBufSize::LARGE;
        evt->data = largeAllocator.allocate(128);
    }
    else if (bufSize > 32) {
        std::lock_guard<std::mutex> lock(dataMtx);
        dataBufSize = TelegramDataBufSize::MEDIUM;
        evt->data = mediumAllocator.allocate(64);
    }
    else if (bufSize > 16) {
        std::lock_guard<std::mutex> lock(dataMtx);
        dataBufSize = TelegramDataBufSize::SMALL;
        evt->data = smallAllocator.allocate(32);
    }
    else if (bufSize > 0) {
        std::lock_guard<std::mutex> lock(dataMtx);
        dataBufSize = TelegramDataBufSize::TINY;
        evt->data = tinyAllocator.allocate(16);
    }
    else {
        evt->dataBufSize = TelegramDataBufSize::NONE;
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
    case TelegramDataBufSize::TINY: {
        std::lock_guard<std::mutex> lock(dataMtx);
        tinyAllocator.deallocate(evt->data, 16);
        break;
    }
    case TelegramDataBufSize::SMALL: {
        std::lock_guard<std::mutex> lock(dataMtx);
        smallAllocator.deallocate(evt->data, 32);
        break;
    }
    case TelegramDataBufSize::MEDIUM: {
        std::lock_guard<std::mutex> lock(dataMtx);
        mediumAllocator.deallocate(evt->data, 64);
        break;
    }
    case TelegramDataBufSize::LARGE: {
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