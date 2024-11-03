set(
	SRCS
	threadpool.cpp
)

add_library(ThreadPool INTERFACE)

target_link_libraries(ThreadPool
	INTERFACE
	    Threads::Threads
		Concurrency::Queue
)

add_library(Concurrency::ThreadPool ALIAS ThreadPool)
