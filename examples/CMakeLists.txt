add_executable(example src/Main.cpp)

#target_sources(example PRIVATE
#    src/Main.cpp
#)

target_link_libraries(example PRIVATE engine)