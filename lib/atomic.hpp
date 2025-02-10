/*
 * This file is part of a project licensed under the GNU General Public License v3.0.
 *
 * Copyright (C) 2023 ZHENYU CHEN
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <stdexcept>

// Our 128-bit unsigned integer type
struct alignas(16) uint128_t {
    uint64_t lower;  // Least significant bits
    uint64_t upper;  // Most significant bits

    constexpr uint128_t(): lower(0), upper(0) {}
    constexpr uint128_t(uint64_t lower, uint64_t upper): lower(lower), upper(upper) {};
};

// Forward declaration of the specialized atomic implementation
namespace std {
    template<>
    class atomic<uint128_t> {
    public:
        atomic() noexcept = default;
        constexpr atomic(uint128_t desired) noexcept;
        
        atomic(const atomic&) = delete;
        atomic& operator=(const atomic&) = delete;
        atomic& operator=(const atomic&) volatile = delete;

        uint128_t load(std::memory_order order = std::memory_order_seq_cst) const noexcept;
        void store(uint128_t desired, std::memory_order order = std::memory_order_seq_cst) noexcept;
        bool compare_exchange_strong(uint128_t& expected, uint128_t desired,
                                   std::memory_order success = std::memory_order_seq_cst,
                                   std::memory_order failure = std::memory_order_seq_cst) noexcept;

    protected:
        alignas(16) uint128_t value_;
    };
}