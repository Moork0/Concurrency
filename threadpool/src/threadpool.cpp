#include "threadpool.h"

namespace Concurrency {

ThreadPool::ThreadPool(size_t number_of_threads)
{
    if (number_of_threads > std::thread::hardware_concurrency())
    {
        number_of_threads = std::thread::hardware_concurrency();
    }

    _local_tasks_queues.reserve(number_of_threads);
    for (size_t i = 0; i < number_of_threads; ++i)
    {
        _local_tasks_queues.emplace_back();
        _threads.emplace_back(&ThreadPool::workerFunc, this, i);
    }
}

ThreadPool::~ThreadPool ()
{
    _done.test_and_set(std::memory_order_relaxed);
}

bool ThreadPool::isCurrentThreadAPoolThread ()
{
    return _this_thread_local_tasks != nullptr;
}

void ThreadPool::runPendingTask ()
{
    std::optional<MovableFunction> task;
    if (_this_thread_local_tasks != nullptr)
    {
        task = _this_thread_local_tasks->dequeue();
    }

    // Try to get a task from global queue if local queue doesn't have any task
    if (!task.has_value())
    {
        task = _global_tasks.pop();
    }

    if (!task.has_value())
    {
        task = stealTasksFromOtherThreads();
    }

    if (task.has_value())
    {
        task.value()();
        return;
    }

    // No task in the task queues. Yield the thread to the OS.
    std::this_thread::yield();
}

void ThreadPool::workerFunc(const size_t thread_index)
{
    _this_thread_idx = thread_index;
    _this_thread_local_tasks = &_local_tasks_queues[thread_index];

    while (!_done.test(std::memory_order_relaxed))
    {
        runPendingTask();
    }
}

std::optional<MovableFunction> ThreadPool::stealTasksFromOtherThreads ()
{
    /**
     * We don't every thread to start from the first queue, creating contention on it.
     * Instead, each thread starts from the next thread's queue in the list.
     */
    const size_t number_of_local_tasks_queues = _local_tasks_queues.size();
    for (size_t i = 0; i < number_of_local_tasks_queues; ++i)
    {
        const size_t index = (_this_thread_idx + i) % number_of_local_tasks_queues;
        auto task = _local_tasks_queues[index].dequeue();
        if (task.has_value())
        {
            return task;
        }
    }

    return std::nullopt;
}

} // namespace Concurrency

