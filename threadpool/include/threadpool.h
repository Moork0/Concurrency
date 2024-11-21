#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <future>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>

#include "concurrent_queue.h"

namespace Concurrency {

class MovableFunction
{

private:

	struct CallableWrapperBase
	{
		virtual void call () = 0;

		virtual ~CallableWrapperBase () = default;
	};

	template <typename Callable>
	struct CallableWrapper final : CallableWrapperBase
	{
		Callable func;

		explicit CallableWrapper (Callable&& f)
			: func(std::move(f))
		{

		}

		void call () override
		{
			func();
		}
	};


	std::unique_ptr<CallableWrapperBase> _func;

public:

	template <typename Func>
	MovableFunction (Func&& func)
		: _func(std::make_unique<CallableWrapper<Func>>(std::move(func)))
	{

	}

	MovableFunction (MovableFunction&& other) noexcept
		: _func(std::move(other._func))
	{
	}

	MovableFunction& operator= (MovableFunction&& other) noexcept
	{		
		_func = std::move(other._func);
		return *this;
	}

	MovableFunction (MovableFunction& other) = delete;
	MovableFunction (const MovableFunction& other) = delete;
	MovableFunction& operator= (const MovableFunction& other) = delete;

	void call () const
	{
		_func->call();
	}

	void operator()() const
	{
		call();
	}

};

class ThreadPool
{
private:
	class WorkStealingQueue
	{
	private:
		using ElemT = MovableFunction;

		mutable std::mutex _queue_mutex;
		std::deque<ElemT>      _queue;

	public:

		WorkStealingQueue	() = default;
		~WorkStealingQueue	() = default;

		WorkStealingQueue (WorkStealingQueue&& other) noexcept
		{
			std::swap(_queue, other._queue);
		}

		WorkStealingQueue& operator= (WorkStealingQueue&& other) noexcept
		{
			std::swap(_queue, other._queue);
			return *this;
		}

		WorkStealingQueue				(const WorkStealingQueue& other) = delete;
		WorkStealingQueue& operator=	(const WorkStealingQueue& other) = delete;

		bool enqueue (ElemT element)
		{
			std::scoped_lock lock(_queue_mutex);
			try
			{
				_queue.push_front(std::move(element));
				return true;

			} catch (std::exception &)
			{
				return false;
			}
		}

		std::optional<ElemT> dequeue()
		{
			std::scoped_lock lock(_queue_mutex);
			if (_queue.empty())
			{
				return std::nullopt;
			}

			ElemT result = std::move(_queue.front());
			_queue.pop_front();

			return result;
		}

		std::optional<ElemT> steal()
		{
			std::scoped_lock lock(_queue_mutex);
			if (_queue.empty())
			{
				return std::nullopt;
			}

			ElemT result = std::move(_queue.back());
			_queue.pop_back();

			return result;
		}
	};

	std::atomic_flag					_done;
    ConcurrentQueue<MovableFunction>	_global_tasks;
	std::vector<WorkStealingQueue>		_local_tasks_queues;
    std::vector<std::jthread>           _threads;

	inline static thread_local WorkStealingQueue*	_this_thread_local_tasks	= nullptr;
	inline static thread_local size_t				_this_thread_idx			= 0;

    void workerFunc (size_t thread_index);

	std::optional<MovableFunction> stealTasksFromOtherThreads ();

public:
    explicit ThreadPool(size_t number_of_threads = std::thread::hardware_concurrency());
	~ThreadPool ();

	static bool isCurrentThreadAPoolThread ();

	void runPendingTask ();

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
