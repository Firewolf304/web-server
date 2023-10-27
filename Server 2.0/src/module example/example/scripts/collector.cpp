//
// Created by firewolf on 19.10.23.
//
#include "usings.h"
#include <future>
#include <fcntl.h>
#include <sstream>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
namespace firewolf::monitor {
    class collect_connection {
    public:
        std::unordered_map<int,sockaddr_in> clients;
        collect_connection() {}
        void async_input(int client_handle) {
            std::async(std::launch::async, [this, client_handle](){
                sockaddr_in client;
                socklen_t len = sizeof(client);
                getsockname(client_handle, (sockaddr *)&client, &len);
                this->clients[client_handle] = client;
            });
        }
    private:
    };
}