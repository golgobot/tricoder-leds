cmake_minimum_required(VERSION 3.8)
project(LEDs)

set(CMAKE_CXX_STANDARD 14)
find_package(Threads)
add_executable(LEDs main.cpp)
target_link_libraries(LEDs pigpio pthread)