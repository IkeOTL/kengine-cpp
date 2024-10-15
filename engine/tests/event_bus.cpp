#include <catch2/catch_test_macros.hpp>

#include <kengine/ecs/World.hpp>
#include <kengine/EventBus.hpp>

#include <future>
#include <thread>
#include <chrono>
#include <format>

TEST_CASE("event-bus::EventBus. Basic process", "[event-bus]") {
    SceneTime time;
    EventBus evtBus(time);
    World world(WorldConfig()
        .addService<EventBus>(&evtBus));

    auto inc = 0;
    uint32_t targetNum = 8008;
    auto subscriberId00 = evtBus.registerHandler(
        [targetNum, &inc](World& w, const Event& e) {
            // pull number out of data
            auto evtNum = *static_cast<uint32_t*>(e.data);
            REQUIRE(evtNum == targetNum);
            inc += 5;
        });
    evtBus.subscribe(1337, subscriberId00);

    auto subscriberId01 = evtBus.registerHandler(
        [targetNum, &inc](World& w, const Event& e) {
            // pull number out of data
            auto evtNum = *static_cast<uint32_t*>(e.data);
            REQUIRE(evtNum == targetNum);
            inc += 10;
        });
    evtBus.subscribe(1337, subscriberId01);

    auto* evt = evtBus.createEvent(1337, 0, sizeof(uint32_t));
    memcpy(evt->data, &targetNum, sizeof(targetNum));

    REQUIRE(inc == 0);
    evtBus.enqueue(evt);
    evtBus.process();
    REQUIRE(inc == 15);

    // check if it removes the event subcription for the subscriber
    evtBus.unregisterHandler(subscriberId00);
    evtBus.enqueue(evt);
    evtBus.process();
    // with a subscription removed the value should increase less
    REQUIRE(inc == 25);
}

TEST_CASE("event-bus::EventBus. delay test", "[event-bus]") {
    SceneTime time;
    EventBus evtBus(time);
    World world(WorldConfig()
        .addService<EventBus>(&evtBus));

    auto inc = 0;
    uint32_t targetNum = 8008;
    auto subscriberId00 = evtBus.registerHandler(
        [targetNum, &inc](World& w, const Event& e) {
            // pull number out of data
            auto evtNum = *static_cast<uint32_t*>(e.data);
            REQUIRE(evtNum == targetNum);
            inc += 5;
        });
    evtBus.subscribe(1337, subscriberId00);

    auto* evt = evtBus.createEvent(1337, 0, sizeof(uint32_t));
    memcpy(evt->data, &targetNum, sizeof(targetNum));

    evtBus.publish(evt);
    REQUIRE(inc == 15);

    // check if it removes the event subcription for the subscriber
    evtBus.unregisterHandler(subscriberId00);
    evtBus.publish(evt);
    // with a subscription removed the value should increase less
    REQUIRE(inc == 25);
}

TEST_CASE("event-bus::EventBus. Basic publish", "[event-bus]") {
    SceneTime time;
    EventBus evtBus(time);
    World world(WorldConfig()
        .addService<EventBus>(&evtBus));

    auto inc = 0;
    uint32_t targetNum = 8008;
    auto subscriberId00 = evtBus.registerHandler(
        [targetNum, &inc](World& w, const Event& e) {
            // pull number out of data
            auto evtNum = *static_cast<uint32_t*>(e.data);
            REQUIRE(evtNum == targetNum);
            inc += 5;
        });
    evtBus.subscribe(1337, subscriberId00);

    auto subscriberId01 = evtBus.registerHandler(
        [targetNum, &inc](World& w, const Event& e) {
            // pull number out of data
            auto evtNum = *static_cast<uint32_t*>(e.data);
            REQUIRE(evtNum == targetNum);
            inc += 10;
        });
    evtBus.subscribe(1337, subscriberId01);

    auto* evt = evtBus.createEvent(1337, 0, sizeof(uint32_t));
    memcpy(evt->data, &targetNum, sizeof(targetNum));

    evtBus.publish(evt);
    REQUIRE(inc == 15);

    // check if it removes the event subcription for the subscriber
    evtBus.unregisterHandler(subscriberId00);
    evtBus.publish(evt);
    // with a subscription removed the value should increase less
    REQUIRE(inc == 25);
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
    auto* evt = pool.rent(9999);
    REQUIRE(evt == nullptr);
}