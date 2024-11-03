#include <gtest/gtest.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <latch>
#include <numeric>
#include <thread>

#include "concurrent_queue.h"

namespace {

struct EnqueueFuncArgs
{
    Concurrency::ConcurrentQueue<int> &queue;
    std::latch                        &latch;
    int                                min;
    int                                max;
};

struct DequeueFuncArgs
{
    Concurrency::ConcurrentQueue<int> &queue;
    std::latch                        &latch;
};

void enqueueFunc(EnqueueFuncArgs args)
{
    args.latch.arrive_and_wait();

    for (int i = args.min; i < args.max; ++i)
    {
        args.queue.push(i);
    }
}

std::vector<int> dequeueFunc(DequeueFuncArgs args)
{
    std::vector<int> result;
    args.latch.arrive_and_wait();

    for (size_t i = 0; i < 500'000; ++i)
    {
        std::optional<int> item = args.queue.pop();
        if (item.has_value())
        {
            result.push_back(item.value());
        }
    }

    return result;
}

} // namespace

TEST(ConcurrentQueue, QueueIsEmpty_Pop_ReturnNull)
{
    Concurrency::ConcurrentQueue<int> queue;
    EXPECT_FALSE(queue.pop().has_value());
}

TEST(ConcurrentQueue, PopItems_OrderIsFIFO)
{
    Concurrency::ConcurrentQueue<int> queue;
    queue.push(42);
    queue.push(11);

    EXPECT_EQ(queue.pop().value(), 42);
    EXPECT_EQ(queue.pop().value(), 11);
    EXPECT_FALSE(queue.pop().has_value());
}

TEST(ConcurrentQueue, PressureTest)
{
    Concurrency::ConcurrentQueue<int> queue;
    std::latch                        latch(4);

    std::jthread enqueue_thread1(enqueueFunc, EnqueueFuncArgs{.queue = queue, .latch = latch, .min = 0, .max = 5000});
    std::jthread enqueue_thread2(enqueueFunc,
                                 EnqueueFuncArgs{.queue = queue, .latch = latch, .min = 5000, .max = 10000});

    auto dequeue_fut1 = std::async(std::launch::async, dequeueFunc, DequeueFuncArgs{.queue = queue, .latch = latch});
    auto dequeue_fut2 = std::async(std::launch::async, dequeueFunc, DequeueFuncArgs{.queue = queue, .latch = latch});

    std::vector<int> dequeued_nums(10000, 0);

    std::vector<int> dequeue_res1 = dequeue_fut1.get();
    std::vector<int> dequeue_res2 = dequeue_fut2.get();

    for (const int element : dequeue_res1)
    {
        ++dequeued_nums[element];
    }

    for (const int element : dequeue_res2)
    {
        ++dequeued_nums[element];
    }

    EXPECT_TRUE(
            std::all_of(dequeued_nums.begin(), dequeued_nums.end(), [](const int element) { return element == 1; }));
}