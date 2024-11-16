#set(
#	SRCS
#	threadpool.cpp
#)


add_library(ThreadPool INTERFACE)

add_dependencies(ThreadPool Concurrency::Queue)

target_include_directories(ThreadPool
		INTERFACE
			../include
)

target_link_libraries(ThreadPool
	INTERFACE
	    Threads::Threads
		Concurrency::Queue
)

add_library(Concurrency::ThreadPool ALIAS ThreadPool)
