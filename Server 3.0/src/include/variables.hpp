//
// Created by firewolf on 11.09.23.
//

typedef int sock_handle;

#ifndef SERVER_2_0_VARIABLES_H

#define SERVER_2_0_VARIABLES_H
#include "include.hpp"
#include "modules/collector.hpp"
#include "modules/log_stream.hpp"
#include "modules/ssl.hpp"
#include "modules/response.hpp"

//============Monitor
extern firewolf::monitor::collect_connection client_map;
//============Monitor
extern string path;
/*----epoll----*/
extern int nfds;
extern int save_nfds;
extern int epfd;
extern int ret;
extern struct epoll_event ev;
extern vector<struct epoll_event> evlist; //[NFDS];
/*----epoll----*/
extern sockaddr_in servAddr;           // address data
extern sock_handle sock;               // socket handle
extern std::shared_ptr<unsigned int> port;                 // port
extern std::string ipAddr;    // ip
extern bool secure_handle;
extern firewolf::streams::logger logging;
extern bool enable_ssl;
extern api app;
extern string PAGE_PATH;
extern int timeout_client;
extern firewolf::ssl_space::SSL space;
//extern firewolf::modules::main_modules module;
extern vector<json> proxy_path;
extern struct timeval sockettimer;
extern firewolf::requester::access_response content_access;
//extern requester::access_response access_resp;
//============SQL
extern sql::sql_dump sqller_postgres;
//============SQL

extern struct main_funcs {
public:
    std::shared_ptr<std::string> pwd;
    firewolf::streams::logger* log{};
    int * timeoutclient{};
    api * app_ptr{};
    string * ip{};
    std::shared_ptr<unsigned int> portptr;
    ssl_space::SSL * ssl_info{};
    std::shared_ptr<int> active_epols;
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
