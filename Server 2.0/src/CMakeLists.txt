cmake_minimum_required(VERSION 3.26)
project(Server_2_0)

set(CMAKE_CXX_STANDARD 23)

add_executable(Server_2_0 main.cpp log_stream.cpp
        variables.h
        usings.h)

find_package(ZLIB)
target_link_libraries(Server_2_0 ZLIB::ZLIB)

find_package(Boost 1.36.0 COMPONENTS iostreams)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(Server_2_0 ${Boost_LIBRARIES})

find_package(nlohmann_json)
target_link_libraries(Server_2_0 nlohmann_json::nlohmann_json)

find_package(OpenSSL REQUIRED)
target_link_libraries(Server_2_0 OpenSSL::SSL)

include_directories(/usr/include/mono-2.0)
target_link_libraries(Server_2_0 mono-2.0)

target_link_libraries(Server_2_0 pqxx)
