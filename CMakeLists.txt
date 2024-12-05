cmake_minimum_required(VERSION 3.28.0)
project(Concurrency VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Threads REQUIRED)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wpedantic)
add_compile_options(-Werror)
add_compile_options(-Wformat)              	 	# warn on security issues around functions that format output (ie printf)
add_compile_options(-Wnull-dereference)       	# (only in GCC >= 6.0) warn if a null dereference is detected
add_compile_options(-Wdouble-promotion)       	# (GCC >= 4.6, Clang >= 3.8) warn if float is implicit promoted to double
add_compile_options(-Wsign-compare)
add_compile_options(-Wtype-limits)            	# size_t - size_t >= 0 -> always true


add_subdirectory(containers)
add_subdirectory(utils)
add_subdirectory(threadpool)
add_subdirectory(algorithms)