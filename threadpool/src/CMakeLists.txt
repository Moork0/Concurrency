set(
	SRCS
	threadpool.cpp
)

add_library(ThreadPool threadpool.cpp)
add_dependencies(ThreadPool Concurrency::Queue)

target_include_directories(ThreadPool
		PUBLIC
			../include
)

target_link_libraries(ThreadPool
	PUBLIC
	    Threads::Threads
		Concurrency::Queue
)

add_library(Concurrency::ThreadPool ALIAS ThreadPool)
