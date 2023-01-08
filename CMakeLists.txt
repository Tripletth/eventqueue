project(eventqueue)

cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_FLAGS "-std=c++20")

add_subdirectory(src)

add_executable(main
    main.cpp
)

target_link_libraries(main
    sources
)
