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
    static inline std::uniform_int_distribution<>     distrib {-10'000, 10'000};

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
    std::list<int> numbers_lst = generateRandomizedList(10'000);

    ASSERT_FALSE(std::is_sorted(numbers_lst.begin(), numbers_lst.end()));

    numbers_lst = ManualThreadingQuickSort(numbers_lst);

    EXPECT_TRUE(std::is_sorted(numbers_lst.begin(), numbers_lst.end()));
}

TEST_F(QuickSortTest, ThreadPoolCorrectnessTest)
{
    std::list<int> numbers_lst = generateRandomizedList(10'000);
    ThreadPool thread_pool;

    ASSERT_FALSE(std::is_sorted(numbers_lst.begin(), numbers_lst.end()));

    numbers_lst = ThreadPoolQuickSort(numbers_lst, thread_pool);

    EXPECT_TRUE(std::is_sorted(numbers_lst.begin(), numbers_lst.end()));
}

} // namespace Concurrency::Testing

