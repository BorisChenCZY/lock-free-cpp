# Create atomic library
add_library(atomic_lib
    atomic.cpp
)

target_include_directories(atomic_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Create atomic tests
add_executable(atomic_tests
    tests/atomic_test.cpp
)

target_link_libraries(atomic_tests
    PRIVATE
    atomic_lib
    GTest::GTest
    GTest::Main
    Threads::Threads
)

target_include_directories(atomic_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add test to ctest
add_test(NAME atomic_tests COMMAND atomic_tests) 