cmake_minimum_required(VERSION 3.10)
project(icp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(icp src/main.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(icp PRIVATE fmt::fmt)