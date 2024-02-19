add_executable(example src/Main.cpp)

#include(CheckCXXCompilerFlag)
#check_cxx_compiler_flag(-fsanitize=address HAS_ASAN)
#if (HAS_ASAN)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")
#endif()

#target_sources(example PRIVATE
#    src/Main.cpp
#)

target_link_libraries(example PRIVATE engine)