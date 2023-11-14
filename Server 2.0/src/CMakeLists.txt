cmake_minimum_required(VERSION 3.26)
project(testServer)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_COMPILER /usr/bin/g++)
# -rdynamic
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpqxx -lpq -rdynamic -fpermissive")


add_executable(testServer
        src/main.cpp
        include/log_stream.cpp
        include/variables.h
        include/usings.h
        include/sql.cpp
        include/response.cpp
        include/globals.h
        include/include.h
        include/collector.cpp
        include/modules.h
        include/log_stream.h
        include/ssl.h
        include/ssl.cpp
        include/response.h
        include/sql.h
        include/collector.h
        src/add.cpp
)

target_link_libraries(testServer pqxx)
find_package(ZLIB)
target_link_libraries(testServer ZLIB::ZLIB)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.36.0 COMPONENTS iostreams)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(testServer ${Boost_LIBRARIES})

find_package(nlohmann_json)
target_link_libraries(testServer nlohmann_json::nlohmann_json)

find_package(OpenSSL REQUIRED)
target_link_libraries(testServer OpenSSL::SSL)

include_directories(/usr/include/mono-2.0)
target_link_libraries(testServer mono-2.0)

find_package(OpenSSL REQUIRED)

target_link_libraries(testServer curl)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
target_link_libraries(testServer ${PYTHON_LIBRARIES})

find_package(Lua51 REQUIRED)
include_directories(/usr/local/include ${LUA_INCLUDE_DIR})
target_link_libraries(testServer ${LUA_LIBRARIES})