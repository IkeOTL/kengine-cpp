#include <catch2/catch_test_macros.hpp>
#include <kengine/DeferredJob.hpp>
#include <kengine/ExecutorService.hpp>
#include <future>



TEST_CASE("Nested DJM jobs", "multithread") {
    ExecutorService pool(2);
    DeferredJobManager djm;

    const auto count = 10;
    const auto incVal = 2;
    std::vector<std::future<int>> maths;
    for (auto i = 0; i < count; i++)
    {
        maths.emplace_back(pool.submit([i, incVal]() {
            return incVal;
            }));
    }

    auto sum = 0;
    for (auto& f : maths)
        sum += f.get();

    REQUIRE(sum == (count * incVal));
}