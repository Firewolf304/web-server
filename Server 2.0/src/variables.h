//
// Created by firewolf on 11.09.23.
//

typedef int sock_handle;

#ifndef SERVER_2_0_VARIABLES_H
#define SERVER_2_0_VARIABLES_H
#include "include.h"




string path;
/*----epoll----*/
int nfds = 20000;
int save_nfds = nfds;
int epfd;
int ret;
struct epoll_event ev;
struct epoll_event * evlist; //[NFDS];
void close_epolls() {
    for (int i = 0; i < save_nfds; i++)
    {
        close(evlist[i].data.fd);
    }
}
/*----epoll----*/
sockaddr_in servAddr;           // address data
sock_handle sock;               // socket handle
std::shared_ptr<unsigned int> port = std::make_shared<unsigned int>(81);                 // port
std::string ipAddr = "127.0.0.1";    // ip
bool secure_handle = NULL;
firewolf::streams::logger logging;
bool enable_ssl = false;
api app;
string PAGE_PATH = "/pages";
int timeout_client = -1;
firewolf::ssl_space::SSL space(false, &secure_handle);
vector<json> proxy_path;
/*struct main_funcs {
    int * timeoutclient = &timeout_client;
    api * app_ptr = &app;
    string * ip = &ipAddr;
    std::shared_ptr<unsigned int> portptr = port;
    ssl_space::SSL * ssl_info = &space;
    std::shared_ptr<int> active_epols = std::make_shared<int>(nfds);
    string * pages = &PAGE_PATH;
    json config_Data;
    bool machine_info = false;
    vector<json> proxy_path;
    //std::shared_ptr<firewolf::streams::logger> output = output;
} main_data;*/

struct main_funcs {
    firewolf::streams::logger * log;
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
    //std::shared_ptr<firewolf::streams::logger> output = output;
};
main_funcs main_data = {&logging, &timeout_client, &app, &ipAddr, port, &space, std::make_shared<int>(nfds), &PAGE_PATH, NULL, false, {} } ;

#endif //SERVER_2_0_VARIABLES_H
