#include <catch2/catch_test_macros.hpp>

#include <kengine/EventBus.hpp>

#include <future>
#include <thread>
#include <chrono>
#include <format>

TEST_CASE("event-bus::EventBus. Add/remove subscriber", "[event-bus]") {
    class FakeSub : public Subscriber {
        bool handleMessage(Event* msg) override {
            int i = 0;
            return false;
        }
    };

    EventBus evtBus;
    auto sub0 = eastl::make_shared<FakeSub>();
    evtBus.addSubscriber(1337, eastl::weak_ptr<Subscriber>(sub0));
    evtBus.removeSubscriber(1337, sub0);
}

TEST_CASE("event-bus::EventBus. pool basic", "[event-bus]") {
    EventPool pool;

    auto* r0 = pool.rent();
    r0->timestamp = 1337;

    auto* r1 = pool.rent();
    r1->timestamp = 1337;

    // simple test on data block
    auto* r2 = pool.rent(sizeof(uint32_t));
    auto r = *static_cast<uint32_t*>(r2->data);
    // test is zeroed
    REQUIRE(r == 0);

    uint32_t lol = 1337;
    memcpy(r2->data, &lol, sizeof(uint32_t));
    r = *static_cast<uint32_t*>(r2->data);
    REQUIRE(r == lol);

    pool.release(r0);
    pool.release(r1);
    pool.release(r2);
}

TEST_CASE("event-bus::EventBus. Max data buf size", "[event-bus]") {
    EventPool pool;
    auto* evt = pool.rent(256);
    REQUIRE(evt == nullptr);
}