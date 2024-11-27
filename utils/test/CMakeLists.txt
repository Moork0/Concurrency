find_package(GTest REQUIRED)

add_executable(
    ${PROJECT_NAME}_test
	interruptible_thread_test.cpp
)

target_link_libraries(
	    ${PROJECT_NAME}_test
		Concurrency::Utils
    	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)
