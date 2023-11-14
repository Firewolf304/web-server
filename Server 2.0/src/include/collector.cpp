//
// Created by firewolf on 19.10.23.
//
#include "collector.h"
namespace firewolf::monitor {
    void collect_connection::async_input(int client_handle) {
        std::async(std::launch::async, [this, client_handle](){
            sockaddr_in client;
            socklen_t len = sizeof(client);
            getsockname(client_handle, (sockaddr *)&client, &len);
            this->clients[client_handle] = client;
        });
    }
}
