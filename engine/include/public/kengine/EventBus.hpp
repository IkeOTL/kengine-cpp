#pragma once
#include <EASTL/shared_ptr.h>
#include <EASTL/fixed_allocator.h>
#include <EASTL/priority_queue.h>
#include <shared_mutex>
#include "Logger.hpp"
#include <EASTL/fixed_hash_map.h>

/// Inspired by https://github.com/libgdx/gdx-ai/

enum TelegramReceiptMode {
    RETURN_RECEIPT_UNNEEDED, RETURN_RECEIPT_NEEDED, RETURN_RECEIPT_SENT
};

enum TelegramDataBufSize {
    NONE, TINY, SMALL, MEDIUM, LARGE
};

class Subscriber;

struct Event {
public:
    using EventOpcode = uint32_t;

    eastl::weak_ptr<Subscriber> sender;
    eastl::weak_ptr<Subscriber> recipient;
    EventOpcode opcode = 0;
    float timestamp = 0;
    TelegramReceiptMode returnReceiptStatus = TelegramReceiptMode::RETURN_RECEIPT_UNNEEDED;
    TelegramDataBufSize dataBufSize = TelegramDataBufSize::NONE;
    void* data = nullptr;

    bool operator==(const Event& other) const {
        if (this->sender.lock() != other.sender.lock())
            return false;
        if (this->recipient.lock() != other.recipient.lock())
            return false;
        if (opcode != other.opcode)
            return false;
        return std::bit_cast<uint32_t>(timestamp) == std::bit_cast<uint32_t>(other.timestamp);
    }
};

class Subscriber {
public:
    virtual bool handleMessage(Event* msg) = 0;
};

class EventPool
{
private:
    unsigned char telegramBuf[1000 * sizeof(Event)];
    eastl::fixed_allocator_with_overflow telegramAllocator;

    unsigned char tinyDatabuf[1000 * 16];
    eastl::fixed_allocator tinyAllocator;

    unsigned char smallDatabuf[500 * 32];
    eastl::fixed_allocator smallAllocator;

    unsigned char mediumDataBuf[250 * 64];
    eastl::fixed_allocator mediumAllocator;

    unsigned char largeDataBuf[150 * 128];
    eastl::fixed_allocator largeAllocator;

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

    Event* rent(uint32_t bufSize = 0) {
        if (bufSize > 128) {
            KE_LOG_ERROR("Max buf size supported is 128.");
            return nullptr;
        }

        auto* mem = telegramAllocator.allocate(sizeof(Event));

        if (!mem)
            throw std::bad_alloc();

        memset(mem, 0, sizeof(Event));
        auto* evt = new (mem) Event();

        auto dataBufSize = TelegramDataBufSize::NONE;

        if (bufSize > 64) {
            dataBufSize = TelegramDataBufSize::LARGE;
            evt->data = largeAllocator.allocate(128);
        }
        else if (bufSize > 32) {
            dataBufSize = TelegramDataBufSize::MEDIUM;
            evt->data = mediumAllocator.allocate(64);
        }
        else if (bufSize > 16) {
            dataBufSize = TelegramDataBufSize::SMALL;
            evt->data = smallAllocator.allocate(32);
        }
        else if (bufSize > 0) {
            dataBufSize = TelegramDataBufSize::TINY;
            evt->data = tinyAllocator.allocate(16);
        }
        else {
            evt->dataBufSize = TelegramDataBufSize::NONE;
            evt->data = nullptr;
        }

        evt->dataBufSize = dataBufSize;


        return evt;
    }

    void release(Event* evt) {
        if (!evt)
            return;

        evt->~Event();

        // deallocate for data
        switch (evt->dataBufSize) {
        case TelegramDataBufSize::TINY:
            tinyAllocator.deallocate(evt->data, 16);
            break;
        case TelegramDataBufSize::SMALL:
            smallAllocator.deallocate(evt->data, 32);
            break;
        case TelegramDataBufSize::MEDIUM:
            mediumAllocator.deallocate(evt->data, 64);
            break;
        case TelegramDataBufSize::LARGE:
            largeAllocator.deallocate(evt->data, 128);
            break;
        }

        telegramAllocator.deallocate(evt, sizeof(Event));
    }
};


class EventBus {
private:
    struct EventComparator {
        bool operator()(const Event* a, const Event* b) const {
            if (*a == *b)
                return false;
            return a->timestamp > b->timestamp;
        }
    };

    eastl::priority_queue<Event*, eastl::vector<Event*>, EventComparator> queue;
    eastl::hash_map<Event::EventOpcode, eastl::vector<eastl::weak_ptr<Subscriber>>> evtListeners;
    EventPool eventPool;
    std::shared_mutex mutex;

public:
    void addSubscriber(Event::EventOpcode opcode, const eastl::weak_ptr<Subscriber>& subscriber) {
        evtListeners[opcode].push_back(subscriber);

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

    void removeSubscriber(Event::EventOpcode opcode, const eastl::shared_ptr<Subscriber>& subscriber) {
        auto it = evtListeners.find(opcode);

        if (it == evtListeners.end())
            return;

        auto& subscribers = it->second;

        // clean up expired weak_ptrs in one pass
        subscribers.erase(
            eastl::remove_if(subscribers.begin(), subscribers.end(),
                [](const eastl::weak_ptr<Subscriber>& weakSub) {
                    return weakSub.expired();
                }),
            subscribers.end());

        for (auto subIt = subscribers.begin(); subIt != subscribers.end(); ++subIt) {
            if (auto sub = subIt->lock()) {
                if (sub == subscriber) {
                    subscribers.erase(subIt);
                    break;
                }
            }
        }

        if (subscribers.empty())
            evtListeners.erase(it);
    }
};