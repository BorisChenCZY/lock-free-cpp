#include "atomic.hpp"
#include <iostream>
#include <thread>
#include <vector>

// Check for CMPXCHG16B support at runtime
bool check_cmpxchg16b_support() {
#ifdef NDEBUG
    return true; // Skip runtime check in release builds
#else
    uint32_t eax, ebx, ecx, edx;
    __asm__ __volatile__(
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    return (ecx & (1 << 13)) != 0;  // Check if CMPXCHG16B bit is set
#endif
}

// Specialized atomic implementation for uint128_t
constexpr std::atomic<uint128_t>::atomic(uint128_t desired) noexcept : value_(desired) {}

uint128_t std::atomic<uint128_t>::load(std::memory_order order) const noexcept {
    static bool has_cmpxchg16b = check_cmpxchg16b_support();
    if (!has_cmpxchg16b) {
        std::cerr << "Warning: CMPXCHG16B not supported, atomic operations may not be reliable\n";
    }

    static_assert(alignof(uint128_t) >= 16, "uint128_t must be 16-byte aligned");
    uint128_t result;
    
    // Ensure we read the entire 128 bits atomically
    asm volatile(
        "lock; cmpxchg16b %1\n"  // Use CMPXCHG16B for atomic read
        : "=A"(result)           // Output to result through EDX:EAX
        : "m"(value_),           // Memory operand
          "c"(0), "b"(0)         // Clear RCX:RBX as we don't need them
        : "memory"
    );
    return result;
}

void std::atomic<uint128_t>::store(uint128_t desired, std::memory_order order) noexcept {
    uint128_t expected = value_;
    // Keep trying until the store succeeds
    while (!compare_exchange_strong(expected, desired)) {
        expected = value_;
    }
}

bool std::atomic<uint128_t>::compare_exchange_strong(uint128_t& expected, uint128_t desired,
                           std::memory_order success,
                           std::memory_order failure) noexcept {
    static bool has_cmpxchg16b = check_cmpxchg16b_support();
    if (!has_cmpxchg16b) {
        std::cerr << "Warning: CMPXCHG16B not supported, atomic operations may not be reliable\n";
    }
    
    bool result;
    
    asm volatile(
        "lock; cmpxchg16b %[value]\n"  // Compare and exchange 16 bytes
        "setz %[result]\n"              // Set result based on success
        : [result]"=q"(result),
          [value]"+m"(value_),
          "=a"(expected.lower),         // RAX gets current lower value on failure
          "=d"(expected.upper)          // RDX gets current upper value on failure
        : "a"(expected.lower),          // RAX holds expected lower value
          "d"(expected.upper),          // RDX holds expected upper value
          "b"(desired.lower),           // RBX holds desired lower value
          "c"(desired.upper)            // RCX holds desired upper value
        : "cc", "memory"                // Clobbers condition codes and memory
    );
    
    return result;
}