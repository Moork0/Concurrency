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
    ConcurrentQueue<MovableFunction>	_tasks;

    void workerFunc()
	{
		while (!_done.test(std::memory_order_relaxed))
		{
			auto task = _tasks.pop();
			if (task.has_value())
			{
				task.value()();
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}

public:
    explicit ThreadPool(size_t number_of_threads = std::thread::hardware_concurrency())
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

	~ThreadPool ()
	{
		_done.test_and_set(std::memory_order_relaxed);
	}


	template<typename Func>
	std::optional<std::future<std::invoke_result_t<Func>>> submit (Func function)
	{
		using FuncReturnType = std::invoke_result_t<Func>;
		std::packaged_task<FuncReturnType()> packaged_task(function);
		auto future = packaged_task.get_future();

        const bool push_succeed = _tasks.push(std::move(packaged_task));

		if (!push_succeed)
		{
			return std::nullopt;
		}

		return future;
	}
};

} // namespace Concurrency


#endif // THREADPOOL_H
