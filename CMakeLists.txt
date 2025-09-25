cmake_minimum_required(VERSION 3.10)
project(icp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

set(CMAKE_BUILD_TYPE Debug)

add_executable(icp src/main.cpp)

target_include_directories(icp PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(icp PRIVATE ${OpenCV_LIBS} GLEW::GLEW glfw glm::glm)