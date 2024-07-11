//
// Created by firewolf304 on 01.04.24.
//

#ifndef SERVER4_0_SOCKET_HPP
#define SERVER4_0_SOCKET_HPP
#include <utility>
#include "../../include/include.hpp"
#include <liburing/io_service.hpp>
#include "../../include/module/restAPI.hpp"
#include "../../include/module/session.hpp"


namespace firewolf::web_server::varsocket {

    typedef boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;
    typedef boost::log::trivial::severity_level log_level;

    enum mode : int {
        BOOST = 1,
        SOCKET = 0
    };

    template<mode sock_type>
    class Socket {
    private:

        sockaddr_in servAddr{};
        typedef boost::asio::ip::tcp::socket boost_sock;
        typedef std::variant<int, boost_sock> var_sock;
        var_sock sock;

        // Only for boost
        boost::asio::io_context ioc{1};
        std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
        boost::asio::ip::tcp::endpoint endpoint;

        // Only for C socket.h
        std::map<std::string, int> flags = {
                {"family",AF_INET },
                {"socket_flags", fcntl(std::get<int>(this->sock), F_GETFL) | O_NONBLOCK }
        };
        std::shared_ptr<uio::io_service> service;
        fd_set readfds;

        // magic loop (recursion)
        void do_accept() {
            (*this->acceptor).async_accept(std::get<boost_sock>(sock),
                [this](boost::beast::error_code ec) {
                   try {
                       if (!ec) {
                           //Session sess(std::move(std::get<boost_sock>(sock)));
                           std::make_shared<Session_boost>(std::move(std::get<boost_sock>(this->sock)),this->log)->start();
                       }
                   } catch (const std::runtime_error &runtime_error) {

                   } catch (const std::exception &runtime_error) {

                   }
                   do_accept();
                }
            );


        }

    public:
        std::shared_ptr<logger> log;
        std::string ipAddr;
        int port;
        mode socket_mode = mode::BOOST;
        Socket() : socket_mode(sock_type) {}
        Socket(const Socket & copy) noexcept {
            this->ipAddr = copy.ipAddr;
            this->port = copy.port;
            //this->sock = copy.sock;
            this->socket_mode = copy.socket_mode;
        }
        ~Socket() {
            switch(sock_type) {
                case mode::BOOST: break;
                case mode::SOCKET: {
                    close(std::get<int>(sock));
                }
            }
        }

        [[maybe_unused]] Socket (std::string ip, int port) : ipAddr(std::move(ip)), port(port), socket_mode(sock_type) {}

        /*
         * Socket input:
         *      family => Address families from socket.h
         *      socket_type => Types of sockets from socket.h
         *      socket_flags => fcntl flag values from fcntl-linux.h
         */
        template<int t>
        [[maybe_unused]] void setValue(const std::string &name) {
            this->flags[name] = t;
        }
        void init() {
            switch(sock_type) {
                case mode::BOOST: {
                    BOOST_LOG_SEV(*this->log, log_level::debug) << "Init boost socket";
                    boost::beast::error_code error;
                    auto const address = boost::asio::ip::make_address(this->ipAddr);
                    auto porter = static_cast<unsigned short>(this->port);
                    boost::asio::ip::tcp::acceptor te_acc{this->ioc, {address, porter}};
                    //this->acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(this->ioc);
                    this->acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(std::move(te_acc));
                    boost_sock te{ioc};
                    this->sock = std::move(te);
                    // наконец принял компиль спустя 200 попыток капризов
                    if(error) {
                        throw firewolf::code_error::run_error (error.to_string());
                    }
                } break;
                case mode::SOCKET: {
                    BOOST_LOG_SEV(*this->log, log_level::debug) << "Init raw socket";
                    this->sock = 0;
                    memset(&this->servAddr, 0, sizeof(this->servAddr));
                    this->servAddr.sin_family = this->flags["family"];
                    this->servAddr.sin_addr.s_addr = inet_addr(this->ipAddr.c_str());
                    this->servAddr.sin_port = htons(this->port);
                    this->sock = socket(this->flags["family"], SOCK_STREAM, 0);
                    if (fcntl(std::get<int>(this->sock), F_SETFL, this->flags["socket_flags"]) < 0) {
                        throw std::runtime_error("Cant unlock socket");
                    }
                    if(bind(std::get<int>(this->sock), (struct sockaddr*)&this->servAddr, sizeof(this->servAddr)) < 0)
                    {
                        throw std::runtime_error("Cant bind socket");
                    }
                    FD_ZERO(&readfds);
                    FD_SET(std::get<int>(this->sock), &readfds);
                } break;
            }
            BOOST_LOG_SEV(*this->log, log_level::debug) << "Success!";
        }
        void listen() {
            switch(sock_type) {
                case mode::BOOST: {
                    BOOST_LOG_SEV(*this->log, log_level::debug) << "Listening server: " << this->ipAddr << ":" << this->port;
                    do_accept();
                    this->ioc.run();
                    (*this->acceptor).listen();
                }
                    break;
                case mode::SOCKET: {
                    this->service = std::make_shared<uio::io_service>(10000);
                    service->run([this](std::shared_ptr<uio::io_service> & service, int socket) noexcept -> uio::task<void> {
                                     while (true) {
                                         int clientfd = 0;
                                         while ((clientfd = co_await service->accept(socket, nullptr, nullptr)) <= 0);
                                         if(clientfd > socket) {
                                             auto func_async = [&](int client) -> uio::task<void> {
                                                 fcntl(client, F_SETFL, fcntl(client, F_GETFL) | O_NONBLOCK);
                                                 std::make_shared<Session_int>(socket, std::move(this->log)) -> start();
                                                 co_return;
                                                 //co_await accept_for(clientfd);
                                             } (clientfd);
                                         }
                                         co_await service->shutdown(clientfd, SHUT_RDWR);
                                         co_await service->close(clientfd);
                                     }
                                 }(this->service, std::move(std::get<int>(this->sock)))
                    );
                }
                    break;
            }

        }

    };
}

#endif //SERVER4_0_SOCKET_HPP
