project(${PROJECT_NAME}.Algorithms VERSION 0.1.0 LANGUAGES CXX)


find_package(GTest REQUIRED)


add_executable(
    ${PROJECT_NAME}_test
		parallel_sum.cpp
		quicksort.cpp
)

add_dependencies(${PROJECT_NAME}_test Concurrency::ThreadPool)
add_dependencies(${PROJECT_NAME}_test Concurrency::Algorithms)

target_link_libraries(
	    ${PROJECT_NAME}_test
		Concurrency::ThreadPool
		Concurrency::Algorithms
    	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)
