//
// Created by firewolf304 on 10.02.24.
//

#ifndef SERVER_3_0_WEB_SERVER_HPP
#define SERVER_3_0_WEB_SERVER_HPP
#include "../include/variables.hpp" // вызывать где надо
#include <liburing/io_service.hpp>
#include <utility>
namespace firewolf::web_server {
    class web_server {
    public:
        // ==============basic server==============
        std::string ipAddr = "127.0.0.1";                                                           // ip
        std::shared_ptr<unsigned int> port = std::make_shared<unsigned int>(81);                    // port
        string PAGE_PATH = "/pages";                                                                // front content
        firewolf::streams::logger logging;                                                          // logging system
        bool enable_ssl = false;                                                                    // SSL enabler
        std::string config_file_name = "config.json";
        std::string access_file_name = "access.json";
        // ==============multiplexer==============
        std::shared_ptr<uio::io_service> service;
        //vector<struct epoll_event> evlist; //[NFDS];
        //int ret;
        //int epfd;
        // ==============basic client==============
        int timeout_socket = 3000; //milisec
        int timeout_client = -1;
        int nfds = 20000;                                                                           // multiplexer size
        fd_set readfds;
        // ==============local==============
        sockaddr_in servAddr;                                                                       // address data
        sock_handle sock;                                                                           // socket handle
        api app;                                                                                    // dynamic apis
        struct timeval sockettimer;
        //struct epoll_event ev;
        int save_nfds = this->nfds;
        string path;                                                                         // server file location in system
        // ==============monitoring==============
        firewolf::requester::access_response content_access;
        firewolf::monitor::collect_connection client_map;
        // ==============SSL==============
        firewolf::ssl_space::SSL space;                                                             // secure constructor
        bool secure_handle = NULL;                                                                  // secure status pointer
        // ==============SQL==============
        sql::sql_dump sqller_postgres;

        // ==============global==============
        main_funcs main_data = { std::make_shared<std::string>(path), &logging, &timeout_client, &app,
                                 &ipAddr, port, &space, &PAGE_PATH,
                                 NULL, false, {}, {}, &sqller_postgres,
                                 std::make_shared<sock_handle>(sock), &client_map, //&module,
                                 std::make_shared<firewolf::requester::access_response>(content_access)
        };
        /*void epoll_add(int fd, uint32_t events) {
            struct epoll_event ev;
            ev.events = events;
            ev.data.fd = fd;
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
                logging.out("ERROR", "Error epoll add");
            }
        }*/
        web_server() {}
        ~web_server() {
            //this->close_epolls();
            space.close();
            logging.close();
        }
        web_server(std::string const & config_name) : config_file_name(std::move(config_name)){
            signal(SIGPIPE, SIG_IGN);
            this->space = firewolf::ssl_space::SSL(false, &secure_handle);
            sockettimer.tv_sec = (int)(timeout_socket / 1000);
            char pwd[PATH_MAX];
            getcwd(pwd, sizeof(pwd));
            this->path = (string)pwd;
            this->logging.config->show_id_thread = false;
            this->logging.config->wait = 1;
            //logging.allowed_type[log_type::DEBUG] = false;
            this->logging.config->format_type = "%D-%T";
            read_cfg();
            if (this->main_data.config_Data["SERVER"]["CONFIG"].contains("MODULES")) {
                if (this->main_data.config_Data["SERVER"]["CONFIG"]["MODULES"]["ENABLE"].get<bool>()) {
                    this->app.init();
                }
            }
            //this->evlist = vector<struct epoll_event>(nfds);
            if(this->enable_ssl) {
                this->space.init();
            }

            memset(&pwd, 0, sizeof(pwd));
            memset(&this->servAddr, 0, sizeof(this->servAddr));
            this->servAddr.sin_family = AF_INET;
            this->servAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
            this->servAddr.sin_port = htons(*this->port);
            this->sock = socket(AF_INET, SOCK_STREAM, 0);
            if (fcntl(this->sock, F_SETFL, fcntl(this->sock, F_GETFL) | O_NONBLOCK)) {
                perror("Cant unlock socket");
                exit(-4);
            }
            if(bind(this->sock, (struct sockaddr*)&this->servAddr, sizeof(this->servAddr)) < 0)
            {
                perror("Cant bind socket");
                exit(-1);
            }
            FD_ZERO(&readfds);
            FD_SET(this->sock, &readfds);
            this->logging << "Binded ip address: " + (string)((this->secure_handle) ? "https://" : "http://") + (string)inet_ntoa(this->servAddr.sin_addr) + (string)":" + to_string(ntohs(this->servAddr.sin_port));
        }
        void listenning() {
            listen(this->sock, 1000);
            /*if((epfd = epoll_create1(0)) < 0 ) {
                perror("Error create epoll");
                exit(-4);
            }*/
            //epoll_add(sock, EPOLLIN);
            //ev.events = EPOLLIN;
            //ev.data.fd = sock;
            //if ( (ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev)) < 0) {
            //    perror("Error control epoll");
            //    exit(-5);
            //}
            auto check = [this]( sockaddr_in client, ssl_st * ssl, bool secure, int clientfd) -> uio::task<void> {

                requester::request_data info;
                bool access = true;
                auto stop = [ssl, clientfd, secure, this]() {
                    //client_map.clients.erase(client_handle);
                    if(secure) {
                        SSL_shutdown(ssl); SSL_free(ssl); }
                    //epoll_ctl(epfd, EPOLL_CTL_DEL, (*event).data.fd, event.get());
                    close(clientfd);
                };
                int flags = MSG_PEEK;
                auto check_nonblock = [&flags, clientfd](){
                    int bytes;
                    int result = ioctl(clientfd, FIONREAD, &bytes);
                    if(result == -1) {
                        flags |= MSG_DONTWAIT;
                    } else {
                        if(result <= 0) {
                            flags |= MSG_DONTWAIT;
                        }
                    }
                };

                int const max_buffer = 2048;
                char buffer[max_buffer];
                memset(&buffer, 0, sizeof(buffer));


                bool eof = false;
                ssize_t total = 0;
                try {
                    check_nonblock();
                    //ssize_t return_code =  recvmsg(clientfd, &msg, flags);
                    if(!secure) {
                        msghdr msg = {nullptr};
                        iovec iov[1];
                        iov[0].iov_base = buffer;
                        iov[0].iov_len = sizeof(buffer);
                        msg.msg_iov = iov;
                        msg.msg_iovlen = 1;
                        timeval timeout{};
                        timeout.tv_sec = 0;
                        timeout.tv_usec = timeout_client;
                        int tryes = 0;
                        logging.out("DEBUG", "FROM ERRNO: " + std::to_string(errno));
                        int maxtry = 5;
                        while (total < max_buffer && maxtry >= 0) {
                            ssize_t n = recvmsg(clientfd, &msg, flags);
                            logging.out("DEBUG", "TO ERRNO: " + std::to_string(errno));
                            if (n > 0) {
                                total += n;
                            } else if (n == 0) {
                                eof = true;
                            } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                                tryes++;
                                int ready = select(clientfd, &readfds, nullptr, nullptr, &timeout);
                                if (ready < 0) {
                                    logging.out("DEBUG", "No timeout");
                                }
                                maxtry--;
                            } else {
                                break;
                            }
                        }
                    } else {
                        total += SSL_read(ssl, buffer, sizeof(buffer) - 1);
                    }
                    /*if(nonblock ) {
                        logging.out("DEBUG", "NONBLOCK");
                        return_code = (secure) ? SSL_read(ssl, buffer, sizeof(buffer) - 1) : recvmsg(clientfd, &msg, MSG_PEEK | MSG_DONTWAIT);
                    }
                    else {
                        logging.out("DEBUG", "BLOCK");
                        return_code = (secure) ? SSL_read(ssl, buffer, sizeof(buffer) - 1) : recvmsg(clientfd, &msg, MSG_PEEK );
                    }*/
                    //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, (string) "Connected client, status - " + ((secure) ? (string) "true" : (string) "false"), &client);
                    logging.out("LOG", "Connected client: " + (string)inet_ntoa(client.sin_addr) + ":" + to_string(ntohs(client.sin_port)) + ", status - " + ((secure) ? (string) "true" : (string) "false"));
                    logging.out("DEBUG", "ID socket = " + to_string(clientfd));
                    /*if(errno == EAGAIN) {
                        logging.out("WARNING", "Need again: " + to_string(errno));
                        return_code = recvmsg(clientfd, &msg, MSG_PEEK | MSG_NOSIGNAL);
                    }*/
                } catch(const std::exception & e ) {
                    stop(); co_return;
                }
                try{

                    if (eof || total <= 0) {
                        logging.out("WARNING", "Detect no data");
                        stop(); co_return;
                    }
                    if (  ::requester::get_request(buffer, &info) < 0) {
                        logging.out("WARNING", "Incorrect request");
                        stop(); co_return;
                    }
                    if(std::string(info.request_info.method).empty()) {
                        logging.out("WARNING", "No method co    de");
                        stop(); co_return;
                    }

                    memset(&buffer, 0, sizeof(buffer));
                    //if(info.request_info.path == "/") { info.request_info.path = ""; }
                    logging.out("LOG", std::string("Request: Method - ") + info.request_info.method + ", path - " +  std::string(info.request_info.path) + ", media_path - " + std::string(info.request_info.media_path) + ", body - " + std::string(info.body));
                    //if (info.headers.contains("Content-Type"))
                    //logging.out(log_type::DEBUG,(string) "\t\t---------------\nData info: \n" +(string) "\t\tContent type: " + (string) info.headers["Content-Type"].get<string>());
                }
                catch(std::exception const & e){
                    /*async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, (string) "Error parsing! " +
                                                                                                    (string) "\n Try check page: " +
                                                                                                    info.request_info.path +
                                                                                                    "\n Headers: " +
                                                                                                    (string) info.headers, &client);*/
                    logging.out("ERROR", std::string("Error parsing!\n Try check page: ") + info.request_info.path + "\n Headers: " +  info.headers.dump());
                    //cout << std::stacktrace::current() << endl;
                }
                string compressed_data;
                try {
                    std::string path_save = path + PAGE_PATH + info.request_info.path;
                    requester::check_path data(path_save);
                    //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::LOG,"Trying check by path: " + (string) (data.is_dir() ? data.path + "/main.html" : data.path), &client);
                    //logging.out("DEBUG", "Trying check by path: " + (string) (data.is_dir() ? data.path + "/main.html" : data.path));
                    responser::response rep(secure);
                    auto access_info = content_access.check(info);
                    switch(access_info.first) {
                        case requester::REQUEST_TO_API:
                            logging.out("LOG", "Denied to " + access_info.second);
                            rep.code_status = 308;
                            rep.header_body += "Location: " + access_info.second + "\r\nStatus: Denied\r\n";
                            access = false;
                            break;
                        case requester::REQUEST_TO_URL:
                            logging.out("LOG", "Denied to " + access_info.second);
                            rep.code_status = 308;
                            rep.header_body += "Location: " + access_info.second + "\r\nStatus: Denied\r\n";
                            access = false;
                            break;
                        case requester::CLOSE_CLIENT:
                            stop();
                            co_return;
                            break;
                        case requester::ALLOWED:
                            break;
                    }
                    if(main_data.machine_info) {
                        utsname unameData;
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
                                app.routes[info.request_info.path](&compressed_data, static_cast<responser::response *>(&rep), info, app.access, static_cast< void* >(&main_data) );
                            } catch(const std::exception & err){}
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
                    rep.body_text = rep.make_request();
                    if(!secure) {
                        msghdr msg = {nullptr};
                        iovec iov[1];
                        iov[0].iov_base = const_cast<char*>(rep.body_text.c_str());
                        iov[0].iov_len = rep.body_text.size();
                        memset(&msg,0,sizeof(msg) );
                        msg.msg_iov = iov;
                        msg.msg_iovlen = 1;
                        sendmsg(clientfd, &msg, MSG_NOSIGNAL );
                    } else {
                        SSL_write(ssl,rep.make_request().c_str(), rep.make_request().length());;
                    }

                    //(secure) ? SSL_write(ssl,rep.make_request().c_str(), rep.make_request().length()) : sendmsg(clientfd, &msg, MSG_NOSIGNAL ); //send((*event).data.fd, rep.make_request().c_str(), rep.make_request().length(), MSG_NOSIGNAL);
                } catch (const std::exception& ex) {
                    //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::ERROR,"Error: " + (string) ex.what() + "\n Try check page: " + info.request_info.path, &client);
                    logging.out("ERROR", "Error: " + (string) ex.what() + "\n Try check page: " + info.request_info.path);
                    compressed_data = "Error response";
                }
                stop();
                co_return;
            };
            auto accept_for = [this, check](int clientfd) -> uio::task<void> {
                uio::on_scope_exit closefd([&]() { service->close(clientfd); });
                setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, static_cast<timeval *>(&sockettimer), sizeof(timeval));
                client_map.async_input(clientfd);
                sockaddr_in client;
                socklen_t len = sizeof(client);
                getsockname(clientfd, (sockaddr *)&client, &len);
                int secure = secure_handle; // add algorithm multi i/o
                SSL * ssl = NULL;
                if(secure_handle) {
                    ssl = SSL_new(space());
                    if (SSL_set_fd(ssl, clientfd) <= 0) {
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
                //void (*check)( sockaddr_in, ssl_st *, bool, std::shared_ptr< epoll_event > );
                //std::async(std::launch::async, check, client, ssl, secure, clientfd);//check(client, client_handle);
                co_await check(client, ssl, secure, clientfd);
                co_return;
            };
            this->service = std::make_shared<uio::io_service>(10000);
            service->run([](std::shared_ptr<uio::io_service> & service, int & socket, auto & accept_for) noexcept -> uio::task<void> {
                             while (true) {
                                 int clientfd = 0;
                                 while ((clientfd = co_await service->accept(socket, nullptr, nullptr)) <= 0);
                                 if(clientfd > socket) {
                                     auto func_async = [&](int client) -> uio::task<void> {
                                         fcntl(client, F_SETFL, fcntl(client, F_GETFL) | O_NONBLOCK);
                                         co_await accept_for(clientfd);
                                     }(clientfd);
                                 }
                                 co_await service->shutdown(clientfd, SHUT_RDWR);
                                 co_await service->close(clientfd);
                             }
                         }(this->service, this->sock, accept_for)
            );
            /*while(true)
            {
                if(( this->nfds = epoll_wait(this->epfd, this->evlist.data(), this->save_nfds, this->timeout_client) ) < 0) continue;
                int i = 0;
                while(i < nfds)
                {
                    if(evlist[i].data.fd == sock) {
                        sockaddr_in client; // fuck poll
                        socklen_t clientSize = sizeof(client);
                        int client_handle = accept4(sock, (sockaddr*)&client, &clientSize, 0); //accept(sock, (sockaddr *)&client, &clientSize);
                        if(client_handle < 0) { close(client_handle); continue; }
                        //evlist[i].events = EPOLLOUT | EPOLLET;
                        //evlist[i].data.fd = client_handle;
                        epoll_add(client_handle, EPOLLOUT | EPOLLET);
                        fcntl(client_handle, F_SETFL, fcntl(client_handle, F_GETFL) | O_NONBLOCK);
                        //ev.events = EPOLLOUT | EPOLLET;
                        //ev.data.fd = client_handle;
                        //ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_handle, &ev);
                    }
                    else {
                        auto async = std::async(std::launch::async, accept_for, std::make_shared<epoll_event>(evlist[i]) );
                    }
                    i++;
                }
                usleep(10);
            }*/
        }
        /*void close_epolls() {
            for (int i = 0; i < save_nfds; i++)
            {
                close(this->evlist[i].data.fd);
            }
        }*/
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
            std::ifstream file_cfg(this->config_file_name, std::ios::binary);
            std::string text_cfg = open_file(file_cfg);
            file_cfg.close();
            std::ifstream file_access(this->access_file_name, std::ios::binary);
            if(file_access.good()) {
                std::string text_access = open_file(file_access);
                nlohmann::json config_access = nlohmann::json::parse(text_access);
                content_access = firewolf::requester::access_response( config_access );
            }
            else {
                content_access = firewolf::requester::access_response( nlohmann::json({}) );
            }
            file_access.close();
            //cout << "LINE PARSE " + text_cfg;
            nlohmann::json config = nlohmann::json::parse(text_cfg);
            if(config["SERVER"].contains("CONFIG")) {
                //*(this->main_data.active_epols) = config["SERVER"]["CONFIG"]["EPOLLS_COUNT"].get<int>();
                *(this->main_data.ip) = config["SERVER"]["CONFIG"]["IP"].get<string>();
                *(this->main_data.portptr) =
                        config["SERVER"]["CONFIG"]["PORT"].get<uint>();
                *(this->main_data.pages) = config["SERVER"]["CONFIG"]["PATH_PAGES"].get<string>();
                this->main_data.machine_info = config["SERVER"]["CONFIG"]["MACHINE_INFO"].get<bool>();
                /*if(config["SERVER"]["CONFIG"].contains("MODS")) {
                    if(config["SERVER"]["CONFIG"]["MODS"]["ENABLE"].get<bool>()) {
                        module.load();
                    }
                }*/
            }
            if(config["SERVER"].contains("SSL") == true) {
                this->enable_ssl = config["SERVER"]["SSL"]["ENABLE"].get<bool>();
            }
            if(config.contains("PROXY_SERVER")) {
                this->main_data.proxy_path = config["PROXY_SERVER"]["PROXY"];
            }
            try {
                if(config["SERVER"].contains("SQL")) {
                    this->main_data.server->configure(config["SERVER"]["SQL"]["CONFIG"]);
                }
            } catch (const std::exception& e) {
                cout << "Error connect to SQL: " << e.what() << endl;
            }
            cout<< "------------------\nProxy pathes:" << endl;
            for(json data : main_data.proxy_path) {
                cout << "\t" + data["PATH"].get<string>() + (string)" to " + data["ADRESS"].get<string>() << endl;
                this->main_data.app_ptr->access[data["PATH"]] = data;
                this->main_data.app_ptr->routes[data["PATH"]] = [&](std::string * s, responser::response* rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void*funcs) {
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

                                }},
                                {"move", [&s, &rep, &req, &access_info](){
                                    std::regex pattern(R"(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})");
                                    rep->code_status = 301;
                                    rep->header_body += "Location: " + access_info[req.request_info.path]["PROTOCOL"].get<string>() + "://" +
                                                                       access_info[req.request_info.path]["ADRESS"].get<string>() +
                                                                       ( (regex_match(access_info[req.request_info.path]["ADRESS"].get<string>(), pattern)) ?
                                                                         ":" + access_info[req.request_info.path]["PORT"].get<string>() : "" ) +
                                                                       access_info[req.request_info.path]["PROXY_PATH"].get<string>() + "\r\n";
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

            this->main_data.config_Data = config;
        }


    };

}



#endif //SERVER_3_0_WEB_SERVER_HPP
