#include <catch2/catch_test_macros.hpp>

#include <kengine/Logger.hpp>
#include <kengine/DeferredJob.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/ecs/World.hpp>

#include <future>
#include <thread>
#include <chrono>
#include <format>


TEST_CASE("Basic ExecutorService test", "multithread") {
    ExecutorService pool(2);
    DeferredJobManager djm;

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

TEST_CASE("Basic DJM test", "multithread") {
    ExecutorService pool(2);
    DeferredJobManager djm;
    World world(WorldConfig()
        .addService<ExecutorService>(&pool)
        .addService<DeferredJobManager>(&djm)
    );

    // submit work to thread pool. use the future to scehdule something 
    // to happen when djm.process is executed

    auto magic = 1337;
    auto future = pool.submitShared([magic]() { return magic; });

    djm.submit(
        world,
        future,
        [magic](World&, int i) { REQUIRE(magic == i); }
    );

    // wait until future is done so we dont miss it on djm.process
    future.wait();

    // pretend this is the start of a frame
    djm.process(world);
}