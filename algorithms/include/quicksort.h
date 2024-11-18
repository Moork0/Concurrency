#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <algorithm>
#include <list>

#include "threadpool.h"

namespace Concurrency{

template<typename T>
std::list<T> ManualThreadingQuickSort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(),input,input.begin());

    T const& pivot=*result.begin();

    auto divide_point=std::partition(input.begin(),input.end(),
    [&](T const& t)
    {
        return t < pivot;

    });

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(),
    divide_point);

    std::future<std::list<T>> new_lower
    (
        std::async(&ManualThreadingQuickSort<T>, std::move(lower_part))
    );

    auto new_higher(ManualThreadingQuickSort(std::move(input)));

    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

template<typename T>
std::list<T> ThreadPoolQuickSort(std::list<T> input, ThreadPool& thread_pool)
{
    if(input.empty())
    {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(),input,input.begin());

    T const& pivot=*result.begin();

    auto divide_point=std::partition(input.begin(),input.end(),
    [&](T const& t)
    {
        return t < pivot;

    });

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(),input,input.begin(),
    divide_point);

    auto new_lower
    (
        thread_pool.submit([&thread_pool, &lower_part] ()
        {
            return ThreadPoolQuickSort<T>(std::move(lower_part), thread_pool);
        })
    );

    if (!new_lower.has_value()) [[unlikely]]
    {
        throw std::runtime_error("Submitting task to thread pool failed!");
    }
    std::future<std::list<T>> new_lower_fut = std::move(new_lower.value());

    auto new_higher(ThreadPoolQuickSort(std::move(input), thread_pool));

    result.splice(result.end(), new_higher);

    // Run other pending tasks while waiting for our lower part to get sorted by threadpool.
    while (new_lower_fut.wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
    {
        thread_pool.runPendingTask();
    }
    result.splice(result.begin(), new_lower_fut.get());
    return result;
}

} // namespace Concurrency



#endif //QUICKSORT_H
