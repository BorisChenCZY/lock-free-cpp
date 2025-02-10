#include <gtest/gtest.h>
#include "spsc.h"
#include <thread>
#include <vector>
#include <chrono>

TEST(RingBufferTest, BasicOperations) {
    RingBuffer<int, 4> buffer; // Size 4 means capacity of 3 due to reserved slot
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(buffer.capacity(), 3);

    // Test single push/pop
    buffer.push(42);
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer.pop(), 42);
    EXPECT_TRUE(buffer.empty());
}

TEST(RingBufferTest, FullBufferOperations) {
    RingBuffer<int, 4> buffer;
    
    // Fill the buffer
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    
    EXPECT_TRUE(buffer.full());
    EXPECT_EQ(buffer.size(), 3);
    
    // Verify values
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_TRUE(buffer.empty());
}

TEST(RingBufferTest, TryPushPopOperations) {
    RingBuffer<int, 3> buffer; // Capacity of 2
    
    EXPECT_TRUE(buffer.try_push(1));
    EXPECT_TRUE(buffer.try_push(2));
    EXPECT_FALSE(buffer.try_push(3)); // Should fail as buffer is full
    
    int val;
    EXPECT_TRUE(buffer.try_pop(val));
    EXPECT_EQ(val, 1);
    EXPECT_TRUE(buffer.try_pop(val));
    EXPECT_EQ(val, 2);
    EXPECT_FALSE(buffer.try_pop(val)); // Should fail as buffer is empty
}

TEST(RingBufferTest, WrapAroundBehavior) {
    RingBuffer<int, 4> buffer;
    
    // Fill and empty multiple times to test wrap-around
    for (int cycle = 0; cycle < 3; ++cycle) {
        buffer.push(1);
        buffer.push(2);
        buffer.push(3);
        
        EXPECT_EQ(buffer.pop(), 1);
        EXPECT_EQ(buffer.pop(), 2);
        EXPECT_EQ(buffer.pop(), 3);
    }
}

TEST(RingBufferTest, ThreadSafety) {
    RingBuffer<int, 1024> buffer;
    const int num_operations = 10000;
    
    std::thread producer([&]() {
        for (int i = 0; i < num_operations; ++i) {
            buffer.push(i);
        }
    });
    
    std::thread consumer([&]() {
        for (int i = 0; i < num_operations; ++i) {
            int val = buffer.pop();
            EXPECT_EQ(val, i);
        }
    });
    
    producer.join();
    consumer.join();
}

TEST(RingBufferTest, NonTrivialType) {
    RingBuffer<std::string, 4> buffer;
    
    buffer.push("Hello");
    buffer.push("World");
    
    EXPECT_EQ(buffer.pop(), "Hello");
    EXPECT_EQ(buffer.pop(), "World");
}

TEST(RingBufferTest, StressTest) {
    RingBuffer<int, 8> buffer;
    bool producer_done = false;
    
    std::thread producer([&]() {
        for (int i = 0; i < 1000; ++i) {
            while (!buffer.try_push(i)) {
                std::this_thread::yield();
            }
        }
        producer_done = true;
    });
    
    std::thread consumer([&]() {
        int expected = 0;
        int val;
        while (!producer_done || !buffer.empty()) {
            if (buffer.try_pop(val)) {
                EXPECT_EQ(val, expected++);
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    consumer.join();
} 