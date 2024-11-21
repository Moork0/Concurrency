#include <gtest/gtest.h>

#include <functional>
#include <future>
#include <chrono>
#include <latch>


#include "threadpool.h"

namespace {

constexpr int return_number = 5;
int dummyFunction ()
{
	return return_number;
}

} // namespace

TEST(ThreadPool, SubmitAndReceiveFunctionDoneSignalTest)
{
	Concurrency::ThreadPool thread_pool;

	auto res = thread_pool.submit(dummyFunction);
	
	ASSERT_TRUE(res.has_value());

	auto fut = std::move(res.value());

	ASSERT_EQ(fut.wait_for(std::chrono::seconds(1)), std::future_status::ready);
	EXPECT_EQ(fut.get(), return_number);


	std::latch latch(1);
	auto lambda_result = thread_pool.submit([&latch]()
	{
	    latch.count_down();
	});

	ASSERT_TRUE(lambda_result.has_value());

	auto lambda_res_fut = std::move(lambda_result.value());

	ASSERT_EQ(lambda_res_fut.wait_for(std::chrono::seconds(1)), std::future_status::ready);
	EXPECT_TRUE(latch.try_wait());
}

TEST(ThreadPool, FirstSubmitIsOnGlobalQueueSecondIsOnLocalQueue)
{
    Concurrency::ThreadPool thread_pool;
    ASSERT_FALSE(thread_pool.isCurrentThreadAPoolThread());

    auto res = thread_pool.submit([&thread_pool]()
    {
        EXPECT_TRUE(thread_pool.isCurrentThreadAPoolThread());

        auto inner_res = thread_pool.submit(dummyFunction);
        thread_pool.runPendingTask();

        EXPECT_EQ(inner_res.value().get(), return_number);
    });

    ASSERT_TRUE(res.has_value());
    ASSERT_EQ(res.value().wait_for(std::chrono::seconds(1)), std::future_status::ready);
}