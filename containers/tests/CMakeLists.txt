find_package(GTest REQUIRED)

add_executable(
    ${PROJECT_NAME}_test
	lock_based_queue_unit_test.cpp
)

target_link_libraries(
	    ${PROJECT_NAME}_test
		Concurrency::Containers
    	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)
