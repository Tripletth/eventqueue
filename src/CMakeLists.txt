add_library(sources
    dispatch.cpp
    eventqueue.cpp
)
target_include_directories(sources PUBLIC ${CMAKE_SOURCE_DIR}/include)
