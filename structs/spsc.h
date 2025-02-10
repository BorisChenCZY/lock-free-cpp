/*
 * This file is part of a project licensed under the GNU General Public License v3.0.
 *
 * Copyright (C) 2025 ZHENYU CHEN
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

#include <atomic>
#include <cstdint>
#include <cstddef>
#include <array>
#include <utility>

/** this is a spsc ring buffer, with a fixed size.
 * @tparam T: must be default constructable
 * we're using two atomic variable to manage the states of this ring buffer.
 * @note m_start is able to be larger then m_end (as it is ring buffer)
 * @note we'd like to reserve one slot, to differenciate if the queue is empty or full. 
 *       if the size is already N-1, we'd consider it's full.
*/
template <typename T, size_t N>
struct RingBuffer{

static_assert(N > 1, "Buffer size must be greater than 1");
static_assert(std::is_default_constructible_v<T>, 
              "T must be default constructible");

private:
    std::array<T, N> m_arr;

    std::atomic<size_t> m_end {0}; // next slot to push. Update by the writer thread.
    std::atomic<size_t> m_start {0}; // next slot to pop. Updated by the reader thread.




    // next will return the next slot according to prev slot. (so that access to m_arr is always correct)
    // @param prev: prev slot
    // @return next slot. Return value is [0, N)
    size_t next(size_t prev) {
        auto ret = prev + 1;
        if (ret >= N) [[unlikely]]
        {
            ret -= N;
        }
        return ret;
    }

public:

    RingBuffer(): m_end(0), m_start(0) {}

    // @return if the queue is empty.
    // @note this is if a queue is empty at a serilization point.
    //       doesn't necessarlily mean it is still empty when reading the result
    bool empty() {
        return m_end.load(std::memory_order::relaxed) == m_start.load(std::memory_order::relaxed);
    }

    // @return if the queue is full
    // @note this is if a queue is full at a serilization point.
    //       doesn't necessarlily mean it is still full when reading the result
    bool full() {
        auto end = m_end.load(std::memory_order::relaxed);
        return next(end) == m_start.load(std::memory_order::relaxed);
    }

    /**
     * Push an element to the queue
     * @param val: the value to be pushed
     * @note this function will block until there's a slot being able to use
     */
    void push(T val) {
        size_t to_write;
        do {
            to_write = m_end.load(std::memory_order_relaxed);
        } while (next(to_write) == m_start.load(std::memory_order_acquire));
        // now we have at least one slot to use

        m_arr[to_write] = val;
        m_end.store(next(to_write), std::memory_order::release);
    }

    /**
     * pop an elemet out of the queue
     * @return the value to be poped
     * @note this function will block until there's a slot to pop
     */
    T pop() {
        size_t to_pop;
        do {
            to_pop = m_start.load(std::memory_order_relaxed);
        } while (m_end.load(std::memory_order_acquire) == to_pop);

        // now we have at least one slot to use

        auto val = m_arr[to_pop];
        m_start.store(next(to_pop), std::memory_order::release);

        return val;
    }

    /**
     * get the size of the queue
     * @return the size of the queue
     * @note this is the size of the queue at a serilization point.
             doesn't necessarlily mean it is still of that size when using the size
     */
    size_t size() {
        auto to_write = m_end.load(std::memory_order::relaxed);
        auto to_store = m_start.load(std::memory_order::relaxed);

        if (to_write < to_store) return to_write + N - to_store;
        else return to_write - to_store;
    }

    constexpr size_t capacity() const noexcept {
        return N - 1;
    }

    bool try_push(const T& val) {
        auto to_write = m_end.load(std::memory_order_relaxed);
        if (next(to_write) == m_start.load(std::memory_order_acquire)) {
            return false;
        }
        m_arr[to_write] = val;
        m_end.store(next(to_write), std::memory_order::release);
        return true;
    }

    bool try_pop(T& val) {
        auto to_pop = m_start.load(std::memory_order_relaxed);
        auto next_end = m_end.load(std::memory_order_acquire);
        if (to_pop == next_end) {
            return false;
        }
        val = std::move(m_arr[to_pop]);
        m_start.store(next(to_pop), std::memory_order::release);
        return true;
    }
};