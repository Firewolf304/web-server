//
// Created by firewolf304 on 24.01.24.
//
#include "../../include/modules/collector.hpp"
namespace firewolf::monitor {
    void collect_connection::async_input(int client_handle) {
        auto f = std::async (std::launch::async, [this, client_handle]() {
            sockaddr_in client;
            socklen_t len = sizeof(client);
            getsockname(client_handle, (sockaddr *)&client, &len);
            this->clients[client_handle] = client;
        });
    }
}