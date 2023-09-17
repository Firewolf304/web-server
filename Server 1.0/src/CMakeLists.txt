cmake_minimum_required(VERSION 3.24)
project(Server)

set(CMAKE_CXX_STANDARD 23)

add_executable(Server main.cpp response.cpp ssl.cpp log_stream.cpp
        sql.cpp)
find_package(ZLIB)
target_link_libraries(Server ZLIB::ZLIB)

find_package(Boost 1.36.0 COMPONENTS iostreams)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(Server ${Boost_LIBRARIES})

find_package(nlohmann_json)
target_link_libraries(Server nlohmann_json::nlohmann_json)

find_package(OpenSSL REQUIRED)
target_link_libraries(Server OpenSSL::SSL)

include_directories(/usr/include/mono-2.0)
target_link_libraries(Server mono-2.0)

target_link_libraries(Server pqxx)
