#include <catch2/catch_test_macros.hpp>

#include <kengine/ecs/World.hpp>
#include <kengine/EventBus.hpp>

#include <future>
#include <thread>
#include <chrono>
#include <format>

TEST_CASE("event-bus::EventBus. Basic process", "[event-bus]") {
    using namespace ke;

    SceneTime time;
    BufferPool bufPool;
    EventBus evtBus(time);
    World world(WorldConfig()
        .addService<EventBus>(&evtBus));

    auto inc = 0;
    uint32_t targetNum = 8008;
    auto handlerId00 = evtBus.registerHandler(
        [targetNum, &inc](const Event& e, World& w) {
            inc += 5;
        });
    evtBus.subscribe(1337, handlerId00);

    auto handlerId01 = evtBus.registerHandler(
        [targetNum, &inc](const Event& e, World& w) {
            inc += 10;
        });
    evtBus.subscribe(1337, handlerId01);

    REQUIRE(inc == 0);

    // sned to a single handler
    evtBus.publish(1337, handlerId00, 0, 0, nullptr);
    evtBus.process();
    REQUIRE(inc == 5);

    // check if it removes the event subcription for the subscriber
    // send to all listening to eventid
    evtBus.publish(1337, 0, 0, 0, nullptr);
    evtBus.process();
    // with a subscription removed the value should increase less
    REQUIRE(inc == 20);

    // unregister one and publsuih to all
    evtBus.unregisterHandler(handlerId00);
    evtBus.publish(1337, 0, 0, 0, nullptr);
    evtBus.process();
    REQUIRE(inc == 30);
}

TEST_CASE("event-bus::EventBus. Data buf contents", "[event-bus]") {
    using namespace ke;

    SceneTime time;
    BufferPool bufPool;
    EventBus evtBus(time);
    World world(WorldConfig()
        .addService<EventBus>(&evtBus));

    uint32_t targetNum = 8008;
    auto handlerId00 = evtBus.registerHandler(
        [targetNum](const Event& e, World& w) {
            // pull number out of data
            auto evtNum = *static_cast<uint32_t*>(e.dataBuf.data);
            REQUIRE(evtNum == targetNum);
        });
    evtBus.subscribe(1337, handlerId00);

    auto handlerId01 = evtBus.registerHandler(
        [targetNum](const Event& e, World& w) {
            // pull number out of data
            auto evtNum = *static_cast<uint32_t*>(e.dataBuf.data);
            REQUIRE(evtNum == targetNum);
        });
    evtBus.subscribe(1337, handlerId01);

    auto dataBuf = bufPool.lease(sizeof(uint32_t));
    memcpy(dataBuf.data, &targetNum, sizeof(targetNum));

    evtBus.publish(1337, 0, 0, 0, &dataBuf);
    evtBus.process();
}

TEST_CASE("event-bus::EventBus. Delay test", "[event-bus]") {
    using namespace ke;

    SceneTime time;
    BufferPool bufPool;
    EventBus evtBus(time);
    World world(WorldConfig()
        .addService<EventBus>(&evtBus));

    auto inc = 0;

    auto handlerId00 = evtBus.registerHandler(
        [&inc](const Event& e, World& w) {
            inc += 5;
        });

    // start at 0
    REQUIRE(inc == 0);

    // target a specific handler
    evtBus.publish(1337, handlerId00, 0, 5, nullptr);
    evtBus.process();
    REQUIRE(inc == 0); // should still be 0
    time.addSceneTime(5); // increment scenetime
    evtBus.process();
    REQUIRE(inc == 5); // should now be 5

    // try a broadcast
    evtBus.subscribe(1337, handlerId00);
    evtBus.publish(1337, 0, 0, 2, nullptr);
    evtBus.process();
    REQUIRE(inc == 5); // should still be 5
    time.addSceneTime(15); // increment scenetime
    evtBus.process();
    REQUIRE(inc == 10); // should now be 10
}

TEST_CASE("event-bus::EventBus. Large test", "[event-bus]") {
    using namespace ke;

    SceneTime time;
    BufferPool bufPool;
    EventBus evtBus(time);
    World world(WorldConfig()
        .addService<EventBus>(&evtBus));

    auto inc = 0;
    auto handlerCount = 10000;

    std::vector<int> handlerIds;
    handlerIds.reserve(handlerCount);

    for (size_t i = 0; i < handlerCount; i++)
    {
        auto hId = evtBus.registerHandler(
            [&inc](const Event& e, World& w) {
                inc += 1;
            });

        evtBus.subscribe(8008, hId);
        handlerIds.push_back(hId);
    }

    // start at 0
    REQUIRE(inc == 0);

    evtBus.publish(8008, 0, 0, 0, nullptr);
    evtBus.process();

    REQUIRE(inc == handlerCount);
}