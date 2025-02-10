#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "treiber_stack.h"

class TreiberStackTest : public ::testing::Test {
protected:
    Stack<int> stack;
};

TEST_F(TreiberStackTest, EmptyStackTest) {
    EXPECT_TRUE(stack.empty());
}

TEST_F(TreiberStackTest, PushPopSingleThreadTest) {
    stack.push(1);
    EXPECT_FALSE(stack.empty());
    EXPECT_EQ(stack.pop(), 1);
    EXPECT_TRUE(stack.empty());
}

TEST_F(TreiberStackTest, MultiplePushPopTest) {
    std::vector<int> values = {1, 2, 3, 4, 5};
    for (int val : values) {
        stack.push(val);
    }
    
    // Stack is LIFO, so we should get values in reverse order
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
        EXPECT_EQ(stack.pop(), *it);
    }
    EXPECT_TRUE(stack.empty());
}

TEST_F(TreiberStackTest, ConcurrentPushTest) {
    const int num_threads = 4;
    const int pushes_per_thread = 1000;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < pushes_per_thread; ++j) {
                stack.push(i * pushes_per_thread + j);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify we have all elements
    std::vector<bool> found(num_threads * pushes_per_thread, false);
    while (!stack.empty()) {
        int val = stack.pop();
        ASSERT_GE(val, 0);
        ASSERT_LT(val, num_threads * pushes_per_thread);
        EXPECT_FALSE(found[val]) << "Duplicate value found: " << val;
        found[val] = true;
    }
    
    // Verify all values were found
    for (bool was_found : found) {
        EXPECT_TRUE(was_found);
    }
}

TEST_F(TreiberStackTest, ConcurrentPushPopTest) {
    const int num_threads = 4;
    const int ops_per_thread = 1000;
    std::atomic<int> sum_pushed(0);
    std::atomic<int> sum_popped(0);
    std::vector<std::thread> threads;
    
    // Half threads push, half pop
    for (int i = 0; i < num_threads; ++i) {
        if (i % 2 == 0) {
            threads.emplace_back([&]() {
                for (int j = 0; j < ops_per_thread; ++j) {
                    int val = j + 1;
                    stack.push(val);
                    sum_pushed.fetch_add(val);
                }
            });
        } else {
            threads.emplace_back([&]() {
                for (int j = 0; j < ops_per_thread; ++j) {
                    while (stack.empty()); // Wait for items
                    sum_popped.fetch_add(stack.pop());
                }
            });
        }
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Pop remaining elements
    while (!stack.empty()) {
        sum_popped.fetch_add(stack.pop());
    }
    
    // Verify total sum pushed equals total sum popped
    EXPECT_EQ(sum_pushed.load(), sum_popped.load());
} 