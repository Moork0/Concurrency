#ifndef MOVABLE_FUNCTION_H
#define MOVABLE_FUNCTION_H

namespace Concurrency::Internal {

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

} // namespace Concurerncy::Internal

#endif //MOVABLE_FUNCTION_H
