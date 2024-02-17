//
// Created by firewolf304 on 10.02.24.
//
#include "include/web-server.hpp"
#include <cctype>
int main() {
    firewolf::web_server::web_server server("config.json");

    server.app.routes["/apis/db_change"] = [](std::string *s, responser::response *rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void *f) -> void {
        signal(SIGSEGV, [](int sig) -> void { throw std::runtime_error("something wrong with db"); });
        rep->body_text = "<!doctype html><html> "
                         "<style>body {background-color: #3e3e3e;color: #fff;}pre {white-space: pre-wrap;}</style> " // for style
                         "<pre>";
        try {
            rep->header_body += "test: allowed\r\nContent-Type: text/html; charset=utf-8\r\nContent-Disposition: inline\r\n";
            main_funcs *funcs = static_cast<main_funcs *>(f);
            if(req.request_info.method_request != firewolf::requester::request_data::req::POST ) { throw std::runtime_error("no post request"); }
            if (req.request_info.media_path.empty()) {
                if (req.headers["method"].get<std::string>() == "add") {
                    *funcs->server << "insert into test values (default, '" + req.body + "')" ;
                    rep->header_body += "status: added\r\n";
                } else if(req.headers["method"].get<std::string>() == "search") {
                    auto value = std::stoi(req.body);   // throw invalid_argument or out_of_range
                    pqxx::result result = funcs->server->operator<<("select b from test where a = " + std::to_string(value) + ";") ;
                    pqxx::row const & row = result[0];
                    for(auto const & r : row) {
                        rep->body_text += r.c_str() + std::string("\t");
                    }
                    rep->header_body += "status: found\r\n";
                }
            } else { // мне лень обрабатывать media

            }

        }
        catch (std::exception const & e) {
            rep->code_status = 400;
            rep->body_text += "[api error: " + std::string(e.what()) + "]";
        }
        catch (std::runtime_error const & e) {
            rep->code_status = 400;
            rep->body_text += "[api error: " + std::string(e.what()) + "]";
        }
        catch (std::invalid_argument const & e) {
            rep->code_status = 400;
            rep->body_text += "[api error: " + std::string(e.what()) + "]";
        }
        catch (std::out_of_range const & e) {
            rep->code_status = 400;
            rep->body_text += "[api error: " + std::string(e.what()) + "]";
        }
        rep->body_text += "</pre></html>";
    };
    server.listenning();
    return 0;
}
