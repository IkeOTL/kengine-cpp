cmake_minimum_required(VERSION 3.25)

project(kengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
#    add_compile_options(/EHs- /MP)
    add_compile_options(/MP)
#else()
#    add_compile_options(-fno-exceptions)
endif()

if(WIN32)
    #add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

add_subdirectory(engine)
add_subdirectory(examples)