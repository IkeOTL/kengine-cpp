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
)

foreach(test_file ${TEST_SOURCES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file})
    set_target_properties(${test_name} PROPERTIES CXX_STANDARD 20)
    set_target_properties(${test_name} PROPERTIES FOLDER "tests/engine")
    target_link_libraries(${test_name} PRIVATE Catch2::Catch2WithMain kengine_engine)
    
    # (Optional) Add labels or properties if needed
    # set_tests_properties(${test_name} PROPERTIES LABELS "unit;math")
endforeach()