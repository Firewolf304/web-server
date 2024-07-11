//
// Created by firewolf304 on 18.03.24.
//

#ifndef SERVER4_0_MAIN_HPP
#define SERVER4_0_MAIN_HPP



#include <utility>
#include "../../include/include.hpp"
#include <liburing/io_service.hpp>
#include "socket.hpp"
namespace firewolf::web_server {
    typedef boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;
    typedef boost::log::trivial::severity_level log_level;


#define DEBUG(logger) BOOST_LOG_STREAM_SEV(logger, log_level::debug)

    template<firewolf::web_server::varsocket::mode socket_type>
    class Server {
    private:
        firewolf::web_server::varsocket::mode sock_mode = socket_type;
        void init_config() {
            // read config
            if(this->fileConf.empty()) {
                this->fileConf = "/config.json";
            }
            std::string temppath = firewolf_server_path + this->fileConf;
            std::string
            text;
            std::ifstream file_conf(temppath);
            if(!file_conf.is_open()) {
                firewolf::methods::create_file(temppath, "{\n"
                                                         "  \"SERVER\" : {\n"
                                                         "    \"ADDRESS\" : \"" + this->ip + "\",\n"
                                                         "    \"PORT\" : " + std::to_string(this->port) + ",\n"
                                                         "    \"ENABLE_MODULES\" : false,\n"
                                                         "    \"PAGES_FOLDER\" : \"" + firewolf::web_server::content::api::firewolf_main_data.pagesPath + "\"\n"
                                                         "  }\n"
                                                         "}");
                BOOST_LOG_SEV( *log, log_level::error ) << "No file config in " << firewolf_server_path + this->fileConf;
                return;
            }
            text = firewolf::methods::open_file(file_conf);
            firewolf_file_config = nlohmann::json::parse(text);
            // use config
            if(firewolf_file_config.contains("SERVER")) {
                if (firewolf_file_config["SERVER"].contains("LOG_LEVEL")) {
                    auto value = static_cast< boost::log::trivial::severity_level >(firewolf_file_config["SERVER"]["LOG_LEVEL"].template get<int>());
                    boost::log::core::get()->set_filter(value <= boost::log::trivial::severity);
                    BOOST_LOG_SEV(*log, log_level::debug) << "Changed enable_modules to " << value;
                } else {
                    boost::log::core::get()->set_filter(boost::log::trivial::info <= boost::log::trivial::severity);
                }
                if(firewolf_file_config["SERVER"].contains("ADDRESS")) {
                    this->ip = firewolf_file_config["SERVER"]["ADDRESS"].template get<std::string>();
                    BOOST_LOG_SEV( *log, log_level::debug ) << "Changed ip to " << this->ip;
                }
                if(firewolf_file_config["SERVER"].contains("PORT")) {
                    this->port = firewolf_file_config["SERVER"]["PORT"].template get<int>();
                    BOOST_LOG_SEV( *log, log_level::debug ) << "Changed port to " << this->port;
                }
                if(firewolf_file_config["SERVER"].contains("ENABLE_MODULES")) {
                    this->enable_modules = firewolf_file_config["SERVER"]["ENABLE_MODULES"].template get<bool>();
                    BOOST_LOG_SEV( *log, log_level::debug ) << "Changed enable_modules to " << this->enable_modules;
                }
                if(firewolf_file_config["SERVER"].contains("MACHINE_INFO")) {
                    firewolf::web_server::content::api::firewolf_main_data.machine_info = firewolf_file_config["SERVER"]["ENABLE_MODULES"].template get<bool>();
                    BOOST_LOG_SEV( *log, log_level::debug ) << "Changed machine_info to " << firewolf::web_server::content::api::firewolf_main_data.machine_info;
                }
            }
            if(firewolf_file_config.contains("SQL")) {
                if (firewolf_file_config["SQL"].contains("PSQL")) {
                    if (firewolf_file_config["SQL"]["PSQL"].contains("ENABLE")) {
                        if(firewolf_file_config["SQL"]["PSQL"]["ENABLE"].get<bool>()) {
                            if (firewolf_file_config["SQL"]["PSQL"].contains("CONFIG")) {
                                try {
                                    firewolf::web_server::content::api::firewolf_main_data.psql = std::make_shared<firewolf::sql::sql_dump>(
                                            firewolf_file_config["SQL"]["PSQL"]["CONFIG"]);
                                    BOOST_LOG_SEV( *log, log_level::debug ) << "Changed psql";
                                } catch (std::exception const &e) {
                                    BOOST_LOG_SEV(*log, log_level::error) << "Error start sql: " << e.what();
                                }
                            }
                        }
                    }
                }
            }

            firewolf::web_server::content::api::firewolf_main_data.ip = this->ip;
            firewolf::web_server::content::api::firewolf_main_data.port = this->port;
            firewolf::web_server::content::api::firewolf_main_data.enable_modules = this->enable_modules;
            firewolf::web_server::content::api::firewolf_main_data.fileConf = this->fileConf;
            if(firewolf_file_config.contains("SERVER") && firewolf_file_config.contains("PAGES_FOLDER")) {
                firewolf::web_server::content::api::firewolf_main_data.pagesPath =  firewolf_file_config["SERVER"]["PAGES_FOLDER"].get<std::string>();
            }
        }
    public:
        std::shared_ptr<logger> log;
        std::shared_ptr<firewolf::web_server::varsocket::Socket<socket_type>> sock;
        std::string fileConf = "/config.json";
        std::string ip = "127.0.0.1";
        int port = 8080;
        bool enable_modules = false;

        Server(Server const & serv) {
            this->port = serv.port;
            this->sock = serv.socket;
            this->ip = serv.ip;
        }
        /*Server(std::string ip, int port) : ip(std::move(ip)), port(port)  {
            this->socket.init();
        }*/

        template<class a, typename p>
        Server(a ip, p port) : ip(std::move(ip)), port(std::move(port))  {}
        ~Server() {  }
        Server() {}
        Server(firewolf::web_server::varsocket::Socket<socket_type> sock) : sock(std::move(sock)) {}

        Server& add_route( std::string const & route, std::function<void(
                                                    const std::shared_ptr<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>> & client,
                                                    const std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>>& request,
                                                    boost::beast::http::response<boost::beast::http::dynamic_body>& response,
                                                    const std::shared_ptr<firewolf::web_server::content::api::firewolf_global> & global)> func ) {
            if(enable_modules)
                firewolf::web_server::content::api::firewolf_main_data.app->routes.insert(std::pair<std::string, content::api::api_info>(route, { func, "", {} }));
            return *this;
        }
        Server& add_route( std::string const & route, firewolf::web_server::content::api::api_info const & structure) {
            if(enable_modules)
                firewolf::web_server::content::api::firewolf_main_data.app->routes.insert(std::pair<std::string, content::api::api_info>(route, structure));
            return *this;
        }
        Server& init() {
            //log init
            this->log = std::make_shared<logger>();
            typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger_t;
            //BOOST_LOG_GLOBAL_LOGGER( logger_t, (*this->log) );
            //(*this->log).remove_all_attributes();
            //core->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
            boost::log::add_common_attributes();
            firewolf::web_server::content::api::firewolf_main_data.logger = (this->log);


            //get pwd of server
            char pwd[PATH_MAX];
            getcwd(pwd, sizeof(pwd));
            firewolf_server_path = std::string(pwd);
            firewolf::web_server::content::api::firewolf_main_data.serverPath = firewolf_server_path;
            // start config init
            if(!firewolf_no_config_file) {
                init_config();
            }
            firewolf::web_server::content::api::firewolf_main_data.app = std::make_shared<firewolf::web_server::content::api::application>();
            if(enable_modules) {
                (*firewolf::web_server::content::api::firewolf_main_data.app).init();
            }
            //socket init
            BOOST_LOG_SEV( *log, log_level::info ) << "Hello, server!";
            BOOST_LOG_SEV( *log, log_level::info ) << "Init address " << ip << ":" << port;
            this->sock = std::make_shared<firewolf::web_server::varsocket::Socket<socket_type>>(ip, port);
            this->sock->log = std::move(this->log);
            //(*this->sock).setValue<1>("");
            (*this->sock).init();
            return *this;
        }
        void listen() noexcept {
            (*this->sock).listen();
        }
    };
}
#endif //SERVER4_0_MAIN_HPP
