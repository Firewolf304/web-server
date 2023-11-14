//
// Created by firewolf304 on 13.11.23.
//

#ifndef SERVER_2_0_COLLECTOR_H
#define SERVER_2_0_COLLECTOR_H
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
        void async_input(int client_handle);
    private:
    };
}
#endif //SERVER_2_0_COLLECTOR_H
