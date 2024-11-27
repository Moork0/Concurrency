#include <gtest/gtest.h>

#include "interruptible_thread.h"

#include <format>
#include <future>

TEST(InterruptibleThreadTest, CallInterrupt_ThreadShouldBeInterrupted)
{
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    Concurrency::InterruptibleThread thread ([&promise]()
    {
        for (int i = 0; i < 10; i++)
        {
            if (Concurrency::InterruptibleThread::isInterrupted())
            {
                promise.set_value(true);
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        promise.set_value(false);
    });

    thread.interrupt();

    ASSERT_TRUE(future.wait_for(std::chrono::seconds(5)) == std::future_status::ready);

    EXPECT_EQ(future.get(), true);
}