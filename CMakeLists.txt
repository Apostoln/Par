cmake_minimum_required(VERSION 3.7)
project(Parallelism)

#set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp Parallel.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")
add_executable(Parallelism ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} pthread)
