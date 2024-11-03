project(${PROJECT_NAME}.ThreadPool VERSION 0.1.0 LANGUAGES CXX)


include_directories(include)

add_subdirectory(src)
add_subdirectory(tests)