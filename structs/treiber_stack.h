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

#include "atomic.hpp"

// This is a lock-free thread-safe stack.
// Note that this stack still have the use-after-free issue. 
// TODO: impl hazard pointer or type-based memory collector to resolve this issue.
template <typename T>
class Stack
{
private:
    using CountedPointer = uint128_t;
    using AtomicCountedPointer = std::atomic<uint128_t>;

    struct Node
    {
        CountedPointer next;
        T val;

        Node(T val): val(std::move(val)), next() {}
        Node(T val, CountedPointer next): val(std::move(val)), next(std::move(next)) {}
    };

    struct CountedPointerUtils
    {
        using Base = uint128_t;

        static Node *pointer(const CountedPointer& countedPtr)
        {
            return reinterpret_cast<Node *>(countedPtr.lower);
        }

        static bool isNull(const CountedPointer& countedPtr)
        {
            return countedPtr.lower == 0;
        }

        static bool cas(AtomicCountedPointer& atomicPointer, CountedPointer &compare, CountedPointer &store)
        {
            return atomicPointer.compare_exchange_strong(compare, store, std::memory_order_acq_rel, std::memory_order_acquire);
        }

        static CountedPointer newPointer(Node* address, uint64_t cnt)
        {
            return CountedPointer(size_t(address), cnt);
        }
    };


private:
    AtomicCountedPointer m_top;
    std::atomic<uint64_t> m_counter; // counter is used for counted pointer.
    std::atomic<size_t> m_size;

public:
    Stack() = default;

    ~Stack() {
        // destructor should be only called once, and only when no thread is using the stack!
        CountedPointer cachedTop = m_top.load();
        while(not CountedPointerUtils::isNull(cachedTop))
        {
            auto nextTop = CountedPointerUtils::pointer(cachedTop)->next;
            assert(CountedPointerUtils::cas(m_top, cachedTop, nextTop));
            delete CountedPointerUtils::pointer(cachedTop);
            cachedTop = nextTop;
        }
    }

    bool empty()
    {
        return CountedPointerUtils::isNull(m_top.load());
    }

    void push(const T& val)
    {
        Node* node = new Node(val, m_top.load(std::memory_order_acquire));
        CountedPointer newNode = CountedPointerUtils::newPointer(node, m_counter.fetch_add(1, std::memory_order_relaxed) + 1);

        // here the new node won't be released until a thread success.
        // we assume that we will only have controlable limited number of threads
        while (not CountedPointerUtils::cas(m_top, node->next, newNode));

        m_size.fetch_add(1, std::memory_order_relaxed);
    }

    T pop()
    {
        auto oldTop = m_top.load();
        while (true)
        {
            while (CountedPointerUtils::isNull(oldTop)) oldTop = m_top.load(std::memory_order_acquire); // block if nothing is there
            while (not CountedPointerUtils::isNull(oldTop) && not CountedPointerUtils::cas(m_top, oldTop, CountedPointerUtils::pointer(oldTop)->next));

            if (not CountedPointerUtils::isNull(oldTop)) break;
        }
        m_size.fetch_sub(1);
        T result = CountedPointerUtils::pointer(oldTop)->val;
        delete CountedPointerUtils::pointer(oldTop);

        return result;
    }
};