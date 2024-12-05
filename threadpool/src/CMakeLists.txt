add_library(ThreadPool INTERFACE)
add_dependencies(ThreadPool Concurrency::Containers)

target_include_directories(ThreadPool
		INTERFACE
			../include
)

target_link_libraries(ThreadPool
	INTERFACE
	    Threads::Threads
		Concurrency::Containers
)

add_library(Concurrency::ThreadPool ALIAS ThreadPool)
