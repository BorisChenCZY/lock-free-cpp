cmake_minimum_required(VERSION 3.10)
project(treiber_stack_project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Add compiler flags for threading support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++20")

# Enable optimizations in Release mode
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -std=c++20")

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(structs)