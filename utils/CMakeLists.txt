project(${PROJECT_NAME}.Utils VERSION 0.1.0 LANGUAGES CXX)

add_library(Utils INTERFACE)
target_include_directories(Utils
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(Concurrency::Utils ALIAS Utils)

add_subdirectory(test)