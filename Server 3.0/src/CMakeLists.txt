include(FetchContent)
cmake_minimum_required(VERSION 3.27)
project(Server_3_0)

set(CMAKE_CXX_STANDARD 23)
set(VTK_PYTHON_OPTIONAL_LINK OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/include -I/src -lpqxx -lpq -rdynamic -Wdeprecated-declarations -lpython3.11 ")
# clang: -stdlib=libc++ -Wl

add_executable(Server_3_0 main.cpp
        include/include.hpp
        include/usings.hpp
        include/variables.hpp
        include/modules/modules.hpp
        include/modules/collector.hpp
        src/modules/collector.cpp
        include/modules/log_stream.hpp
        src/modules/log_stream.cpp
        #include/global.h
        include/modules/response.hpp
        include/modules/sql.hpp
        src/modules/sql.cpp
        include/modules/ssl.hpp
        src/modules/ssl.cpp
        src/add.cpp
        include/web-server.hpp
)
target_link_libraries(Server_3_0 pqxx)
find_package(ZLIB)
target_link_libraries(Server_3_0 ZLIB::ZLIB)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.36.0 COMPONENTS iostreams)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(Server_3_0 ${Boost_LIBRARIES})

find_package(nlohmann_json)
target_link_libraries(Server_3_0 nlohmann_json::nlohmann_json)

find_package(OpenSSL REQUIRED)
target_link_libraries(Server_3_0 OpenSSL::SSL)

#include_directories(/usr/include/mono-2.0)
#target_link_libraries(Server_3_0 mono-2.0)

find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED) # Include functions provided by PkgConfig module.

target_link_libraries(Server_3_0 curl)

find_path( Python3 python3/Python.h
    `   /usr/include/
        /usr/include/python3.11
)
#find_package(PythonLibs REQUIRED)
#pkg_check_modules( python python3 )
#message("here: ${PythonLibs}")
include_directories( /usr/include/python3.11)
#include_directories(PkgConfig::python)
#target_link_libraries(Server_3_0 PythonInterp)

find_package(Lua51 REQUIRED)
include_directories(/usr/local/include ${LUA_INCLUDE_DIR})
target_link_libraries(Server_3_0 ${LUA_LIBRARIES})
FetchContent_Declare(
        liburing4cpp
        GIT_REPOSITORY https://github.com/CarterLi/liburing4cpp.git
        GIT_TAG        async
)
FetchContent_MakeAvailable(liburing4cpp)
target_link_libraries(Server_3_0 liburing4cpp)