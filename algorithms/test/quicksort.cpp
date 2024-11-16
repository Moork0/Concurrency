#include <gtest/gtest.h>

#include <random>
#include <algorithm>

#include "quicksort.h"


namespace Concurrency::Testing {

class QuickSortTest : public testing::Test
{
public:
    static inline std::random_device    random_device;  // a seed source for the random number engine
    static inline std::mt19937                        gen{random_device()};            // mersenne_twister_engine
    static inline std::uniform_int_distribution<>     distrib {-1000, 1000};

    static auto generateRandomNumber ()
    {
        return distrib(gen);
    }

    static std::list<int> generateRandomizedList (const size_t size)
    {
        std::list<int> results(size);
        std::generate(results.begin(), results.end(), generateRandomNumber);
        return results;
    }

};

TEST_F(QuickSortTest, ManualThreadingCorrectnessTest)
{
    std::list<int> numbers_vec = generateRandomizedList(10'000);
    numbers_vec = parallelQuickSort(numbers_vec);

    EXPECT_TRUE(std::is_sorted(numbers_vec.begin(), numbers_vec.end()));
}


} // namespace Concurrency::Testing

