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
	std::atomic_flag					_done;
    std::vector<std::jthread>           _threads;
    ConcurrentQueue<MovableFunction>	_global_tasks;

	inline static thread_local std::deque<MovableFunction>	_local_tasks;
	inline static thread_local bool							_is_a_pool_thread;

    void workerFunc();

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
			_local_tasks.push_back(std::move(packaged_task));
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
