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

TEST_CASE("multithread::ExecutorService. Basic yielding", "[multithread]") {
    ExecutorService pool(2);

    const auto count = 1000;
    const auto testModifier = 2;
    std::atomic_int32_t lol = 0;

    auto yieldingTask = pool.submitYielding<int>(
        [&lol, count](auto& pool) {
            std::vector<std::shared_future<void>> tFut;
            tFut.reserve(count);

            for (auto i = 0; i < count; i++)
                tFut.emplace_back(pool.submitShared(
                    [&lol]() {
                        lol++;
                        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    }));

            return [&lol, tFut = std::move(tFut)](auto& promise) mutable {
                for (auto& f : tFut) {
                    if (f.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                        return false;
                }

                promise.set_value(lol * 2);
                return true;
                };
        }
    );

    auto res = yieldingTask.get();

    REQUIRE(res == count * testModifier);
}

TEST_CASE("multithread::ExecutorService. Basic yielding void return", "[multithread]") {
    ExecutorService pool(2);

    const auto count = 1000;

    auto yieldingTask = pool.submitYielding<void>(
        [count](auto& pool) {
            std::vector<std::shared_future<void>> tFut;
            tFut.reserve(count);

            for (auto i = 0; i < count; i++)
                tFut.emplace_back(pool.submitShared([]() {}));

            return [tFut = std::move(tFut)](auto& promise) mutable {
                // not even needed for this scenario since work is placed into the queue in order, 
                // this will always run after tasks before it. i guess we could check if a task failed
                for (auto& f : tFut) {
                    if (f.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                        return false;
                }

                promise.set_value();
                return true;
                };
        }
    );

    REQUIRE_NOTHROW(yieldingTask.get());
}

TEST_CASE("multithread::ExecutorService. Basic yielding exception", "[multithread]") {
    ExecutorService pool(2);

    auto yieldingTask = pool.submitYielding<void>(
        [](auto& pool) {
            auto future = pool.submitShared([]() { throw std::runtime_error("Rawr we broken..."); });

            return [future](auto& promise) mutable {
                // not even needed for this scenario since work is placed into the queue in order, 
                // this will always run after tasks before it. i guess we could check if a task failed
                if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                    return false;

                REQUIRE_THROWS(future.get());

                // make it throw again to see if the promise outside gets hit
                future.get();

                promise.set_value();
                return true;
                };
        }
    );

    REQUIRE_THROWS(yieldingTask.get());
}