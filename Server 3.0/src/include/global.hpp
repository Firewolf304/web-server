//
// Created by firewolf304 on 09.02.24.
//

#ifndef SERVER_3_0_GLOBAL_HPP
#define SERVER_3_0_GLOBAL_HPP
#include "include.hpp"
#include "modules/collector.hpp"
#include "modules/log_stream.hpp"
#include "modules/modules.hpp"
#include "modules/response.hpp"
#include "modules/ssl.hpp"
#include "modules/sql.hpp"
/*extern struct main_funcs1 {
    std::shared_ptr<std::string> pwd;
    firewolf::streams::logger* log{};
    int * timeoutclient{};
    api * app_ptr;
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
    //firewolf::modules::main_modules * module_map{}; //unfinished construction
    std::shared_ptr<firewolf::requester::access_response> access_ptr;
    //std::shared_ptr<firewolf::streams::logger> output = output;
} main_data;*/
#endif //SERVER_3_0_GLOBAL_HPP
