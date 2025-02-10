# Create treiber_stack tests
add_executable(treiber_stack_tests
    tests/treiber_stack_test.cpp
)

target_link_libraries(treiber_stack_tests
    PRIVATE
    atomic_lib
    GTest::GTest
    GTest::Main
    Threads::Threads
)

target_include_directories(treiber_stack_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

# Create spsc ring buffer tests
add_executable(spsc_tests
    tests/spsc_test.cpp
)

target_link_libraries(spsc_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    Threads::Threads
)

target_include_directories(spsc_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

# Add tests to ctest
add_test(NAME treiber_stack_tests COMMAND treiber_stack_tests)
add_test(NAME spsc_tests COMMAND spsc_tests) 