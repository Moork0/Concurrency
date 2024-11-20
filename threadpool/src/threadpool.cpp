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

void ThreadPool::workerFunc()
{
    _is_a_pool_thread = true;
    while (!_done.test(std::memory_order_relaxed))
    {
        runPendingTask();
    }
}

} // namespace Concurrency

