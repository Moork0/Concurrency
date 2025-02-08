#find_package(benchmark REQUIRED)

add_executable(
    ${PROJECT_NAME}_test_benchmark
	lock_based_queue_bm.cpp
	lock_free_ring_bm.cpp
	moody_camel_ring_bm.cpp
)

add_dependencies(${PROJECT_NAME}_test_benchmark Concurrency::Containers)

target_compile_options(${PROJECT_NAME}_test_benchmark PRIVATE -O3)

target_link_libraries(
	    ${PROJECT_NAME}_test_benchmark
		Concurrency::Containers
		benchmark
		benchmark_main
		pfm
)
