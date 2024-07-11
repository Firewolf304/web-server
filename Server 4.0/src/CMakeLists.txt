include(FetchContent)
cmake_minimum_required(VERSION 3.27)
project(Server4_0)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fexperimental-library")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpqxx -lpq -rdynamic -Wall -lboost_url -lboost_iostreams -O3")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_INCLUDE_DIRS /usr/include/boost)
find_package(Boost 1.83.0 COMPONENTS asio, log)
include_directories(${Boost_INCLUDE_DIRS})
find_package(nlohmann_json)
#find_package (glog 0.7.0 REQUIRED)



FetchContent_Declare(
        liburing4cpp
        GIT_REPOSITORY https://github.com/CarterLi/liburing4cpp.git
        GIT_TAG        async
)
FetchContent_MakeAvailable(liburing4cpp)

add_executable(Server4_0 main.cpp
        src/kernel/main.hpp
        #include/module/logger.hpp
        #src/modules/log_stream.cpp
        include/module/restAPI.hpp
        include/Content.hpp
        include/module/session.hpp
        include/module/custom_code.hpp
        include/config.hpp
        src/methods.hpp
        include/module/application.hpp
        include/module/psql.hpp
        src/kernel/socket.hpp
        #src/web-server.hpp
        #src/kernel/main.hpp
        #include/include.hpp
)
#target_link_libraries (Server4_0 glog::glog)
target_link_libraries(Server4_0 liburing4cpp)
target_link_libraries(Server4_0 ${Boost_LIBRARIES})
target_link_libraries(Server4_0 nlohmann_json::nlohmann_json)
target_link_libraries(Server4_0 pqxx)

# вынужденная мера
find_package(Boost 1.83.0 COMPONENTS iostreams, url)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(Server4_0 ${Boost_LIBRARIES})