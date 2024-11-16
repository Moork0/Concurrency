#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <algorithm>
#include <list>

#include "threadpool.h"

namespace Concurrency{

template<typename T>
std::list<T> parallelQuickSort(std::list<T> input)
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

    std::future<std::list<T>> new_lower
    (

        std::async(&parallelQuickSort<T>, std::move(lower_part))
    );

    auto new_higher(parallelQuickSort(std::move(input)));

    result.splice(result.end(),new_higher);
    result.splice(result.begin(),new_lower.get());
    return result;
}

} // namespace Concurrency



#endif //QUICKSORT_H
