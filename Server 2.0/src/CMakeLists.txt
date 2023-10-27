cmake_minimum_required(VERSION 3.26)
project(Server_2_0)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_COMPILER /usr/bin/g++)
# -rdynamic
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpqxx -lpq -rdynamic")


add_executable(Server_2_0
        main.cpp
        log_stream.cpp
        variables.h
        usings.h
        sql.cpp
        ssl.cpp
        response.cpp
        globals.h
        include.h
        collector.cpp
)

target_link_libraries(Server_2_0 pqxx)
find_package(ZLIB)
target_link_libraries(Server_2_0 ZLIB::ZLIB)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.36.0 COMPONENTS iostreams)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(Server_2_0 ${Boost_LIBRARIES})

find_package(nlohmann_json)
target_link_libraries(Server_2_0 nlohmann_json::nlohmann_json)

find_package(OpenSSL REQUIRED)
target_link_libraries(Server_2_0 OpenSSL::SSL)

include_directories(/usr/include/mono-2.0)
target_link_libraries(Server_2_0 mono-2.0)

find_package(OpenSSL REQUIRED)

target_link_libraries(Server_2_0 curl)