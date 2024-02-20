//
// Created by firewolf on 11.09.23.
//

typedef int sock_handle;
#ifndef SERVER_2_0_VARIABLES_H
#define SERVER_2_0_VARIABLES_H

#include <iostream>
#include <openssl/ssl.h>
#include <fcntl.h>
#include <sstream>
#include <vector>
#include <filesystem>
#include <zlib.h>
#include <fstream>
#include <future>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/ssl.hpp>
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <sys/utsname.h>
#include "ssl.cpp"
#include "sql.h"
#include "response.h"
#include "log_stream.cpp"
#include "usings.h"
#include "collector.cpp"
struct main_funcs {
public:
    std::shared_ptr<std::string> pwd;
    firewolf::streams::logger* log{};
    int * timeoutclient{};
    api * app_ptr{};
    string * ip{};
    std::shared_ptr<unsigned int> portptr;
    ssl_space::SSL * ssl_info{};
    string * pages{};
    json config_Data;
    json access_Data;
    bool machine_info = false;
    vector<json> proxy_path;
    firewolf::sql::sql_dump * server{};
    std::shared_ptr<sock_handle> sockserver;
    firewolf::monitor::collect_connection * client_map{};
    //firewolf::modules::main_modules * module_map{};
    std::shared_ptr<firewolf::requester::access_response> access_ptr;
    //std::shared_ptr<firewolf::streams::logger> output = output;
} main_data;
#endif //SERVER_2_0_VARIABLES_H
