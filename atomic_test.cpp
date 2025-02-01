#include "atomic.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

class AtomicUInt128Test : public ::testing::Test {
protected:
    static const size_t NUM_THREADS = 4;
    static const size_t ITERATIONS = 1000;
};

TEST_F(AtomicUInt128Test, BasicLoadStore) {
    std::atomic<uint128_t> atomic_val;
    uint128_t initial_val = {42, 24};
    
    atomic_val.store(initial_val);
    uint128_t loaded_val = atomic_val.load();
    
    EXPECT_EQ(loaded_val.lower, initial_val.lower);
    EXPECT_EQ(loaded_val.upper, initial_val.upper);
}

TEST_F(AtomicUInt128Test, CompareExchangeSuccess) {
    std::atomic<uint128_t> atomic_val;
    uint128_t initial_val = {1, 1};
    uint128_t new_val = {2, 2};
    
    atomic_val.store(initial_val);
    uint128_t expected = initial_val;
    
    bool success = atomic_val.compare_exchange_strong(expected, new_val);
    
    EXPECT_TRUE(success);
    uint128_t final_val = atomic_val.load();
    EXPECT_EQ(final_val.lower, new_val.lower);
    EXPECT_EQ(final_val.upper, new_val.upper);
}

TEST_F(AtomicUInt128Test, CompareExchangeFailure) {
    std::atomic<uint128_t> atomic_val;
    uint128_t initial_val = {1, 1};
    uint128_t wrong_expected = {3, 3};
    uint128_t new_val = {2, 2};
    
    atomic_val.store(initial_val);
    
    bool success = atomic_val.compare_exchange_strong(wrong_expected, new_val);
    
    EXPECT_FALSE(success);
    uint128_t final_val = atomic_val.load();
    EXPECT_EQ(final_val.lower, initial_val.lower);
    EXPECT_EQ(final_val.upper, initial_val.upper);
    EXPECT_EQ(wrong_expected.lower, initial_val.lower);
    EXPECT_EQ(wrong_expected.upper, initial_val.upper);
}

TEST_F(AtomicUInt128Test, ConcurrentIncrements) {
    std::atomic<uint128_t> atomic_val;
    uint128_t initial_val = {0, 0};
    atomic_val.store(initial_val);
    
    auto increment_func = [&]() {
        for (size_t i = 0; i < ITERATIONS; ++i) {
            uint128_t expected = atomic_val.load();
            uint128_t desired;
            do {
                desired = {expected.lower + 1, expected.upper};
            } while (!atomic_val.compare_exchange_strong(expected, desired));
        }
    };
    
    std::vector<std::thread> threads;
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(increment_func);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    uint128_t final_val = atomic_val.load();
    EXPECT_EQ(final_val.lower, NUM_THREADS * ITERATIONS);
    EXPECT_EQ(final_val.upper, 0);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 