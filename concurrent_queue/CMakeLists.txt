project(${CMAKE_PROJECT_NAME}.Queue VERSION 0.1.0 LANGUAGES CXX)

find_package(Threads REQUIRED)

add_library(Queue INTERFACE)
target_include_directories(Queue 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(Concurrency::Queue ALIAS Queue)

add_subdirectory(tests)
