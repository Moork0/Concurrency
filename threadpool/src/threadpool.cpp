#include "threadpool.h"
namespace Concurrency {

ThreadPool::ThreadPool(size_t number_of_threads)
{
    if (number_of_threads > std::thread::hardware_concurrency())
    {
        number_of_threads = std::thread::hardware_concurrency();
    }

    for (size_t i = 0; i < number_of_threads; ++i)
    {
        _threads.emplace_back(&ThreadPool::workerFunc, this);
    }
}

ThreadPool::~ThreadPool ()
{
    _done.test_and_set(std::memory_order_relaxed);
}

bool ThreadPool::isCurrentThreadAPoolThread ()
{
    return _is_a_pool_thread;
}

void ThreadPool::runPendingTask ()
{
    if (!_local_tasks.empty())
    {
        MovableFunction task = std::move(_local_tasks.front());
        _local_tasks.pop_front();
        task();
        return;
    }

    auto task = _global_tasks.pop();
    if (task.has_value())
    {
        task.value()();
        return;
    }

    // No task in the task queues. Yield the thread to the OS.
    std::this_thread::yield();
}

// template<typename Func>
// std::optional<std::future<std::invoke_result_t<Func>>> ThreadPool::submit (Func function)
// {
//     using FuncReturnType = std::invoke_result_t<Func>;
//     std::packaged_task<FuncReturnType()> packaged_task(function);
//     auto future = packaged_task.get_future();
//
//     if (isCurrentThreadAPoolThread())
//     {
//         _local_tasks.push_back(std::move(packaged_task));
//     }
//     else
//     {
//         const bool push_succeed = _global_tasks.push(std::move(packaged_task));
//         if (!push_succeed)
//         {
//             return std::nullopt;
//         }
//     }
//
//     return future;
// }

void ThreadPool::workerFunc()
{
    _is_a_pool_thread = true;
    while (!_done.test(std::memory_order_relaxed))
    {
        runPendingTask();
    }
}

} // namespace Concurrency

