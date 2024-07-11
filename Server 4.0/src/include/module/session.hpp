//
// Created by firewolf304 on 24.03.24.
//

#ifndef SERVER4_0_SESSION_HPP
#define SERVER4_0_SESSION_HPP
#include "../include.hpp"
#include <liburing/io_service.hpp>
#include "restAPI.hpp"
#include "../Content.hpp"
namespace firewolf::web_server {
    typedef boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;
    typedef boost::log::trivial::severity_level log_level;
    class Session_int : public std::enable_shared_from_this<Session_int> {
    private:
        std::shared_ptr<nlohmann::json> config;
    public:
        int socket = -1;
        std::shared_ptr<logger> log;
        std::shared_ptr<restapi::RestAPI> api;
        Session_int() = default;
        ~Session_int() = default;
        Session_int (int socket, std::shared_ptr<logger> log) : socket(socket), log(std::move(log)) { }
        void start() {

            //api = std::make_shared<restapi::RestAPI>(   )
        }
        void read() {

        }
    };
    class Session_boost : public std::enable_shared_from_this<Session_boost> {
    public:
        std::shared_ptr<nlohmann::json> config;
        boost::asio::ip::tcp::socket socket;
        std::shared_ptr<logger> log;
        restapi::RestAPI_boost<boost::beast::http::dynamic_body> api;
        std::shared_ptr<content::content_api<boost::beast::http::dynamic_body>> content;
        boost::asio::steady_timer deadline_ { socket.get_executor(), std::chrono::seconds(60) };
        std::shared_ptr<Session_boost> ptr;
        boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> remote;
        //boost::asio::steady_timer deadline_{this->socket.get_executor(), std::chrono::seconds(60)};
        Session_boost() = delete;
        ~Session_boost() = default;
        Session_boost (boost::asio::ip::tcp::socket socket, std::shared_ptr<logger> log) : socket(std::move(socket)), log(std::move(log)) { init(); }
        void init(  ) {
            //this->api = std::move(std::make_shared<restapi::RestAPI_boost<boost::beast::http::dynamic_body>>());
        }
        void start() {
            auto self = shared_from_this();
            boost::beast::http::async_read( socket, this->api.buffer_, *this->api.request,
                [self, this](boost::beast::error_code ec, std::size_t bytes_transferred) {
                    boost::ignore_unused(bytes_transferred);
                    if(!ec) {
                        this->remote = self->socket.remote_endpoint();
                        BOOST_LOG_SEV(*self->log, log_level::debug) << "New connect: " << this->remote.address().to_string() << ":" << this->remote.port();
                        BOOST_LOG_SEV(*self->log, log_level::debug) << "Connect: " << (self->api).request->method() << " " << (self->api).request->target() << " " << (self->api).request->version();
                        self->request_body();
                    }
                });
            deadline_.async_wait(
                [self](boost::beast::error_code ec)
                {
                    if(!ec)
                    {
                        self->socket.close(ec);
                    }
                });
        }
    private:
        void request_body() {
            auto handshake = [](int sig) -> void {
                throw std::runtime_error("something wrong with api");
            };
            auto self = shared_from_this();
            signal(SIGSEGV, handshake);
            //signal(SIGABRT, handshake);
            try {
                this->content = std::make_shared<content::content_api<boost::beast::http::dynamic_body>>(this->socket,
                        this->api.request, this->log,
                        std::make_shared<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>>(this->remote));

            } catch (std::runtime_error const & e) {
                BOOST_LOG_SEV(*self->log, log_level::error) << "Content error: " << e.what();
            } catch (std::exception const & e) {
                BOOST_LOG_SEV(*self->log, log_level::error) << "Content error: " << e.what();
            }
            this->api.response = this->content->get();

            write_response();
        }
        void write_response()
        {
            auto self = shared_from_this();
            this->api.make_response();

            boost::beast::http::async_write(
                this->socket,
                this->api.response,
                [self](boost::beast::error_code ec, std::size_t)
                {
                    self->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
                    self->deadline_.cancel();
                });
        }
    };
}
#endif //SERVER4_0_SESSION_HPP
