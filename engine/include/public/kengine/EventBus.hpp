#pragma once
#include <EASTL/shared_ptr.h>
#include <EASTL/fixed_allocator.h>
#include <shared_mutex>

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
    eastl::weak_ptr<Subscriber> sender;
    eastl::weak_ptr<Subscriber> recipient;
    uint32_t messageType;
    TelegramReceiptMode returnReceiptStatus;
    float timestamp;
    TelegramDataBufSize dataBufSize;
    void* data;
};

class Subscriber {
public:
    bool handleMessage(Event* msg);
};

class EventPool
{
private:
    unsigned char telegramBuf[1000 * sizeof(Event)];
    eastl::fixed_allocator telegramAllocator;

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
    EventPool eventPool;
    std::shared_mutex mutex;

};