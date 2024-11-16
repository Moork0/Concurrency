#ifndef PARALLEL_SUM_H
#define PARALLEL_SUM_H

#include <numeric>
#include <algorithm>
#include <future>
#include <thread>
#include <random>

#include "threadpool.h"

namespace Concurrency {

template <typename Iterator, typename T>
struct AccumulateBlock
{

	T operator() (Iterator start, Iterator end, T init = T()) const
	{
		return std::accumulate(start, end, init);
	}
};


template <typename Iterator, typename T, size_t min_per_thread = 25>
[[nodiscard]] T parallelAccumulateManualThreading (Iterator start, Iterator end, T init)
{
	const std::size_t length = std::distance(start, end);
	const std::size_t hardware_threads_num = std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();
	const std::size_t maximum_threads = (length + min_per_thread - 1) / min_per_thread;
	const std::size_t number_of_threads = std::min(hardware_threads_num, maximum_threads);
	const std::size_t block_size = length / number_of_threads;

	std::vector<std::thread> threads(number_of_threads - 1);
	std::vector<std::future<T>> results(number_of_threads - 1);

	Iterator block_start = start;
	for (std::size_t i = 0; i < (number_of_threads - 1); ++i)
	{
		Iterator block_end = block_start;
		std::advance(block_end, block_size);

	    std::packaged_task<T(Iterator, Iterator, T)> task(AccumulateBlock<Iterator, T>{});
		results[i] = task.get_future();
		threads[i] = std::thread(std::move(task), block_start, block_end, init);

	    block_start = block_end;
	}

    std::clog << init << std::endl;
	// Calculate the last block in the current thread
	T final_result = AccumulateBlock<Iterator, T>()(block_start, end);
	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    for (std::future<T>& block_result : results)
	{
	    if (block_result.valid())
	    {
	        final_result += block_result.get();
	    }
	}

	return final_result;
}


template <typename Iterator, typename T, size_t chunk_size = 25>
[[nodiscard]] T parallelAccumulateThreadPool (Iterator start, Iterator end, T init)
{
	const std::size_t length = std::distance(start, end);
	if (length == 0)
	{
		return init;
	}

    const size_t num_of_blocks = (length + chunk_size - 1) / chunk_size;
	std::vector<std::future<T>> results(num_of_blocks - 1);
    ThreadPool thread_pool;

	Iterator block_start = start;
	for (std::size_t i = 0; i < num_of_blocks - 1; ++i)
	{
		Iterator block_end = block_start;
		std::advance(block_end, chunk_size);

	    auto res = thread_pool.submit([block_start, block_end, init]()
	    {
	        return AccumulateBlock<Iterator, T>()(block_start, block_end, init);
	    });

	    if (!res.has_value())
	    {
	        throw std::runtime_error("ThreadPool submit failed");
	    }
	    results[i] = std::move(res.value());

	    block_start = block_end;
	}

	// Calculate the last block in the current thread
	T final_result = AccumulateBlock<Iterator, T>()(block_start, end);

    for (std::future<T>& block_result : results)
	{
        if (block_result.valid())
        {
    		final_result += block_result.get();
        }
	}

	return final_result;
}

} // namespace Concurrency


#endif //PARALLEL_SUM_H
