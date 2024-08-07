//
// Created by firewolf304 on 10.02.24.
//
#include "include/web-server.hpp"
#include <cctype>
#include <filesystem>
#include <ctime>

int main() {
    firewolf::web_server::web_server server("config.json");
    server.app.routes["/apis/is_online"] = [](std::string *s, responser::response *rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void *f) -> void {
        rep->header_body += "Content-Type: text/*\r\nAccess-Control-Allow-Origin: *\r\n";
    };
    server.app.routes["/apis/db_change"] = [](std::string *s, responser::response *rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void *f) -> void {
        rep->body_text = "<!doctype html><html> "
                         "<style>body {background-color: #3e3e3e;color: #fff;}pre {white-space: pre-wrap;}</style> " // for style
                         "<pre id=\"result\">";
        main_funcs *funcs = static_cast<main_funcs *>(f);
        try {
            rep->header_body += "test: allowed\r\nContent-Type: text/html; charset=utf-8\r\nContent-Disposition: inline\r\n";

            /*if(!funcs->server->is_alive()){
                rep->code_status = 503;
                throw std::runtime_error("SQL is not alive");
            }*/
            if(req.request_info.method_request != firewolf::requester::request_data::req::POST ) { rep->code_status = 400; throw std::runtime_error("no post request"); }
            if (req.request_info.media_path.empty()) {
                if (req.headers["method"].get<std::string>() == "add") {
                    *funcs->server << "insert into test values (default, '" + req.body + "')" ;
                    rep->header_body += "status: added\r\n";
                } else if(req.headers["method"].get<std::string>() == "search") {
                    auto value = std::stoi(req.body);   // throw invalid_argument or out_of_range
                    pqxx::result result = *funcs->server < "select value from test where id = " + std::to_string(value) + ";" ;
                    for(pqxx::row const & row : result ) {
                        for (auto const &r: row) {
                            rep->body_text += r.c_str() + std::string("\t");
                        }
                    }
                    rep->header_body += "status: found\r\n";
                }
            } else { // мне лень обрабатывать media

            }

        }
        catch (std::exception const & e) {
            funcs->log->out("ERROR", "SQL: " + std::string(e.what()));
            rep->code_status = 400;
            rep->body_text += "[api error: " + std::string(e.what()) + "]";
        }
        catch (std::runtime_error const & e) {
            funcs->log->out("ERROR", "SQL: " + std::string(e.what()));
            rep->body_text += "[api error: " + std::string(e.what()) + "]";
        }
        catch (std::invalid_argument const & e) {
            funcs->log->out("ERROR", "SQL: " + std::string(e.what()));
            rep->code_status = 400;
            rep->body_text += "[api error: " + std::string(e.what()) + "]";
        }
        catch (std::out_of_range const & e) {
            funcs->log->out("ERROR", "SQL: " + std::string(e.what()));
            rep->code_status = 400;
            rep->body_text += "[api error: " + std::string(e.what()) + "]";
        }
        rep->body_text += "</pre></html>";
    };
    server.app.routes["/apis/getpages"] = [&server](std::string *s, responser::response *rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void *f) -> void {
        rep->header_body += "Content-Type: application/json; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\n";
        std::function<json(const std::string&, const std::string&)> recursive = [&recursive](const std::string& path, const std::string& localPath) -> json {
            json value = json::array();
            if(localPath.empty()) {
                value.push_back("/");
            } else {
                value.push_back(localPath);
            }
            for(const auto& d : std::filesystem::directory_iterator(path)) {
                if(d.is_directory()) {
                    //std::cout << localPath + "/" << d.path().filename().string() << std::endl;
                    json value_req = recursive(d.path().string(), localPath + "/" + d.path().filename().string());
                    value.push_back(value_req);
                }
            }
            return value;
        };
        json result = recursive(server.path + std::string("/pages"), "");
        rep->body_text += result.dump();
    };

    server.app.routes["/apis/input"] = [&server](std::string *s, responser::response *rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void *f) -> void {
        try {
            std::ofstream out(server.path + server.PAGE_PATH + "/lab1/file/filetxt", std::ios_base::app);
            if(out.is_open()) {
                json file = json::parse(req.body);
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                char timer[100];
                strftime(timer, 100, "%a %b %d %T %Y", localtime(&time));
                out << std::string(timer) << " Name: '" << file["name"] << "` Comment: " << file["comment"] << std::endl;
                out.close();
                rep->code_status = 201;
            } else {
                rep->code_status = 423;
            }
        }
        catch(const std::runtime_error & e) {}
        catch(const std::out_of_range & e) {}
        catch(const std::exception & e) {}
    };
    server.listenning();
    return 0;
}
