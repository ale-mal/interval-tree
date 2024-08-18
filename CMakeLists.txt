cmake_minimum_required(VERSION 3.5.0)
project(IntervalTree VERSION 0.1.0 LANGUAGES CXX)

add_executable(IntervalTree)

file(GLOB_RECURSE SOURCES "source/*.cpp")
target_sources(IntervalTree PRIVATE ${SOURCES})

target_include_directories(IntervalTree PRIVATE include)

set_property(TARGET IntervalTree PROPERTY CXX_STANDARD 17)