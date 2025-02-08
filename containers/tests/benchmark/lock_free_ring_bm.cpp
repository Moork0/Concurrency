#include <benchmark/benchmark.h>

#include "lock_free_ring.h"


static void enqueueDequeueSingleThread(benchmark::State& state) {

    Concurrency::LockFreeRing<int, 8 << 10> q;
    const int number_of_elements = state.range(0);

    for (auto _ : state)
    {
        for (int i = 0; i < number_of_elements; ++i)
        {
            q.enqueue(i);
        }

        for (int i = 0; i < number_of_elements; ++i)
        {
            [[maybe_unused]] std::optional<int> res = q.dequeue();
            benchmark::DoNotOptimize(res);
        }
    }
}
BENCHMARK(enqueueDequeueSingleThread)->Name("LockFreeRing/enqueueDequeueSingleThread")->Range(8, 8 << 10);

static void enqueueDequeueSingleThreadInPlace(benchmark::State& state) {

    Concurrency::LockFreeRing<int, 8 << 17> q;
    const int number_of_elements = state.range(0);

    for (auto _ : state)
    {
        for (int i = 0; i < number_of_elements; ++i)
        {
            q.enqueue(i);
            [[maybe_unused]] std::optional<int> res = q.dequeue();
            benchmark::DoNotOptimize(res);
        }
    }
}
BENCHMARK(enqueueDequeueSingleThreadInPlace)->Name("LockFreeRing/enqueueDequeueSingleThreadInPlace")->Range(8, 8 << 17);


class LockFreeRingFixture : public benchmark::Fixture {
public:

    void clear ()
    {
        const uint32_t size = queue.size();
        for (uint32_t i = 0; i < size; ++i)
        {
            queue.dequeue();
        }
    }

    void SetUp(::benchmark::State& state)
    {
        if (state.thread_index() == 0)
        {
            clear();
        }
    }

    void TearDown(::benchmark::State&)
    {
    }

    Concurrency::LockFreeRing<int, 8 << 18> queue;
};

BENCHMARK_DEFINE_F(LockFreeRingFixture, EnqueueDequeueSPSC)(benchmark::State& state)
{
    const int number_of_elements = state.range(0);
    switch (state.thread_index())
    {
        case 0:
            for (auto _ : state)
            {
                for (int i = 0; i < number_of_elements; ++i)
                {
                    assert(queue.enqueue(i));
                }
            }
            break;

        case 1:
            int received_elements = 0;
            for (auto _ : state)
            {
                while (received_elements < number_of_elements)
                {
                    if (queue.dequeue().has_value())
                    {
                        ++received_elements;
                    }
                }
            }
            break;
    }
}
BENCHMARK_REGISTER_F(LockFreeRingFixture, EnqueueDequeueSPSC)->Threads(2)->Range(8, 8 << 16)->Iterations(1)
                                                            ->Name("LockFreeRing/EnqueueDequeueSPSC")
                                                            ->ReportAggregatesOnly(true)
                                                            ->Repetitions(100);
