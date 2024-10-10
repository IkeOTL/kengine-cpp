enable_testing()

include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(catch2)
set_target_properties(Catch2 PROPERTIES FOLDER "thirdparty/Catch2")
set_target_properties(Catch2WithMain PROPERTIES FOLDER "thirdparty/Catch2")

set(TEST_SOURCES
    deferred_job.cpp
    executor_service.cpp
    event_bus.cpp
)

add_executable(engine_tests ${TEST_SOURCES})
set_target_properties(engine_tests PROPERTIES CXX_STANDARD 20)
set_target_properties(engine_tests PROPERTIES FOLDER "tests")

target_link_libraries(engine_tests PRIVATE
    Catch2::Catch2WithMain
    kengine_engine
)

if(NOT MSVC)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
endif()

include(Catch)
catch_discover_tests(engine_tests)