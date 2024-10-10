#include <catch2/catch_test_macros.hpp>

#include <kengine/EventBus.hpp>

#include <future>
#include <thread>
#include <chrono>
#include <format>


TEST_CASE("event-bus::EventBus. Basic", "[event-bus]") {
    EventPool pool;
    {
        auto* r0 = pool.rent();
        r0->timestamp = 1337;

        auto* r1 = pool.rent();
        r1->timestamp = 1337;

        auto* r2 = pool.rent();
        r2->timestamp = 1337;

        pool.release(r0);
        pool.release(r1);
        pool.release(r2);
    }
    int i = 0;

}