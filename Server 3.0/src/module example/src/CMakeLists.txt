cmake_minimum_required(VERSION 3.24)
project(modules)

set(CMAKE_CXX_STANDARD 23)
set(SOURCES library.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl -Os -g0")
add_library(modules SHARED library.cpp
        scripts/variables.h
        scripts/usings.h
        scripts/sql.cpp
        scripts/ssl.cpp
        scripts/log_stream.cpp
        scripts/ssl.h
        scripts/sql.h
        scripts/response.h
        scripts/log_stream.h
)
#find_package(PythonLibs REQUIRED )
#include_directories(${PYTHON_INCLUDE_DIRS})
#target_link_libraries(modules ${PYTHON_LIBRARIES})
#find_package(PkgConfig REQUIRED) # Include functions provided by PkgConfig module.
