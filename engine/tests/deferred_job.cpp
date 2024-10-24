#include <catch2/catch_test_macros.hpp>

#include <kengine/Logger.hpp>
#include <kengine/DeferredJob.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/ecs/World.hpp>

#include <future>
#include <thread>
#include <chrono>
#include <format>

TEST_CASE("multithread::DJM. Basic DJM test", "[multithread]") {
    using namespace ke;

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
