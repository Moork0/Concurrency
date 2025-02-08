#include <benchmark/benchmark.h>

#include "moody-camel/readerwritercircularbuffer.h"


static void enqueueDequeueSingleThread(benchmark::State& state) {


    moodycamel::BlockingReaderWriterCircularBuffer<int> q(8 << 10);
    const int number_of_elements = state.range(0);

    for (auto _ : state)
    {
        for (int i = 0; i < number_of_elements; ++i)
        {
            q.try_enqueue(i);
        }

        for (int i = 0; i < number_of_elements; ++i)
        {
            int val = -1;
            q.try_dequeue(val);
            benchmark::DoNotOptimize(val);
        }
    }
}
BENCHMARK(enqueueDequeueSingleThread)->Name("MoodyCamelRing/enqueueDequeueSingleThread")->Range(8, 8 << 10);

static void enqueueDequeueSingleThreadInPlace(benchmark::State& state) {

    moodycamel::BlockingReaderWriterCircularBuffer<int> q(8 << 17);
    const int number_of_elements = state.range(0);

    for (auto _ : state)
    {
        for (int i = 0; i < number_of_elements; ++i)
        {
            q.try_enqueue(i);

            int val = -1;
            q.try_dequeue(val);
            benchmark::DoNotOptimize(val);
        }
    }
}
BENCHMARK(enqueueDequeueSingleThreadInPlace)->Name("MoodyCamelRing/enqueueDequeueSingleThreadInPlace")->Range(8, 8 << 17);


class MoodyCamelRingFixture : public benchmark::Fixture {
public:

    MoodyCamelRingFixture()
        : queue(8 << 18)
    {

    }

    void clear ()
    {
        const uint32_t size = queue.size_approx();
        for (uint32_t i = 0; i < size; ++i)
        {
            queue.try_pop();
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

    moodycamel::BlockingReaderWriterCircularBuffer<int> queue;
};

BENCHMARK_DEFINE_F(MoodyCamelRingFixture, EnqueueDequeueSPSC)(benchmark::State& state)
{
    const int number_of_elements = state.range(0);
    switch (state.thread_index())
    {
        case 0:
            for (auto _ : state)
            {
                for (int i = 0; i < number_of_elements; ++i)
                {
                    assert(queue.try_enqueue(i));
                }
            }
            break;

        case 1:
            int received_elements = 0;
            for (auto _ : state)
            {
                while (received_elements < number_of_elements)
                {
                    int val = -1;
                    if (queue.try_dequeue(val))
                    {
                        ++received_elements;
                    }
                    benchmark::DoNotOptimize(val);
                }
            }
            break;
    }
}
BENCHMARK_REGISTER_F(MoodyCamelRingFixture, EnqueueDequeueSPSC)->Name("MoodyCamelRing/EnqueueDequeueSPSC")
                                                            ->Threads(2)
                                                            ->Range(8, 8 << 16)
                                                            ->Iterations(1)
                                                            ->ReportAggregatesOnly(true)
                                                            ->Repetitions(100);
