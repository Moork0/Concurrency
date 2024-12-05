#include <benchmark/benchmark.h>

#include <atomic>

#include "lock_based_queue.h"


static void enqueueDequeueSingleThread(benchmark::State& state) {

    Concurrency::ConcurrentQueue<int> q;
    const int number_of_elements = state.range(0);

    for (auto _ : state)
    {
        for (int i = 0; i < number_of_elements; ++i)
        {
            q.push(i);
        }

        for (int i = 0; i < number_of_elements; ++i)
        {
            [[maybe_unused]] std::optional<int> res = q.pop();
            benchmark::DoNotOptimize(res);
        }
    }
}
BENCHMARK(enqueueDequeueSingleThread)->Range(8, 8 << 10);

static void enqueueDequeueSingleThreadInPlace(benchmark::State& state) {

    Concurrency::ConcurrentQueue<int> q;
    const int number_of_elements = state.range(0);

    for (auto _ : state)
    {
        for (int i = 0; i < number_of_elements; ++i)
        {
            q.push(i);
            [[maybe_unused]] std::optional<int> res = q.pop();
            benchmark::DoNotOptimize(res);
        }
    }
}
BENCHMARK(enqueueDequeueSingleThreadInPlace)->Range(8, 8 << 17);


class ConcurrentQueueFixture : public benchmark::Fixture {
public:

    Concurrency::ConcurrentQueue<int> queue;
    std::atomic<int> received_elements = 0;
};

BENCHMARK_DEFINE_F(ConcurrentQueueFixture, EnqueueDequeueSPSC)(benchmark::State& state)
{
    const int number_of_elements = state.range(0);
    switch (state.thread_index())
    {
        case 0:
            for (auto _ : state)
            {
                for (int i = 0; i < number_of_elements; ++i)
                {
                    queue.push(i);
                }
            }
            break;

        case 1:
            int received_elements = 0;
            for (auto _ : state)
            {
                while (received_elements < number_of_elements)
                {
                    if (queue.pop().has_value())
                    {
                        ++received_elements;
                    }
                }
            }
            break;
    }
}
BENCHMARK_REGISTER_F(ConcurrentQueueFixture, EnqueueDequeueSPSC)->Threads(2)->Range(8, 8 << 17);


BENCHMARK_DEFINE_F(ConcurrentQueueFixture, EnqueueDequeueMPMC)(benchmark::State& state)
{
    const int number_of_elements = state.range(0);
    switch (state.thread_index())
    {
        case 0:
        case 1:
            for (auto _ : state)
            {
                for (int i = 0; i < number_of_elements / 2; ++i)
                {
                    queue.push(i);
                }
            }
        break;

        case 2:
        case 3:
            for (auto _ : state)
            {
                while (received_elements.load(std::memory_order_relaxed) < number_of_elements)
                {
                    if (queue.pop().has_value())
                    {
                        received_elements.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            }
        break;
    }
}
BENCHMARK_REGISTER_F(ConcurrentQueueFixture, EnqueueDequeueMPMC)->Threads(4)->Range(8, 8 << 17);
