#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <future>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>

#include "lock_based_queue.h"
#include "internal/movable_function.h"
#include "internal/work_stealing_queue.h"

namespace Concurrency {

using Internal::MovableFunction;
using Internal::WorkStealingQueue;

class ThreadPool
{
private:

	std::atomic_flag					_done;
    ConcurrentQueue<MovableFunction>	_global_tasks;
	std::vector<WorkStealingQueue>		_local_tasks_queues;
    std::vector<std::jthread>           _threads;

	inline static thread_local WorkStealingQueue*	_this_thread_local_tasks	= nullptr;
	inline static thread_local size_t				_this_thread_idx			= 0;

    void workerFunc (const size_t thread_index)
    {
    	_this_thread_idx = thread_index;
    	_this_thread_local_tasks = &_local_tasks_queues[thread_index];

    	while (!_done.test(std::memory_order_relaxed))
    	{
    		runPendingTask();
    	}
    }

	std::optional<MovableFunction> stealTasksFromOtherThreads ()
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

public:
    explicit ThreadPool(size_t number_of_threads = std::thread::hardware_concurrency())
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

	~ThreadPool ()
    {
    	_done.test_and_set(std::memory_order_relaxed);
    }


	static bool isCurrentThreadAPoolThread ()
    {
    	return _this_thread_local_tasks != nullptr;
    }

	void runPendingTask ()
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

	template<typename Func>
	std::optional<std::future<std::invoke_result_t<Func>>> submit (Func function)
	{
		using FuncReturnType = std::invoke_result_t<Func>;
		std::packaged_task<FuncReturnType()> packaged_task(function);
		auto future = packaged_task.get_future();

		if (isCurrentThreadAPoolThread())
		{
			const bool push_succeed = _this_thread_local_tasks->enqueue(std::move(packaged_task));
			if (!push_succeed)
			{
				return std::nullopt;
			}
		}
		else
		{
			const bool push_succeed = _global_tasks.push(std::move(packaged_task));
            if (!push_succeed)
            {
            	return std::nullopt;
            }
		}

		return future;
	}
};

} // namespace Concurrency


#endif // THREADPOOL_H
