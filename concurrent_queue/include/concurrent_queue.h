#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include <deque>
#include <mutex>
#include <optional>


namespace Concurrency {

// FIXME: Replace this with a lock-free queue
// TODO: Implement a lock-free ring-buffer queue

template<typename T>
class ConcurrentQueue
{
private:
    mutable std::mutex _queue_mutex;
    std::deque<T>      _queue;


public:
    ConcurrentQueue()  = default;
    ~ConcurrentQueue() = default;

    ConcurrentQueue(const ConcurrentQueue &) = delete;
    ConcurrentQueue(ConcurrentQueue &&)      = delete;

    ConcurrentQueue &operator=(const ConcurrentQueue &) = delete;
    ConcurrentQueue &operator=(ConcurrentQueue &&)      = delete;

    bool push(T element)
    {
        std::scoped_lock lock(_queue_mutex);
        try
        {
            _queue.push_back(std::move(element));
            return true;

        } catch (std::exception &)
        {
            return false;
        }
    }

    std::optional<T> pop()
    {
        std::scoped_lock lock(_queue_mutex);
        if (_queue.empty())
        {
            return std::nullopt;
        }

        T result = std::move(_queue.front());
        _queue.pop_front();

        return result;
    }
};


}; // namespace Concurrency


#endif // CONCURRENT_QUEUE_H
