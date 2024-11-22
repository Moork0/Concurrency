#ifndef WORK_STEALING_QUEUE_H
#define WORK_STEALING_QUEUE_H

#include <mutex>
#include <deque>
#include <optional>

#include "movable_function.h"

namespace Concurrency::Internal {

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



} // namespace Concurrency::Internal


#endif //WORK_STEALING_QUEUE_H
