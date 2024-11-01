#include <catch2/catch_test_macros.hpp>

#include <kengine/Logger.hpp>
#include <kengine/DeferredJob.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/ecs/World.hpp>

#include <future>
#include <thread>
#include <chrono>
#include <format>


TEST_CASE("multithread::ExecutorService. Basic", "[multithread]") {
    using namespace ke;

    ExecutorService pool(2);

    const auto count = 1000;
    const auto incVal = 2;

    std::vector<std::future<int>> futures;
    futures.reserve(count);

    for (auto i = 0; i < count; i++)
        futures.emplace_back(pool.submit([i, incVal]() { return incVal; }));

    auto sum = 0;
    for (auto& f : futures)
        sum += f.get();

    REQUIRE(sum == (count * incVal));
}