cmake_minimum_required(VERSION 3.10)
project(treiber_stack_project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Create atomic library
add_library(atomic_lib
    atomic.cpp
)

# Create atomic tests
add_executable(atomic_tests
    test/atomic_test.cpp
)

target_link_libraries(atomic_tests
    PRIVATE
    atomic_lib
    GTest::GTest
    GTest::Main
    Threads::Threads
)

target_include_directories(atomic_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Create treiber_stack executable
add_executable(treiber_stack
    treiber_stack.cpp
)

# Create treiber_stack tests
add_executable(treiber_stack_tests
    test/treiber_stack_test.cpp
)

target_link_libraries(treiber_stack_tests
    PRIVATE
    atomic_lib
    GTest::GTest
    GTest::Main
    Threads::Threads
)

target_include_directories(treiber_stack_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Create spsc ring buffer tests
add_executable(spsc_tests
    test/spsc_test.cpp
)

target_link_libraries(spsc_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    Threads::Threads
)

target_include_directories(spsc_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link atomic library to treiber_stack
target_link_libraries(treiber_stack
    PRIVATE atomic_lib
)

# Add compiler flags for threading support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Enable optimizations in Release mode
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Enable testing
enable_testing()
add_test(NAME atomic_tests COMMAND atomic_tests)
add_test(NAME treiber_stack_tests COMMAND treiber_stack_tests)
add_test(NAME spsc_tests COMMAND spsc_tests) 