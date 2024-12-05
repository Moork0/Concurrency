project(${CMAKE_PROJECT_NAME}.Containers VERSION 0.1.0 LANGUAGES CXX)

find_package(Threads REQUIRED)

add_library(Containers INTERFACE)
target_include_directories(Containers
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(Concurrency::Containers ALIAS Containers)

add_subdirectory(tests)
