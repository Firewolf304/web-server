//
// Created by firewolf on 11.09.23.
//

typedef int sock_handle;

#ifndef SERVER_2_0_VARIABLES_H

#define SERVER_2_0_VARIABLES_H
#include "include.h"
//#include "include.h"


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
extern struct epoll_event * evlist; //[NFDS];
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
extern firewolf::modules::main_modules module;
extern vector<json> proxy_path;
//============SQL
extern sql::sql_dump sqller;
//============SQL
#pragma once
struct main_funcs {
    firewolf::streams::logger* log;
    int * timeoutclient;
    api * app_ptr;
    string * ip;
    std::shared_ptr<unsigned int> portptr;
    ssl_space::SSL * ssl_info;
    std::shared_ptr<int> active_epols;
    string * pages;
    json config_Data;
    bool machine_info = false;
    vector<json> proxy_path;
    firewolf::sql::sql_dump * server;
    std::shared_ptr<sock_handle> sockserver;
    firewolf::monitor::collect_connection * client_map;
    firewolf::modules::main_modules * module_map;
    //std::shared_ptr<firewolf::streams::logger> output = output;
};
extern main_funcs main_data;

#endif //SERVER_2_0_VARIABLES_H
