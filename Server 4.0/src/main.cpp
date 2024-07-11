#include <iostream>
#include <regex>
#include "include/web-server.hpp"
#include "include/Content.hpp"
std::regex reg(R"((/--|pg_sleep|select|insert|update|;|\w'))", std::regex::icase);
int check_level_user(std::string id, const std::shared_ptr<firewolf::sql::sql_dump> & server) {
    bool checkId = std::regex_search(id, reg) && id.length() == 36;
    if(!checkId) {
        auto res = (*server) <
                   "SELECT users.level FROM session inner join users on users.id = session.user_id where id_session = '" +
                   id + "';";
        if (res.empty()) {
            return 0;
        } else {
            return res[0].at("level").as<int>();
        }
    }
}

int main(int argc, char * argv[]) {

    firewolf::web_server::Server<firewolf::web_server::varsocket::mode::BOOST> server{"127.0.0.1", 82};
    server.init()
        .add_route("/apis/is_online", [](const client_socket & client, const request_ptr& request, response_ptr & response, const firewolf_global_ptr& global){
            response.set("Content-Type", "text/*"); response.set("Access-Control-Allow-Origin", "*");
        })
        .add_route("/apis/db_change", [](const client_socket & client, const request_ptr& request, response_ptr & response, const firewolf_global_ptr& global){
            boost::beast::ostream((response).body()) <<"<!doctype html><html> "
                                                       "<style>body {background-color: #3e3e3e;color: #fff;}pre {white-space: pre-wrap;}</style> " // for style
                                                       "<pre id=\"result\">";
            response.set("Content-Type", "text/html; charset=utf-8");
            response.set("Content-Disposition", "inline");
            try {
                std::string body = boost::beast::buffers_to_string(request->body().data());
                if(request->method() != boost::beast::http::verb::post) { throw std::runtime_error("no post request");}
                if(auto method = request->find("method"); method != request->end()) {
                    if(method->value() == "add") {
                        (*global->psql) << "insert into test values (default, '" + body + "')" ;
                        response.set("status", "added");
                    } else if(method->value() == "search") {
                        auto value = std::stoi(body);
                        pqxx::result result = (*global->psql) < "select value from test where id = " + std::to_string(value) + ";" ;
                        for(pqxx::row const & row : result ) {
                            for (auto const &r: row) {
                                boost::beast::ostream((response).body()) << r.c_str() << std::string("\t");
                            }
                        }
                        response.set("status", "found");
                    }
                }

            }
            catch (std::exception const & e) {
                BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::error) << "SQL: " << e.what();
                response.result(boost::beast::http::status::bad_request);
                boost::beast::ostream((response).body()) << "[api error: " << std::string(e.what()) << "]";
            }
            catch (std::runtime_error const & e) {
                BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::error) << "SQL: " << e.what();
                boost::beast::ostream((response).body()) << "[api error: " << std::string(e.what()) << "]";
            }
            catch (std::invalid_argument const & e) {
                BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::error) << "SQL: " << e.what();
                response.result(boost::beast::http::status::bad_request);
                boost::beast::ostream((response).body()) << "[api error: " << std::string(e.what()) << "]";
            }
            catch (std::out_of_range const & e) {
                BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::error) << "SQL: " << e.what();
                response.result(boost::beast::http::status::bad_request);
                boost::beast::ostream((response).body()) << "[api error: " << std::string(e.what()) << "]";
            }
        });
    server.add_route("/apis/getpages", [](const client_socket & client, const request_ptr& request, response_ptr & response, const firewolf_global_ptr& global){
        response.set("Content-Type", "text/html; charset=utf-8");
        response.set("Access-Control-Allow-Origin", "*");
        std::function<nlohmann::json(const std::string&, const std::string&)> recursive = [&recursive](const std::string& path, const std::string& localPath) -> nlohmann::json {
            nlohmann::json value = nlohmann::json::array();
            if(localPath.empty()) {
                value.push_back("/");
            } else {
                value.push_back(localPath + "/#");
            }
            for(const auto& d : std::filesystem::directory_iterator(path)) {
                if(d.is_directory()) {
                    //std::cout << localPath + "/" << d.path().filename().string() << std::endl;
                    nlohmann::json value_req = recursive(d.path().string(), localPath + "/" + d.path().filename().string());
                    value.push_back(value_req);
                }
            }
            return value;
        };
        nlohmann::json result = recursive(global->serverPath + std::string("/pages"), "");
        boost::beast::ostream((response).body()) << result.dump();
    });
    server.add_route("/apis/input", [](const client_socket & client, const request_ptr& request, response_ptr & response, const firewolf_global_ptr& global){
        std::string body = boost::beast::buffers_to_string(request->body().data());
        BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::debug) << "Used file: " << global->serverPath << global->pagesPath << "/lab1/file/filetxt";
        std::ofstream out(global->serverPath + global->pagesPath + "/lab1/file/filetxt", std::ios_base::app);
        if(out.is_open()) {
            nlohmann::json file = nlohmann::json::parse(body);
            auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char timer[100];
            strftime(timer, 100, "%a %b %d %T %Y", localtime(&time));
            out << std::string(timer) << " Name: '" << file["name"] << "` Comment: " << file["comment"] << std::endl;
            out.close();
            response.result(boost::beast::http::status::created);
        } else {
            response.result(boost::beast::http::status::locked);
        }
    });

    server.add_route("/apis/access", [](const client_socket & client, const request_ptr& request, response_ptr & response, const firewolf_global_ptr& global){
        response.set("Content-Type", "text/json; charset=utf-8");
        response.set("Access-Control-Allow-Origin", "*");
        if(global->psql != nullptr) {
            if(global->psql->is_alive()) {
                nlohmann::json req;
                if(request->body().size() > 0) req = nlohmann::json::parse(boost::beast::buffers_to_string(request->body().data()));
                std::string id = req["id"];
                pqxx::result result;
                try {
                    result = (*global->psql) <
                             "SELECT session.id_session, session.insert_date, session.user_id, session.data, users.login, users.password, users.level, users.role FROM session inner join users on users.id = session.user_id where id_session = '" +
                             id + "';";
                } catch (const std::exception & e ) { BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::error) << "SQL: " << e.what(); }
                nlohmann::json ret;
                if(result.empty()) {
                    ret["login"] = "undefined";
                    ret["role"] = "user";
                    ret["level"] = 0;
                } else {
                    ret["login"] = to_string(result[0]["login"]);
                    ret["role"] = to_string(result[0]["role"]);
                    ret["level"] = result[0]["level"].as<int>();
                }
                boost::beast::ostream((response).body()) << "{\"user_name\" : \"" + ret["login"].get<std::string>() + "\", \"role\" : \"" + ret["role"].get<std::string>() + "\", \"value\":\"";
                std::ifstream f1("access.json");
                nlohmann::json access = nlohmann::json::parse(f1);
                std::ifstream f2("pages.json");
                nlohmann::json pages = nlohmann::json::parse(f2);
                std::string role = ret["role"].get<std::string>();
                int level = ret["level"].get<int>();
                std::unordered_map<std::string, bool> map;
                if(access.contains(role)) {
                    for(auto item : access[role]) {
                        if(item["value"] <= level) {
                            for(auto pattern : item["access"]) {
                                BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::debug) << "Pattern: " << pattern.get<std::string>();
                                for(auto page : pages["PAGES"]) {
                                    std::regex pat(pattern.get<std::string>());
                                    std::string url = page["url"].get<std::string>();
                                    bool check = std::regex_match(url, pat);
                                    BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::debug) << "\tCheck " << url << " - " << (check ? "true" : "false");
                                    if( check && !map.contains(url) ) {
                                        map[url] = true;
                                        boost::beast::ostream((response).body()) << "<li><a href=" << url << ">"
                                                                                 << page["comment"].get<std::string>()
                                                                                 << " "
                                                                                 << page["articles"].get<std::string>()
                                                                                 << "</a>";
                                    }
                                }
                            }
                        }
                    }
                }
                boost::beast::ostream((response).body()) << "\"}";

            }

        }
    }).add_route("/login/login_methods", [](const client_socket & client, const request_ptr& request, response_ptr & response, const firewolf_global_ptr& global){
        if(global->psql != nullptr) {
            if (global->psql->is_alive()) {
                response.set("Content-Type", "text/json; charset=utf-8");
                response.set("Access-Control-Allow-Origin", "*");
                try {
                    nlohmann::json req;
                    if(request->body().size() > 0) req = nlohmann::json::parse(boost::beast::buffers_to_string(request->body().data()));
                    else req["method"] = "";

                    // parse url params
                    auto url = boost::urls::parse_origin_form(request->target()).value();
                    auto url_method = url.params().find("method");

                    if (request->method() == boost::beast::http::verb::post) {
                        if(req["method"] == "create_cookie" || (*url_method).value == "create_cookie" ) {
                            bool checkLogin = std::regex_search(req["login"].get<std::string>(), reg) && req["login"].get<std::string>().length() < 128;
                            bool checkPassword = std::regex_search(req["password"].get<std::string>(), reg) && req["password"].get<std::string>().length() < 128;
                            BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::info) <<
                                        "Create cookie check: " <<
                                        checkLogin << " (" << req["login"].get<std::string>() << ") " <<
                                        checkPassword << " (" << req["password"].get<std::string>() << ")";
                            if(!checkLogin && !checkPassword ) {
                                pqxx::result result;
                                try {
                                    result = (*global->psql) <
                                             "select id, level, login, password from users where login = '" +
                                             req["login"].get<std::string>() + "' and password = '" +
                                             req["password"].get<std::string>() + "';";
                                } catch (const std::exception &e) {}
                                BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::debug)
                                    << "Size response: " << result.size();
                                if (!result.empty() && result.size() == 1) {
                                    std::string user = result[0].at("id").as<std::string>();
                                    std::string level = result[0].at("level").as<std::string>();

                                    if (result[0].at("login").as<std::string>() == req["login"].get<std::string>() &&
                                        result[0].at("password").as<std::string>() ==
                                        req["password"].get<std::string>()) {
                                        try {
                                            (*global->psql) << "insert into session values(gen_random_uuid(), now(), '"
                                                               + user + "', '{}');";
                                        } catch (const std::exception &e) {
                                            (*global->psql) << "delete from session where user_id = '" + user + "';";
                                            (*global->psql) << "insert into session values(gen_random_uuid(), now(), '"
                                                               + user + "', '{}');";
                                        }
                                        //response.result(boost::beast::http::status::created);
                                        result = (*global->psql) <
                                                 "select id_session from session where user_id = '" + user + "';";
                                        boost::beast::ostream((response).body())
                                                << "{\"id_session\" : \"" +
                                                   result[0].at("id_session").as<std::string>() +
                                                   "\", \"user\" : \"" + user +
                                                   "\", \"level\" : \"" + level + "\"}";
                                        return;
                                    }
                                }
                            }
                            boost::beast::ostream(response.body()) << "{\"ERROR\" : \"Incorrect password or login\"}";
                            response.result(boost::beast::http::status::unauthorized);
                        } else if(req["method"] == "check_cookie" || (*url_method).value == "check_cookie") {
                            auto result = (*global->psql) < "SELECT * FROM session where id_session = '" + req["id"].get<std::string>() + "';";
                            if(result.empty()) {
                                boost::beast::ostream((response).body()) << "ERROR";
                                response.result(boost::beast::http::status::unauthorized);
                            } else {
                                boost::beast::ostream((response).body()) << "OK";
                                response.result(boost::beast::http::status::accepted);
                            }
                        } else if(req["method"] == "delete_cookie" || (*url_method).value == "delete_cookie") {
                            boost::beast::ostream((response).body()) << "selected post";
                            auto cookie = firewolf::methods::cookie_to_json( (*request->find(boost::beast::http::field::cookie)).value() );
                            //(*global->psql) << "delete from session where user_id = '" + req["user_id"].get<std::string>() + "' and id_session = '" + req["is_session"].get<std::string>() + "' ;";
                            if(cookie.contains("id") && cookie.contains("user_id")) {
                                (*global->psql) << "delete from session where user_id = '" + cookie["user_id"].get<std::string>() + "' and id_session = '" + cookie["id"].get<std::string>() + "';";
                                response.result(boost::beast::http::status::found);
                                response.set("Location", "/");
                            } else {
                                boost::beast::ostream((response).body()) << "No data";
                                response.result(boost::beast::http::status::no_content);
                            }
                        }
                    } else if (request->method() == boost::beast::http::verb::get) {
                        if(req["method"] == "delete_cookie" || (*url_method).value == "delete_cookie") {
                            boost::beast::ostream((response).body()) << "selected cookie";
                            //BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::debug) << (*request->find(boost::beast::http::field::cookie)).value();
                            auto cookie = firewolf::methods::cookie_to_json( (*request->find(boost::beast::http::field::cookie)).value() );
                            if(cookie.contains("id") && cookie.contains("user_id")) {
                                (*global->psql) << "delete from session where user_id = '" + cookie["user_id"].get<std::string>() + "' and id_session = '" + cookie["id"].get<std::string>() + "';";
                                response.result(boost::beast::http::status::found);
                                response.set("Location", "/");
                            } else {
                                boost::beast::ostream((response).body()) << "No data";
                                response.result(boost::beast::http::status::no_content);
                            }
                        }
                    }
                } catch (const std::exception & e ) {
                    boost::beast::ostream((response).body()) << "[api error]";
                    response.result(boost::beast::http::status::bad_request);
                    BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::error) << "Api error: " << e.what();
                }
            }
        }
    }).add_route("/user_db/user_db", [](const client_socket & client, const request_ptr& request, response_ptr & response, const firewolf_global_ptr& global){
        if(global->psql != nullptr) {
            if (global->psql->is_alive()) {
                response.set("Content-Type", "text/json; charset=utf-8");
                nlohmann::json req;
                if(request->body().size() > 0) req = nlohmann::json::parse(boost::beast::buffers_to_string(request->body().data()));
                else req["method"] = "";
                auto cookie = firewolf::methods::cookie_to_json( (*request->find(boost::beast::http::field::cookie)).value() );
                int level = 0;
                if(cookie.contains("id")) {
                    level = check_level_user(cookie["id"], global->psql);
                } else {
                    boost::beast::ostream(response.body()) << "no access";
                    return;
                }


                // url params
                auto url = boost::urls::parse_origin_form(request->target()).value();
                auto url_method = url.params().find("method");

                auto get_users = [&global, &response, &level]() {
                    auto res = (*global->psql) < "SELECT id, role, level, login, password FROM users ORDER BY role DESC, level DESC;";
                    std::ifstream f1("access.json");
                    nlohmann::json access = nlohmann::json::parse(f1);
                    for(auto row : res) {
                        boost::beast::ostream((response).body()) << "<tr><form class='line'>";
                        boost::beast::ostream((response).body()) << "<td><a id='user_id'>" + row.at("id").as<std::string>() + "</a></td>";
                        boost::beast::ostream((response).body()) << "<td><select id='role'>";
                        for (auto it = access.begin(); it != access.end(); ++it) {
                            boost::beast::ostream((response).body()) << "<option value=\"" + it.key() + "\"";
                            if(it.key() == row.at("role").as<std::string>()) {
                                boost::beast::ostream((response).body()) << "selected";
                            }
                            boost::beast::ostream((response).body()) << ">" + it.key() + "</option>";
                        }
                        boost::beast::ostream((response).body()) << "</select></td>";
                        boost::beast::ostream((response).body()) << "<td><input type=\"number\" id=\"level\" value=\"" + row.at("level").as<std::string>() + "\"></td>";
                        boost::beast::ostream((response).body()) << "<td><input type='text' id='login' value='" + row.at("login").as<std::string>() + "'></td>";
                        boost::beast::ostream((response).body()) << "<td><input type='text' id='password' value='" + row.at("password").as<std::string>() + "'></td>";
                        if(level >= 1000) {
                            boost::beast::ostream((response).body()) << "<td> <button onclick='update(this)'>update</button> </td>";
                        }
                        if(level >= 1000) {
                            boost::beast::ostream((response).body()) << "<td> <button onclick=\"delete_line('" + row.at("id").as<std::string>() + "')\">delete</button> </td>";
                        }
                        boost::beast::ostream((response).body()) << "</tr></form>";
                    }
                    if(level >= 1000) {
                        boost::beast::ostream((response).body()) << "<tr><form class='line'>";
                        boost::beast::ostream((response).body()) << "<td></td>";
                        boost::beast::ostream((response).body()) << "<td><select id='role'>";
                        for (auto it = access.begin(); it != access.end(); ++it) {
                            boost::beast::ostream((response).body()) << "<option value=\"" + it.key() + "\"";
                            boost::beast::ostream((response).body()) << ">" + it.key() + "</option>";
                        }
                        boost::beast::ostream((response).body()) << "</select></td>";
                        boost::beast::ostream((response).body()) << "<td><input type=\"number\" id=\"level\" value=\"1\"></td>";
                        boost::beast::ostream((response).body()) << "<td><input type='text' id='login' value='login'></td>";
                        boost::beast::ostream((response).body()) << "<td><input type='text' id='password' value='password'></td>";
                        boost::beast::ostream((response).body()) << "<td> <button onclick='add(this)'>add</button> </td>";
                        boost::beast::ostream((response).body()) << "</tr></form>";
                    }
                };
                auto update_user = [&global, &response, &level, &req]() {
                    if(level >= 1000) {
                        try {
                            auto res = (*global->psql)
                                    << "update users set login='" + req["login"].get<std::string>() + "', password='" +
                                       req["password"].get<std::string>() + "', level=" +
                                       req["level"].get<std::string>() + ", role='" + req["role"].get<std::string>() +
                                       "' where id='" + req["user_id"].get<std::string>() + "';";
                            boost::beast::ostream((response).body())
                                    << req["user_id"].get<std::string>() + " " + req["password"].get<std::string>();
                            boost::beast::ostream((response).body()) << "Updated";
                            response.result(boost::beast::http::status::created);
                        } catch (const std::exception & e) {
                            boost::beast::ostream((response).body()) << "Error" << e.what();
                            response.result(boost::beast::http::status::bad_request);
                        }
                    }
                };
                auto delete_user = [&global, &response, &level, &req]() {
                    if(level >= 1000) {
                        try {
                            auto res = (*global->psql) << "delete from users where id = '" + req["user_id"].get<std::string>() + "';";
                            boost::beast::ostream((response).body()) << "OK";
                            response.result(boost::beast::http::status::created);
                        } catch (const std::exception & e) {
                            boost::beast::ostream((response).body()) << "Error" << e.what();
                            response.result(boost::beast::http::status::bad_request);
                        }
                    }
                };
                auto add_user = [&global, &response, &level, &req]() {
                    if(level >= 1000) {
                        try {

                            auto res = (*global->psql) << "insert into users values (gen_random_uuid(), '" + req["login"].get<std::string>() + "', '" + req["password"].get<std::string>() + "', " + req["level"].get<std::string>() + ", '" + req["role"].get<std::string>() + "');";
                            BOOST_LOG_SEV((*global->logger), firewolf::web_server::log_level::info) << "Added new user: " << req["login"].get<std::string>();
                            boost::beast::ostream((response).body()) << "OK";
                            response.result(boost::beast::http::status::created);
                        } catch (const std::exception & e) {
                            boost::beast::ostream((response).body()) << "Error: " << e.what();
                            response.result(boost::beast::http::status::bad_request);
                        }
                    }
                };
                auto get_sessions = [&global, &response, &level]() {
                    auto res = (*global->psql) < "select session.id_session, session.insert_date, users.login, session.data from session inner join users on users.id = session.user_id;";
                    for(auto row : res) {
                        boost::beast::ostream((response).body()) << "<tr><form class='line'>";
                        boost::beast::ostream((response).body()) << "<td><a id='id_session'>" + row.at("id_session").as<std::string>() + "</a></td>";
                        boost::beast::ostream((response).body()) << "<td><a>" + row.at("insert_date").as<std::string>() + "</a></td>";
                        boost::beast::ostream((response).body()) << "<td><a>" + row.at("login").as<std::string>() + "</a></td>";
                        boost::beast::ostream((response).body()) << "<td><a>" + row.at("data").as<std::string>() + "</a></td>";
                        if(level >= 1000) {
                            boost::beast::ostream((response).body()) << "<td> <button onclick='delete_sess(this)'>delete</button> </td>";
                        }
                        boost::beast::ostream((response).body()) << "</tr></form>";
                    }
                };
                auto delete_session = [&global, &response, &level, &req]() {
                    if(level >= 1000) {
                        try {
                            (*global->psql) << "delete from session where id_session = '" + req["id_session"].get<std::string>() + "';";
                            boost::beast::ostream((response).body()) << "OK";
                        } catch(const std::exception & e) {
                            boost::beast::ostream((response).body()) << "ERROR";
                        }
                    }
                };
                if(request->method() == boost::beast::http::verb::post) {
                    if (level >= 10) {
                        if(url_method != url.params().end()) {
                            if((*url_method).value == "get_users") {
                                get_users();
                            } else if((*url_method).value == "delete_user") {
                                delete_user();
                            } else if((*url_method).value == "update_user") {
                                update_user();
                            } else if((*url_method).value == "add_user") {
                                add_user();
                            } else if((*url_method).value == "get_sessions") {
                                get_sessions();
                            } else if((*url_method).value == "delete_sessions") {
                                delete_session();
                            }
                        }
                        else {
                            if (req["method"] == "get_users") {
                                get_users();
                            } else if (req["method"] == "delete_user" ) {
                                delete_user();
                            } else if (req["method"] == "update_user" ) {
                                update_user();
                            } else if (req["method"] == "add_user" ) {
                                add_user();
                            } else if (req["method"] == "get_sessions" ) {
                                get_sessions();
                            } else if (req["method"] == "delete_sessions" ) {
                                delete_session();
                            }
                        }
                    }
                } else if(request->method() == boost::beast::http::verb::get) {
                    if (level >= 10) {

                    }
                }
            }
        }
    });

    server.listen();


    return 0;
}
