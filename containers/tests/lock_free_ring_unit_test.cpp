#include <gtest/gtest.h>
#include <algorithm>
#include <future>
#include <latch>
#include <numeric>
#include <thread>

#include "lock_free_ring.h"



namespace {

template <size_t RingSize>
struct EnqueueFuncArgs
{
    Concurrency::LockFreeRing<int, RingSize>&   queue;
    std::latch&                                 latch;
    int                                         number_of_elems;
};

template <size_t RingSize>
struct DequeueFuncArgs
{
    Concurrency::LockFreeRing<int, RingSize>&   queue;
    std::latch&                                 latch;
};

template <size_t RingSize>
void enqueueFunc(EnqueueFuncArgs<RingSize> args)
{
    args.latch.arrive_and_wait();

    int enqueued_elems = 0;
    while (enqueued_elems < args.number_of_elems)
    {
        if (args.queue.enqueue(enqueued_elems))
        {
            ++enqueued_elems;
        }
    }
}

template <size_t RingSize>
std::vector<int> dequeueFunc(DequeueFuncArgs<RingSize> args)
{
    std::vector<int> result;
    args.latch.arrive_and_wait();

    for (size_t i = 0; i < 500'000; ++i)
    {
        std::optional<int> item = args.queue.dequeue();
        if (item.has_value())
        {
            result.push_back(item.value());
        }
    }

    return result;
}

} // namespace

TEST(LockFreeRing, QueueIsEmpty_Pop_ReturnNull)
{
    Concurrency::LockFreeRing<int, 128> queue;
    EXPECT_FALSE(queue.dequeue().has_value());
}

TEST(LockFreeRing, PopItems_OrderIsFIFO)
{
    Concurrency::LockFreeRing<int, 128> queue;
    queue.enqueue(42);
    queue.enqueue(11);

    auto deq_res = queue.dequeue();
    ASSERT_TRUE(deq_res.has_value());
    EXPECT_EQ(deq_res.value(), 42);

    deq_res = queue.dequeue();
    ASSERT_TRUE(deq_res.has_value());
    EXPECT_EQ(deq_res.value(), 11);

    EXPECT_FALSE(queue.dequeue().has_value());
}

TEST(LockFreeRing, PressureTest)
{
    Concurrency::LockFreeRing<int, 4096> queue;
    std::latch                        latch(2);
    constexpr size_t                  num_of_elems = 50'000;

    std::jthread enqueue_thread(enqueueFunc<4096>,
                                 EnqueueFuncArgs{.queue = queue, .latch = latch, .number_of_elems = num_of_elems});

    auto dequeue_fut = std::async(std::launch::async, dequeueFunc<4096>, DequeueFuncArgs{.queue = queue, .latch = latch});

    std::vector<int> dequeued_nums(num_of_elems, 0);

    std::vector<int> dequeue_res = dequeue_fut.get();

    ASSERT_EQ(dequeue_res.size(), num_of_elems);

    for (const int element : dequeue_res)
    {
        ++dequeued_nums[element];
    }

    EXPECT_TRUE(
            std::all_of(dequeued_nums.begin(), dequeued_nums.end(), [](const int element) { return element == 1; }));
}


