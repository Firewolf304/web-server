#include "../include/variables.hpp" // вызывать где надо
firewolf::streams::logger logging;
string path;
sql::sql_dump sqller_postgres;
int nfds = 20000;
int save_nfds = nfds;
int epfd;
api app;
int ret;
struct epoll_event ev;
firewolf::requester::access_response content_access;
vector<struct epoll_event> evlist; //[NFDS];
std::shared_ptr<unsigned int> port = std::make_shared<unsigned int>(81);                 // port
std::string ipAddr = "127.0.0.1";    // ip
sockaddr_in servAddr;           // address data
sock_handle sock;               // socket handle
bool secure_handle = NULL;
bool enable_ssl = false;
string PAGE_PATH = "/pages";
int timeout_client = -1;
firewolf::ssl_space::SSL space(false, &secure_handle);
firewolf::monitor::collect_connection client_map;
//firewolf::modules::main_modules module;
struct timeval sockettimer;
main_funcs main_data = {std::make_shared<std::string>(path), &logging, &timeout_client, &app,
                        &ipAddr, port, &space,
                        std::make_shared<int>(nfds), &PAGE_PATH,
                        NULL, false, {}, {}, &sqller_postgres,
                        std::make_shared<sock_handle>(sock), &client_map, //&module,
                        std::make_shared<firewolf::requester::access_response>(content_access)
};
void close_epolls() {
    for (int i = 0; i < save_nfds; i++)
    {
        close(evlist[i].data.fd);
    }
}
std::string open_file( std::ifstream & file ) {
    std::string line, text;
    if (file.is_open()) {
        while (getline(file, line)) {
            text += line + "\n";
        }
        file.close();
    }
    return text;
}
void read_cfg() {
    std::ifstream file_cfg("config.json", std::ios::binary);
    std::string text_cfg = open_file(file_cfg);
    file_cfg.close();
    std::ifstream file_access("access.json", std::ios::binary);
    if(file_access.good()) {
        std::string text_access = open_file(file_access);
        nlohmann::json config_access = nlohmann::json::parse(text_access);
        content_access = firewolf::requester::access_response( config_access );
    }
    file_access.close();
    //cout << "LINE PARSE " + text_cfg;
    nlohmann::json config = nlohmann::json::parse(text_cfg);
    if(config["SERVER"].contains("CONFIG")) {
        *(main_data.active_epols) = config["SERVER"]["CONFIG"]["EPOLLS_COUNT"].get<int>();
        *(main_data.ip) = config["SERVER"]["CONFIG"]["IP"].get<string>();
        *(main_data.portptr) =
                config["SERVER"]["CONFIG"]["PORT"].get<uint>();
        *(main_data.pages) = config["SERVER"]["CONFIG"]["PATH_PAGES"].get<string>();
        main_data.machine_info = config["SERVER"]["CONFIG"]["MACHINE_INFO"].get<bool>();
        /*if(config["SERVER"]["CONFIG"].contains("MODS")) {
            if(config["SERVER"]["CONFIG"]["MODS"]["ENABLE"].get<bool>()) {
                module.load();
            }
        }*/
    }
    if(config["SERVER"].contains("SSL") == true) {
        enable_ssl = config["SERVER"]["SSL"]["ENABLE"].get<bool>();
    }
    if(config.contains("PROXY_SERVER")) {
        main_data.proxy_path = config["PROXY_SERVER"]["PROXY"];
    }
    try {
        if(config["SERVER"].contains("SQL")) {
            main_data.server->configure(config["SERVER"]["SQL"]["CONFIG"]);
        }
    } catch (const std::exception& e) {
        cout << "Error connect to SQL: " << e.what() << endl;
    }
    cout<< "------------------\nProxy pathes:" << endl;
    for(json data : main_data.proxy_path) {
        cout << "\t" + data["PATH"].get<string>() + (string)" to " + data["ADRESS"].get<string>() << endl;
        main_data.app_ptr->access[data["PATH"]] = data;
        main_data.app_ptr->routes[data["PATH"]] = [](std::string * s, responser::response* rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void*funcs) {
            //std::async(launch::async, logger::log_chat_async, "SERVER", logger::type_log::DEBUG, "RUNNING PROXY PATH");
            try {
                std::unordered_map<std::string, std::function<void()>> switch_lang = {
                        {"curl", [&s, &rep, &req, &access_info](){
                            CURL* curl = curl_easy_init();
                            string headers;
                            if (curl) {
                                std::regex pattern(R"(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})");
                                curl_easy_setopt(curl, CURLOPT_URL, (access_info[req.request_info.path]["PROTOCOL"].get<string>() + "://" +
                                                                     access_info[req.request_info.path]["ADRESS"].get<string>() +
                                                                     ( (regex_match(access_info[req.request_info.path]["ADRESS"].get<string>(), pattern)) ?
                                                                       ":" + access_info[req.request_info.path]["PORT"].get<string>() : "" ) +
                                                                     access_info[req.request_info.path]["PROXY_PATH"].get<string>() ).c_str() );      // header set option
                                curl_slist* header_list = nullptr;
                                bool client = access_info[req.request_info.path]["CONFIG_HEADERS"]["USE_CLIENT_HEADERS"].get<bool>();
                                for (auto it = access_info[req.request_info.path]["HEADERS"].begin(); it != access_info[req.request_info.path]["HEADERS"].end(); ++it) { // config
                                    header_list = curl_slist_append(header_list, (it.key() + ": " + it.value().get<string>() ).c_str() );
                                    if (client && (req.headers.contains(it.key()))) {
                                        req.headers.erase(it.key());
                                    }
                                }
                                if (client) {
                                    for (auto it = req.headers.begin(); it != req.headers.end(); ++it) { // client
                                        auto rem = access_info[req.request_info.path]["CONFIG_HEADERS"]["REMOVE_CLIENT_HEADERS"];
                                        if (std::find(rem.begin(), rem.end(), it.key()) != rem.end()) continue;
                                        header_list = curl_slist_append(header_list, (it.key() + ": " + it.value().get<string>() ).c_str() );
                                    }
                                }
                                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
                                // ----------------- get data options -----------------
                                auto get_data = [](char *contents, size_t size, size_t nmemb, void *userp) {
                                    *((std::string*)userp) += (string)((char*)contents);
                                    return size * nmemb;
                                };
                                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, static_cast<size_t(*)(char*,size_t, size_t, void*)>(get_data));
                                curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
                                curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, static_cast<size_t(*)(char*,size_t, size_t, void*)>(get_data));
                                curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);
                                // ----------------- get data options -----------------
                                /*CURLcode res = curl_easy_perform(curl); // run request
                                if (res != CURLE_OK) {
                                    char error[256];
                                    ERR_error_string(res, static_cast<char *>(error));
                                    throw std::runtime_error("CURL NOT OK: " + (string)error );
                                    delete[] &error;
                                }*/
                                CURLM* multi_handle = curl_multi_init();
                                curl_multi_add_handle(multi_handle, curl);
                                int still_running = 0;
                                do {
                                    curl_multi_perform(multi_handle, &still_running);
                                } while (still_running);
                                curl_multi_remove_handle(multi_handle, curl);
                                curl_easy_cleanup(curl);
                                curl_multi_cleanup(multi_handle);
                                headers = firewolf::requester::pop_line(headers, "HTTP/2 200", "\r\n");
                                if (access_info[req.request_info.path]["RESPONSE_CONFIG"]["REMOVE_HEADERS"].get<bool>()) {
                                    auto array = access_info[req.request_info.path]["RESPONSE_CONFIG"]["REMOVE_HEADERS_NAME"];
                                    for (auto d = array.begin(); d != array.end(); ++d) {// format to array?
                                        headers = firewolf::requester::pop_line(headers, d.value().get<string>() + ": ", "\r\n");
                                    }
                                }
                                rep->header_body += headers.substr(0, headers.find("\r\n\r\n") + 2);
                                curl_slist_free_all(header_list);
                                curl_easy_cleanup(curl);
                            }
                        }},
                        {"socket", [&s, &rep, &req, &access_info](){

                        }}
                };
                switch_lang[access_info[req.request_info.path]["METHOD"].get<std::string>()]();

            }
            catch (boost::system::system_error& exp) {
                logging.out("ERROR", "ALERT! Api error: " + req.request_info.method + " " + req.request_info.path + " " + req.request_info.media_path + "\nwhat: " + exp.what() );
                *s+="[api error]";
            }
            catch (std::exception& e) {
                logging.out("ERROR", "Api error: " + req.request_info.method + " " + req.request_info.path + " " + req.request_info.media_path + "\nwhat: " + e.what() );
                *s+="[api error]";
            }
        };
    }

    main_data.config_Data = config;
}

void check( sockaddr_in client, ssl_st * ssl, bool secure, std::shared_ptr< epoll_event > event)
{
    requester::request_data info;
    bool access = true;
    auto stop = [ssl, event, secure]() {
        //client_map.clients.erase(client_handle);
        if(secure) {
            SSL_shutdown(ssl); SSL_free(ssl); }
        epoll_ctl(epfd, EPOLL_CTL_DEL, (*event).data.fd, event.get());
        close((*event).data.fd);
    };
    char buffer[2048];
    try {
        (secure) ? SSL_read(ssl, buffer, sizeof(buffer) - 1) : recv((*event).data.fd, &buffer, sizeof(buffer), MSG_PEEK );
        //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, (string) "Connected client, status - " + ((secure) ? (string) "true" : (string) "false"), &client);
        logging.out("LOG", "Connected client: " + (string)inet_ntoa(client.sin_addr) + ":" + to_string(ntohs(client.sin_port)) + ", status - " + ((secure) ? (string) "true" : (string) "false"));
        logging.out("DEBUG", "ID socket = " + to_string((*event).data.fd));
    } catch(std::exception ) {
        stop(); return;
    }
    try{
        if (((string)buffer).length()<= 0) {
            //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, "Detect no data", &client);
            logging.out("WARNING", "Detect no data");
            stop(); return;
        }
        if (  ::requester::get_request(buffer, &info) < 0) {
            logging.out("WARNING", "Incorrect request");
            stop(); return;
        }
        if((string)(info.request_info.method) == "") {
            logging.out("WARNING", "No method code");
            stop(); return;
        }

        memset(&buffer, 0, sizeof(buffer));
        //if(info.request_info.path == "/") { info.request_info.path = ""; }
        logging.out("LOG", (string) "Request: Method - " + (string) (info.request_info.method) +", path - " + (string) (info.request_info.path) + ", media_path - " +(string) (info.request_info.media_path) + ", body - " + (string) (info.body));

        //if (info.headers.contains("Content-Type"))
        //logging.out(log_type::DEBUG,(string) "\t\t---------------\nData info: \n" +(string) "\t\tContent type: " + (string) info.headers["Content-Type"].get<string>());
    }
    catch(std::exception e){
        /*async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, (string) "Error parsing! " +
                                                                                        (string) "\n Try check page: " +
                                                                                        info.request_info.path +
                                                                                        "\n Headers: " +
                                                                                        (string) info.headers, &client);*/
        logging.out("ERROR", (string) "Error parsing! " +
                             (string) "\n Try check page: " +
                             info.request_info.path +
                             "\n Headers: " +
                             (string) info.headers);
        //cout << std::stacktrace::current() << endl;
    }
    string compressed_data;
    try {
        requester::check_path data(path + PAGE_PATH + info.request_info.path);
        //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::LOG,"Trying check by path: " + (string) (data.is_dir() ? data.path + "/main.html" : data.path), &client);
        //logging.out("DEBUG", "Trying check by path: " + (string) (data.is_dir() ? data.path + "/main.html" : data.path));
        responser::response rep(secure);
        auto access_info = content_access.check(info);
        switch(access_info.first) {
            case requester::REQUEST_TO_API:
                logging.out("LOG", "Denied to " + access_info.second);
                rep.code_status = 301;
                rep.header_body += "Location: " + access_info.second + "\r\nStatus: Denied\r\n";
                access = false;
                break;
            case requester::REQUEST_TO_URL:
                logging.out("LOG", "Denied to " + access_info.second);
                rep.code_status = 301;
                rep.header_body += "Location: " + access_info.second + "\r\nStatus: Denied\r\n";
                access = false;
                break;
            case requester::CLOSE_CLIENT:
                stop();
                break;
            case requester::ALLOWED:
                break;
        }
        if(main_data.machine_info) {
            struct utsname unameData;
            uname(&unameData);
            rep.header_body += "Architecture: " + (string) (unameData.machine) + "\r\n"
                     "System: " + (string) (unameData.sysname) + "\r\n"
                     "Version: " + (string) (unameData.version) + "\r\n"
                     "Release: " + (string) (unameData.release) + "\r\n"
                     "Name: nginx\r\n";
        }
        if(access) {
            if(app.is_api(info.request_info.path)){
                try {
                    //logging.out(log_type::DEBUG, "This is an api");
                    app.routes[info.request_info.path](&compressed_data, static_cast<responser::response *>(&rep), info, app.access, static_cast< void* >(&main_data) );
                } catch(const std::exception err){}
            }
            else {
                // compress
                stringstream compressedBuffer;
                boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
                inbuf.push(boost::iostreams::gzip_compressor());
                std::ifstream file((data.is_dir() ? data.path + "/main.html" : data.path), std::ios::binary);
                if(file.good()) {
                    rep.header_body += "Connection: close\r\nContent-Type: " + data.return_content() + "\r\n" +
                                       "Content-Encoding: gzip\r\nAccept-Encoding: gzip, deflate\r\n";
                    inbuf.push(file);
                    boost::iostreams::copy(inbuf, compressedBuffer);
                    compressed_data = compressedBuffer.str(); // оно нахуй работает
                    inbuf.pop();
                    file.close();
                    boost::iostreams::close(inbuf);
                }
                else {
                    rep.code_status = 404;
                }
                // end compress
            }
        }
        rep.body_text += compressed_data;
        //std::cout << rep.make_request() << std::endl;
        (secure) ? SSL_write(ssl,rep.make_request().c_str(), rep.make_request().length()) : send((*event).data.fd, rep.make_request().c_str(), rep.make_request().length(), MSG_NOSIGNAL);
    } catch (const std::exception& ex) {
        //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::ERROR,"Error: " + (string) ex.what() + "\n Try check page: " + info.request_info.path, &client);
        logging.out("ERROR", "Error: " + (string) ex.what() + "\n Try check page: " + info.request_info.path);
        compressed_data = "Error response";
    }
    stop();
    return;
}
void accept_for(std::shared_ptr< epoll_event > event) {
    setsockopt((*event).data.fd, SOL_SOCKET, SO_RCVTIMEO, static_cast<struct timeval *>(&sockettimer), sizeof(struct timeval));
    client_map.async_input((*event).data.fd);
    sockaddr_in client;
    socklen_t len = sizeof(client);
    getsockname((*event).data.fd, (sockaddr *)&client, &len);
    int secure = secure_handle; // add algorithm multi i/o
    SSL * ssl = NULL;
    if(secure_handle) {
        ssl = SSL_new(space());
        if (SSL_set_fd(ssl, (*event).data.fd) <= 0) {
            secure = 0;
            SSL_shutdown(ssl); SSL_free(ssl); //close(client_handle);
            //continue;
        }
        if (SSL_accept(ssl) <= 0 ) {
            secure = 0;
            SSL_shutdown(ssl); SSL_free(ssl); //close(client_handle);
            //continue;
        }

    }

    std::async(std::launch::async, check, client, ssl, secure, event);//check(client, client_handle);
}
int main() {
    sockettimer.tv_sec = 3;
    char pwd[PATH_MAX];
    getcwd(pwd, sizeof(pwd));
    path = (string)pwd;
    logging.config->show_id_thread = false;
    logging.config->wait = 1;
    //logging.allowed_type[log_type::DEBUG] = false;
    logging.config->format_type = "%D-%T";
    read_cfg();

    if (main_data.config_Data["SERVER"]["CONFIG"].contains("MODULES")) {
        if (main_data.config_Data["SERVER"]["CONFIG"]["MODULES"]["ENABLE"].get<bool>()) {
            app.init();
        }
    }
    evlist = vector<struct epoll_event>(nfds);
    if(enable_ssl) {
        space.init();
    }

    memset(&pwd, 0, sizeof(pwd));
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    servAddr.sin_port = htons(*port);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    signal(SIGPIPE, SIG_IGN);
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK)) {
        perror("Cant unlock socket");
        exit(-4);
    }
    if(bind(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Cant bind socket");
        exit(-1);
    }
    logging << "Binded ip address: " + (string)((secure_handle) ? "https://" : "http://") + (string)inet_ntoa(servAddr.sin_addr) + (string)":" + to_string(ntohs(servAddr.sin_port));
    listen(sock, 1000);
    if((epfd = epoll_create1(0)) < 0 ) {
        perror("Error create epoll");
        exit(-4);
    }
    ev.events = EPOLLIN;
    ev.data.fd = sock;
    if ( (ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev)) < 0) {
        perror("Error control epoll");
        exit(-5);
    }

    while(true)
    {
        if(( nfds = epoll_wait(epfd, evlist.data(), save_nfds, timeout_client) ) < 0) continue;
        int i = 0;
        while(i < nfds)
        {
            if(evlist[i].data.fd == sock) {
                sockaddr_in client; // fuck poll
                socklen_t clientSize = sizeof(client);
                int client_handle = accept4(sock, (sockaddr*)&client, &clientSize, 0); //accept(sock, (sockaddr *)&client, &clientSize);
                if(client_handle < 0) { close(client_handle); continue; }
                evlist[i].events = EPOLLOUT | EPOLLET;
                evlist[i].data.fd = client_handle;
                //ev.events = EPOLLOUT | EPOLLET;
                //ev.data.fd = client_handle;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_handle, &ev);
            }
            else {
                std::async(std::launch::async, accept_for, std::make_shared<epoll_event>(evlist[i]) );
            }
            i++;
        }
        usleep(10);
    }
    /*std::mutex mtx;
    auto async_loopper = [&mtx](std::stop_token token){ // multithread loops
        std::stringstream dd; dd << std::this_thread::get_id();
        while(!token.stop_requested())
        {
            try {
                if ((nfds = epoll_wait(epfd, evlist.data(), save_nfds, timeout_client)) < 0) continue;
                logging.out("DEBUG", dd.str() + " getted ndfs=" + to_string(nfds));
                int i = 0;
                while (i < nfds) {
                    if (evlist[i].data.fd == sock) {
                        logging.out("DEBUG", dd.str() + " in checking socket ");
                        sockaddr_in client; // fuck poll
                        socklen_t clientSize = sizeof(client);
                        logging.out("DEBUG", dd.str() + " locking");
                        int client_handle = accept4(sock, (sockaddr *) &client, &clientSize,O_NONBLOCK); //accept(sock, (sockaddr *)&client, &clientSize);
                        logging.out("DEBUG", dd.str() + " getted " + to_string(client_handle) + " socket");
                        logging.out("DEBUG", dd.str() + " unlocking");
                        if (client_handle < 0) {
                            close(client_handle);
                            continue;
                        }
                        ev.events = EPOLLOUT | EPOLLET;
                        ev.data.fd = client_handle;
                        ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_handle, &ev);
                    } else {
                        std::async(std::launch::async, accept_for, std::make_shared<epoll_event>(evlist[i]));
                    }
                    i++;
                }
                usleep(1);
            }
            catch (std::exception & e) {
                logging.out("ERROR", dd.str() + " getted error: '" + e.what() );
            }
        }
    };
    int thread_size = 2;
    vector<jthread> threads ( thread_size );
    for(; thread_size >=0; thread_size-- ) {
        threads.push_back(std::jthread([&async_loopper](std::stop_token token) {async_loopper(token);}));
    }*/
    space.close();
    logging.close();
    return 0;
}
