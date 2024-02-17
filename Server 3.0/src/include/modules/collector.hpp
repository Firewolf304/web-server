//
// Created by firewolf304 on 24.01.24.
//

#ifndef SERVER_3_0_COLLECTOR_HPP
#define SERVER_3_0_COLLECTOR_HPP
#include "../include.hpp"
namespace firewolf::monitor {
    class collect_connection {
    public:
        std::unordered_map<int,sockaddr_in> clients;
        collect_connection() {}
        void async_input(int client_handle);
    private:
    };
}
#endif //SERVER_3_0_COLLECTOR_HPP
