project(${PROJECT_NAME}.Algorithms VERSION 0.1.0 LANGUAGES CXX)

add_library(Algorithms INTERFACE)

add_dependencies(Algorithms Concurrency::ThreadPool)

target_include_directories(Algorithms
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Algorithms
    INTERFACE
        Concurrency::ThreadPool
)

add_library(Concurrency::Algorithms ALIAS Algorithms)

add_subdirectory(test)
