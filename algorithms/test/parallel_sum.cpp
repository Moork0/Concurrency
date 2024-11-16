#include <gtest/gtest.h>

#include <random>

#include "parallel_sum.h"


namespace Concurrency::Testing {


class ParallelSumTest : public testing::Test
{
public:
    static inline std::random_device    random_device;  // a seed source for the random number engine
    static inline std::mt19937                        gen{random_device()};            // mersenne_twister_engine
    static inline std::uniform_int_distribution<>     distrib {-1000, 1000};

    static auto generateRandomNumber ()
    {
        return distrib(gen);
    }

    static std::vector<int> generateRandomizedVector (const size_t size)
    {
        std::vector<int> results(size);
        std::generate(results.begin(), results.end(), generateRandomNumber);
        return results;
    }

};

TEST_F(ParallelSumTest, SumWithThreadpoolTest)
{
    constexpr size_t size = 10'00;
    std::vector<int> numbers_vec = generateRandomizedVector(size);

    const size_t threadpool_sum_res = parallelAccumulateThreadPool<std::vector<int>::iterator, int, 100>
                                                                    (numbers_vec.begin(), numbers_vec.end(), 0);

    EXPECT_EQ(threadpool_sum_res, std::accumulate(numbers_vec.begin(), numbers_vec.end(), 0));
}

TEST_F(ParallelSumTest, SumWithManualThreadTest)
{
    constexpr size_t size = 10'00;
    std::vector<int> numbers_vec = generateRandomizedVector(size);

    const size_t threadpool_sum_res = parallelAccumulateManualThreading<std::vector<int>::iterator, int, 100>
                                                                        (numbers_vec.begin(), numbers_vec.end(), 0);

    EXPECT_EQ(threadpool_sum_res, std::accumulate(numbers_vec.begin(), numbers_vec.end(), 0));
}

} // namespace Concurrency::Testing

