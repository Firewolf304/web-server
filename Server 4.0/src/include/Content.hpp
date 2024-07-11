//
// Created by firewolf304 on 24.03.24.
//

#ifndef SERVER4_0_CONTENT_HPP
#define SERVER4_0_CONTENT_HPP
#include "include.hpp"
#include "module/restAPI.hpp"
#include "module/psql.hpp"


namespace firewolf::web_server::content {
    typedef boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;
    typedef boost::log::trivial::severity_level log_level;
    namespace api {

        class application;
        struct firewolf_global {
        public:
            std::string fileConf;
            std::string serverPath;
            std::string pagesPath = "/pages";
            std::string ip;
            int port;
            bool enable_modules = true;
            bool machine_info = false;
            bool enable_gzip = true;
            int mode_status;
            std::shared_ptr<boost::log::sources::severity_logger<boost::log::trivial::severity_level>> logger;
            std::shared_ptr<application> app;
            std::shared_ptr<nlohmann::json> config = std::make_shared<nlohmann::json>(firewolf_file_config);
            std::shared_ptr<firewolf::sql::sql_dump> psql;
        } firewolf_main_data;
        struct api_info {
        public:
            std::function<void(
                    std::shared_ptr<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>> client,
                    std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request,
                    boost::beast::http::response<boost::beast::http::dynamic_body> & response,
                    std::shared_ptr<firewolf_global> global)> run_func;
            std::string dir_path = "";
            nlohmann::json config = {};
            std::function<std::string()> run_info_func = []() { return ""; };
            void * data = nullptr;
        };
        class application {
        public:
            std::string path = "/apis";
            std::multimap<std::string, api_info> routes;    // functions run
            application( ) { }

            void init() {
                if(firewolf_main_data.logger != nullptr) {
                    BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::debug) << "Init apis";
                }
                auto dirs = std::filesystem::directory_iterator(firewolf_server_path + path);
                if(dirs == end(dirs)) {
                    BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::debug) << "No apis";
                    return;
                }
                for(auto file : dirs) {
                    if(file.is_directory()) {
                        std::ifstream config_file(file.path().string() + "/config.json");
                        if(config_file.is_open()) {
                            try {
                                nlohmann::json config = nlohmann::json::parse( firewolf::methods::open_file(config_file) );
                                std::string path_conf = config["path"].get<std::string>();
                                BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::debug) << "find => " << file.path().filename() << " to => " << path_conf;
                                /*routes[path_conf].route_path = path;
                                routes[path_conf].dir_path = file.path().string();
                                routes[path_conf] = {};
                                routes[path_conf].config = config;*/
                                std::unordered_map<std::string, std::function<void()>> switch_lang = {
                                        {"cpp", [this, &path_conf, &file, &config]() -> void {
                                            char * error;
                                            typedef std::string (*InfoFunc)();
                                            typedef void (*Func)(std::shared_ptr<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>>,
                                                                 std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> ,
                                                                 boost::beast::http::response<boost::beast::http::dynamic_body> & ,
                                                                 std::shared_ptr<firewolf_global>);
                                            void *handle;
                                            Func start; InfoFunc info;
                                            try {
                                                handle = dlopen((file.path().string() + "/main.so").c_str(), RTLD_LAZY | RTLD_DEEPBIND);
                                                if (!handle) {
                                                    BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::error) << "\t\tcancel: cant open module";
                                                    return;
                                                }
                                                info = reinterpret_cast<InfoFunc>(dlsym(handle, "info"));
                                                if ((error = dlerror()) != NULL) {
                                                    BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::error)<< "\t\tcancel: cant load info (" << error << ")";
                                                    return;
                                                }
                                                start = reinterpret_cast<Func>(dlsym(handle, "hello"));
                                                if ((error = dlerror()) != NULL) {
                                                    BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::error)<< "\t\tcancel: cant load start (" << error << ")";
                                                    return;
                                                }
                                                BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::debug) << "\t\t" << info();
                                            } catch(std::exception const & e) {  }


                                            while(std::find_if(this->routes.begin(), this->routes.end(), [&path_conf](const auto& pair) {
                                                return pair.first == path_conf;
                                            }) != this->routes.end()){
                                                path_conf += "1";
                                            }
                                            BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::debug) << "\t\tsetted to " << path_conf;
                                            this->routes.insert ( std::pair<std::string, api_info>(path_conf, { start, file.path().string(), config, info, handle }));
                                        } },
                                        {"lua", [this]() -> void {

                                        } },
                                        {"python3", [this]() -> void {

                                        } },
                                        {"php", [this]() -> void {

                                        } }
                                };
                                switch_lang[config["lang"].get<std::string>()]();
                            } catch (std::exception const & e) {
                                firewolf::methods::create_file(file.path().string() + "/config.json", "{}");
                                BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::error) << "Error set route: " << e.what();
                            }
                        } else {
                            BOOST_LOG_SEV((*firewolf_main_data.logger), log_level::error) << "Cant open config.json in " << file.path().string();
                        }
                    }
                }
            }
            bool is_api(const std::string &path)
            {
                if(routes.find(path) != routes.end())
                {
                    return true;
                }
                else{
                    return false;
                }
            }
        };


        //application app; // for global start

    }

    class check_path{
    public:
        std::unordered_map<std::string, std::string> format = {
                {".js", "text/javascript"},
                {".html", "text/html"},
                {".jpg", "image/jpg"},
                {".png", "image/png"},
                {".txt", "text/html"},
                {".xml","text/xml"},
                {".csv", "text/csv"},
                {".css", "text/css"},
                {".ogg", "audio/ogg"},
                {".pdf", "application/pdf"},
                {".json","text/json"},
                {".zip", "application/zip"},
                {".ico", "image/x-icon"}
        };
        check_path(std::string & path) {
            if(this->is_dir(path)) {
                path+="/main.html";
            }
            this->path = path;
            info = std::filesystem::path(path);
        }
        std::filesystem::path info;
        std::string path = "";
        bool is_dir()
        {
            return std::filesystem::is_directory(this->info);
        }
        bool is_dir(std::string &value)
        {
            const std::filesystem::path information(value);
            return is_directory(information);
        }
        bool is_dir(const std::string &value)
        {
            const std::filesystem::path information(value);
            return is_directory(information);
        }
        bool has_format() {
            return this->info.has_extension();
        }
        std::string return_content() {
            std::string value = this->info.filename().extension().string();
            if(!(this->is_dir()) && has_format(value)) {
                return this->format[value];
            }
            return format["txt"];
        }
        bool has_format(std::string & value)
        {
            if(format.find(value) != format.end())
            {
                return true;
            }
            return false;
        }
    };

    template<class body_type>
    class content_api : public std::enable_shared_from_this<content_api<body_type>> {
    public:
        std::shared_ptr<nlohmann::json> config;
        std::shared_ptr<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>> client;
        std::shared_ptr<boost::beast::http::request<body_type>> request;
        boost::asio::ip::tcp::socket & socket;
        std::string pages;
        boost::beast::http::response<body_type> response {};
        std::shared_ptr<logger> log;
        content_api() = default;
        content_api(boost::asio::ip::tcp::socket & socket, std::shared_ptr<boost::beast::http::request<body_type>> request, std::shared_ptr<logger> log, std::shared_ptr<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>> client)
            : socket( socket ), request(std::move(request)), client(std::move(client)), log(std::move(log)) {
            /*if(firewolf_file_config.contains("SERVER") && firewolf_file_config.contains("PAGES_FOLDER")) {
                this->pages = firewolf_file_config["SERVER"]["PAGES_FOLDER"].get<std::string>();
            }*/
            this->pages = firewolf::web_server::content::api::firewolf_main_data.pagesPath;
            init();
        }
        ~content_api() = default;
        void init() {
            //BOOST_LOG_SEV(*this->log, log_level::debug) << "Method content: " << this->request->target();
            //BOOST_LOG_SEV(*this->log, log_level::warning) << "Global = " << firewolf_file_config["SERVER"]["ADDRESS"].get<std::string>();
            if(firewolf::web_server::content::api::firewolf_main_data.machine_info) {
                utsname unameData;
                uname(&unameData);
                this->response.set("Architecture", std::string(unameData.machine));
                this->response.set("System", std::string(unameData.sysname));
                this->response.set("Version", std::string(unameData.version));
                this->response.set("Release", std::string(unameData.release));
            }
            if(firewolf::web_server::content::api::firewolf_main_data.enable_modules) {
                std::string target = this->request->target();
                size_t found_pos = target.find('?', 0);
                target = target.substr(0, found_pos);
                //if((*firewolf::web_server::content::api::firewolf_main_data.app).is_api(target)) {
                if(firewolf::web_server::content::api::firewolf_main_data.app->routes.contains(target)) {
                    auto get_signal = [](int code) {
                        std::signal(code, SIG_DFL);
                        throw std::runtime_error("undefined signal error (" + std::to_string(code) + ")");
                    };
                    auto get_sigaction = [](int code, siginfo_t *si, void *arg) {
                        std::signal(code, SIG_DFL);
                        throw std::runtime_error("undefined sigaction error (" + std::to_string(code) + ")");
                    };
                    std::signal(SIGSEGV, get_signal);
                    std::signal(SIGPIPE, get_signal);
                    std::signal(SIGALRM, get_signal);
                    std::signal(SIGABRT, get_signal);
                    try {
                        BOOST_LOG_SEV(*this->log, log_level::debug) << "goto " << target;
                        firewolf::web_server::content::api::firewolf_main_data.app->routes.equal_range(target).first->second.run_func(
                                std::move(this->client),
                                std::move(this->request),
                                this->response,
                                std::make_shared<firewolf::web_server::content::api::firewolf_global>(
                                        firewolf::web_server::content::api::firewolf_main_data)
                        );
                    } catch (std::runtime_error const & e) {
                        BOOST_LOG_SEV(*this->log, log_level::error) << "Api runtime error: " << e.what();
                    } catch (std::exception const & e) {
                        BOOST_LOG_SEV(*this->log, log_level::error) << "Api error: " << e.what();
                    }
                    // fastAPI
                    return;
                }
            }
            get_file();
            //boost::beast::ostream(this->response.body()) << "hello";
        }

        void get_file() {
            std::string path_file  = firewolf_server_path + this->pages + std::string(this->request->target());
            try { // max path error!
                check_path data(path_file);
                std::ifstream file( data.path, std::ios_base::binary );

                if(file.good()) {


                    /*std::stringstream buffer;
                    buffer << file.rdbuf();
                    boost::beast::ostream(this->response.body()) << buffer.str();
                    file.close();*/

                    boost::iostreams::filtering_streambuf<boost::iostreams::input > inbuf;
                    inbuf.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_speed)));
                    this->response.set("Connection", "close");
                    this->response.set("Content-Type", data.return_content());
                    this->response.set("Content-Encoding", "gzip");
                    this->response.set("Accept-Encoding", "gzip, deflate");

                    inbuf.push(file);
                    std::ostringstream compressedBuffer;
                    boost::iostreams::copy(inbuf, compressedBuffer);
                    boost::beast::ostream(this->response.body()) << compressedBuffer.str(); // оно нахуй работает
                    file.close();
                    boost::iostreams::close(inbuf);
                } else {
                    this->response.result(boost::beast::http::status::internal_server_error);
                    this->response.keep_alive(false);
                }
            } catch (const std::exception& e) {
                BOOST_LOG_SEV(*this->log, log_level::error) << "Detected error: " << e.what();
                this->response.result(boost::beast::http::status::internal_server_error);
                this->response.keep_alive(false);
            };
        }

        boost::beast::http::response<body_type> get() {
            return std::move(this->response);
        }
    };

}

#endif //SERVER4_0_CONTENT_HPP
