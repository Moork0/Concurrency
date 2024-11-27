#ifndef INTERRUPTIBLE_THREAD_H
#define INTERRUPTIBLE_THREAD_H

#include <type_traits>
#include <thread>

#include <iostream>
#include <format>

namespace Concurrency {


namespace Internal {

struct InterruptFlag
{
    std::atomic_flag flag {};

    [[nodiscard]] bool isSet () const
    {
        return flag.test(std::memory_order_relaxed);
    }

    void set ()
    {
        flag.test_and_set(std::memory_order_relaxed);
    }
};

}

class InterruptibleThread
{

private:
    inline static thread_local Internal::InterruptFlag this_thread_interrupt_flag;

    Internal::InterruptFlag*    _interrupt_flag;
    std::thread                 _internal_thread;
    std::atomic_flag            _is_ready;

    template <typename Callable, typename... Args>
    void worker (Callable&& callable, Args&&... args)
    {
        _interrupt_flag = &this_thread_interrupt_flag;
        _is_ready.test_and_set();
        _is_ready.notify_one();
        std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

public:

    template <typename Callable, typename... Args>
    requires std::is_invocable_v<Callable, Args...>
    InterruptibleThread(Callable&& callable, Args&&... args)
        : _interrupt_flag{nullptr}
    {
        _internal_thread = std::thread(&InterruptibleThread::worker<Callable, Args...>, this, std::forward<Callable>(callable), std::forward<Args>(args)...);
        _is_ready.wait(false);
    }

    ~InterruptibleThread()
    {
        if (joinable())
        {
            interrupt();
            join();
        }
    }

    InterruptibleThread (const InterruptibleThread&) = delete;
    InterruptibleThread (InterruptibleThread&&) = delete;
    InterruptibleThread& operator=(const InterruptibleThread&) = delete;
    InterruptibleThread& operator=(InterruptibleThread&&) = delete;


    void join ()
    {
        _internal_thread.join();
    }

    void detach ()
    {
        _interrupt_flag = nullptr;
        _internal_thread.detach();
    }

    [[nodiscard]] bool joinable () const
    {
        return _internal_thread.joinable();
    }

    void interrupt ()
    {
        if (!_interrupt_flag)
        {
            return;
        }

        _interrupt_flag->set();
    }

    [[nodiscard]] bool static isInterrupted ()
    {
        return this_thread_interrupt_flag.isSet();
    }
};

} // namespace Concurrency

#endif //INTERRUPTIBLE_THREAD_H
