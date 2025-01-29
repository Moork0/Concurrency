#ifndef LOCK_FREE_RING_H
#define LOCK_FREE_RING_H

#include <atomic>
#include <cstdint>
#include <optional>
#include <stdexcept>

/**
 * This file contains a lock-free ring buffer implementation.
 * This implementation is meant to run on X86 architecture. It may have UB on other architectures.
 */

namespace Concurrency {

enum class LockFreeRingType
{
    SPSC
};

template<typename T, size_t N, LockFreeRingType RING_TYPE = LockFreeRingType::SPSC>
requires (std::has_single_bit(N))
class LockFreeRing
{
private:
    static constexpr size_t cache_line_size = 64;

    alignas(cache_line_size) T* _ring;

    alignas(cache_line_size) std::atomic<uint32_t> _head;
    alignas(cache_line_size) std::atomic<uint32_t> _tail;

    static constexpr size_t size_mask = N - 1;
    static constexpr size_t capacity = size_mask;

    [[nodiscard]] constexpr size_t freeSpace(const uint32_t head, const uint32_t tail) const noexcept
    {
        return capacity + tail - head;
    }

    [[nodiscard]] constexpr size_t size(const uint32_t head, const uint32_t tail) const noexcept
    {
        return head - tail;
    }

    static constexpr bool isPowerOf2(const uint32_t number) noexcept
    {
        return number != 0 && (number & (number - 1)) == 0;
    }

    static constexpr size_t alignTo(const size_t number, const size_t align) noexcept
    {
        return (number + (align - 1)) & ~(align - 1);
    }

public:

    LockFreeRing ()
        : _ring(nullptr), _head(0), _tail(0)
    {
        if (!isPowerOf2(N))
        {
            throw std::runtime_error("LockFreeRing size is not a power of 2");
        }

        _ring = new T[N];
    }

    ~LockFreeRing ()
    {
        delete[] _ring;
    }

    LockFreeRing (const LockFreeRing &) = delete;
    LockFreeRing (LockFreeRing&&) = delete;

    LockFreeRing& operator= (const LockFreeRing&) = delete;
    LockFreeRing& operator= (LockFreeRing&&) = delete;


    bool enqueue (T item) noexcept requires (RING_TYPE == LockFreeRingType::SPSC)
    {
        const uint32_t head = _head.load(std::memory_order_relaxed);

        if (freeSpace(head, _tail.load(std::memory_order_relaxed)) < 1)
        {
            return false;
        }

        _ring[head & size_mask] = std::move(item);
        _head.store(head + 1, std::memory_order_release);

        return true;
    }

    std::optional<T> dequeue () noexcept requires (RING_TYPE == LockFreeRingType::SPSC)
    {
        const uint32_t tail = _tail.load(std::memory_order_relaxed);

        if (size(_head.load(std::memory_order_relaxed), tail) < 1)
        {
            return std::nullopt;
        }

        T item = std::move(_ring[tail & size_mask]);
        _tail.store(tail + 1, std::memory_order_release);
        return item;
    }
};


} // namespace Concurrency

#endif // LOCK_FREE_RING_H
