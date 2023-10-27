cmake_minimum_required(VERSION 3.24)
project(module)

set(CMAKE_CXX_STANDARD 23)
set(SOURCES library.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl")
add_library(module SHARED library.cpp
        scripts/variables.h
        scripts/usings.h
        scripts/sql.cpp
        scripts/ssl.cpp
        scripts/log_stream.cpp
        scripts/response.cpp
        scripts/collector.cpp
)
