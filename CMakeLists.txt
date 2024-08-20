cmake_minimum_required(VERSION 3.5.0)
project(IntervalTree VERSION 0.1.0 LANGUAGES CXX)

file(GLOB_RECURSE SOURCES "source/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "test/source/*.cpp")
add_executable(IntervalTree ${SOURCES} ${TEST_SOURCES})

target_include_directories(IntervalTree PRIVATE include test/include)

set_property(TARGET IntervalTree PROPERTY CXX_STANDARD 17)