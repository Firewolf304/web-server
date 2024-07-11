//
// Created by firewolf304 on 18.03.24.
//

#ifndef SERVER4_0_WEB_SERVER_HPP
#define SERVER4_0_WEB_SERVER_HPP
#include "../src/kernel/main.hpp"
using namespace firewolf::web_server::content::api;

typedef std::shared_ptr<firewolf_global> firewolf_global_ptr;
typedef std::shared_ptr<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>> client_socket;
typedef std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request_ptr;
typedef boost::beast::http::response<boost::beast::http::dynamic_body> & response_ptr;

#endif //SERVER4_0_WEB_SERVER_HPP
