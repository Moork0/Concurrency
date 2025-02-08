#ifndef LOCK_FREE_RING_H
#define LOCK_FREE_RING_H

#include <atomic>
#include <cstdint>
#include <optional>


/**
 * This file contains a lock-free ring buffer implementation.
 * This implementation is meant to run on X86 architecture. It may have UB on other architectures.
 */

namespace Concurrency {

enum class LockFreeRingType
{
    SPSC
};

template<typename T, uint32_t N, LockFreeRingType RING_TYPE = LockFreeRingType::SPSC>
class LockFreeRing
{
private:
    static constexpr uint32_t cache_line_size = 64;

    alignas(cache_line_size) T* _ring;

    alignas(cache_line_size) std::atomic<uint32_t> _head;
    alignas(cache_line_size) std::atomic<uint32_t> _tail;

    static constexpr uint32_t size_mask = N - 1;
    static constexpr uint32_t capacity = size_mask;

    [[nodiscard]] constexpr uint32_t freeSpace(const uint32_t head, const uint32_t tail) const noexcept
    {
        return capacity + tail - head;
    }

    [[nodiscard]] constexpr uint32_t size(const uint32_t head, const uint32_t tail) const noexcept
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
        static_assert(isPowerOf2(N), "LockFreeRing size must be a power of 2");

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

    [[nodiscard]] constexpr uint32_t freeSpace() const noexcept
    {
        return capacity + _tail.load(std::memory_order_seq_cst) - _head.load(std::memory_order_seq_cst);
    }

    [[nodiscard]] constexpr uint32_t size() const noexcept
    {
        return _head.load(std::memory_order_seq_cst) - _tail.load(std::memory_order_seq_cst);
    }

    bool enqueue (T item) noexcept requires (RING_TYPE == LockFreeRingType::SPSC)
    {
        const uint32_t head = _head.load(std::memory_order_relaxed);

        if (freeSpace(head, _tail.load(std::memory_order_relaxed)) < 1)
        {
            return false;
        }

        _ring[head & size_mask] = std::move(item);
        _head.store(head + 1, std::memory_order_relaxed);

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
        _tail.store(tail + 1, std::memory_order_relaxed);
        return item;
    }
};


} // namespace Concurrency

#endif // LOCK_FREE_RING_H
