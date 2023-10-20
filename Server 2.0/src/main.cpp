

//
//
//
//
//  Created by
//  ███████╗██╗██████╗ ███████╗██╗    ██╗ ██████╗ ██╗     ███████╗██████╗  ██████╗ ██╗  ██╗
//  ██╔════╝██║██╔══██╗██╔════╝██║    ██║██╔═══██╗██║     ██╔════╝╚════██╗██╔═████╗██║  ██║
//  █████╗  ██║██████╔╝█████╗  ██║ █╗ ██║██║   ██║██║     █████╗   █████╔╝██║██╔██║███████║
//  ██╔══╝  ██║██╔══██╗██╔══╝  ██║███╗██║██║   ██║██║     ██╔══╝   ╚═══██╗████╔╝██║╚════██║
//  ██║     ██║██║  ██║███████╗╚███╔███╔╝╚██████╔╝███████╗██║     ██████╔╝╚██████╔╝     ██║
//  ╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝ ╚══╝╚══╝  ╚═════╝ ╚══════╝╚═╝     ╚═════╝  ╚═════╝      ╚═╝
//
//
//
//
//

#include "variables.h"

void read_cfg() {
    std::ifstream file("config.json", std::ios::binary);
    std::string line;
    std::string text;
    if (file.is_open()) {
        while (getline(file, line)) {
            text += line + "\n";
        }
        file.close();
    }
    //cout << "LINE PARSE " + text;
    nlohmann::json config = config.parse(text);
    *(main_data.active_epols) = config["SERVER"]["CONFIG"]["EPOLLS_COUNT"].get<int>();
    *(main_data.ip) = config["SERVER"]["CONFIG"]["IP"].get<string>();
    *(main_data.portptr) = config["SERVER"]["CONFIG"]["PORT"].get<uint>();
    *(main_data.pages) = config["SERVER"]["CONFIG"]["PATH_PAGES"].get<string>();
    enable_ssl = config["SERVER"]["SSL"]["ENABLE"].get<bool>();
    main_data.machine_info = config["SERVER"]["CONFIG"]["MACHINE_INFO"].get<bool>();
    main_data.proxy_path = config["PROXY_SERVER"]["PROXY"];
    try {
        main_data.server->configure(config["SERVER"]["SQL"]["CONFIG"]);
    }catch (const std::exception& e) {
        cout << "Error connect to SQL: " << e.what() << endl;
    }
    cout<< "Proxy pathes:" << endl;
    for(json data : main_data.proxy_path) {
        cout << "\t" + data["PATH"].get<string>() + (string)" to " + data["ADRESS"].get<string>() << endl;
        main_data.app_ptr->access[data["PATH"]] = data;
        main_data.app_ptr->routes[data["PATH"]] = [](std::string * s, responser::response* rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void*funcs) {
            //std::async(launch::async, logger::log_chat_async, "SERVER", logger::type_log::DEBUG, "RUNNING PROXY PATH");
            try {
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
                    CURLcode res = curl_easy_perform(curl); // run request
                    if (res != CURLE_OK) {
                        char error[256];
                        ERR_error_string(res, static_cast<char *>(error));
                        throw std::runtime_error("CURL NOT OK: " + (string)error );
                        delete[] &error;
                    }
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

void check( sockaddr_in client, int client_handle, ssl_st * ssl, bool secure);
void accept_for(int client_handle) {
    client_map.async_input(client_handle);
    sockaddr_in client;
    socklen_t len = sizeof(client);
    getsockname(client_handle, (sockaddr *)&client, &len);
    int secure = secure_handle; // add algorithm multi i/o
    SSL * ssl = NULL;
    if(secure_handle) {
        ssl = SSL_new(space());
        if (SSL_set_fd(ssl, client_handle) <= 0) {
            secure = 0;
            SSL_shutdown(ssl); SSL_free(ssl); //close(client_handle);
            //continue;
        }
        if (SSL_accept(ssl) <= 0 ) {
            //async(std::launch::async, logger::log_chat_async, "CLIENT", logger::type_log::WARNING, "Nonsecure socket!",&client);
            //logging[firewolf::streams::logger::WARNING] << "Nonsecure socket!";
            //cout << "nonsecure\n";
            secure = 0;
            SSL_shutdown(ssl); SSL_free(ssl); //close(client_handle);
            //continue;
        }

    }

    std::async(std::launch::async, check, client, client_handle, ssl, secure);//check(client, client_handle);
}
int main(int argc, char* argv[]) {
    logging.config->show_id_thread = false;
    logging.config->wait = 1;
    //logging.allowed_type[log_type::DEBUG] = false;
    logging.config->format_type = "%D-%T";
    read_cfg();
    evlist = new epoll_event[nfds];
    if(enable_ssl) {
        space.init();
    }
    //char pwd[PATH_MAX];
    //getcwd(pwd, sizeof(pwd));
    path = (string)argv[0];
    //memset(&pwd, 0, sizeof(pwd));
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
        nfds = epoll_wait(epfd, evlist, save_nfds, timeout_client);
        int i = 0;
        while(i < nfds)
        {
            if(evlist[i].data.fd == sock) {
                sockaddr_in client; // fuck poll
                socklen_t clientSize = sizeof(client);
                int client_handle = accept4(sock, (sockaddr*)&client, &clientSize, 0); //accept(sock, (sockaddr *)&client, &clientSize);
                if(client_handle < 0) { close(client_handle); continue; }
                ev.events = EPOLLOUT | EPOLLET;
                ev.data.fd = client_handle;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_handle, &ev);
            }
            else {

                std::async(std::launch::async, accept_for, (int)evlist[i].data.fd);
            }
            i++;
        }
        usleep(10);
    }
    space.close();
    logging.close();
    return 0;
}

void check( sockaddr_in client, int client_handle, ssl_st * ssl, bool secure)
{
    ::requester::request_data info;
    auto stop = [ssl, client_handle, secure]() {
        client_map.clients.erase(client_handle);
        if(secure) {
            SSL_shutdown(ssl); SSL_free(ssl); }
        close(client_handle);
    };
    //cout << "Coroutine started on thread: " << this_thread::get_id() << '\n';
    //jthread out;
    //co_await switch_to_new_thread(out);
    //out.detach();
    char buffer[2048];
    try {
        (secure) ? SSL_read(ssl, buffer, sizeof(buffer) - 1) : recv(client_handle, &buffer, sizeof(buffer), MSG_DONTWAIT );
        //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, (string) "Connected client, status - " + ((secure) ? (string) "true" : (string) "false"), &client);
        logging.out("LOG", "Connected client: " + (string)inet_ntoa(client.sin_addr) + ":" + to_string(ntohs(client.sin_port)) + ", status - " + ((secure) ? (string) "true" : (string) "false"));
        logging.out("DEBUG", "ID socket = " + to_string(client_handle));
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
            stop(); return;
        }
        if((string)(info.request_info.method) == "") {

            stop(); return;

        }
        memset(&buffer, 0, sizeof(buffer));
        if(info.request_info.path == "/") { info.request_info.path = ""; }
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
    }
    string compressed_data;
    try {
        requester::check_path data(path + PAGE_PATH + info.request_info.path);
        //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::LOG,"Trying check by path: " + (string) (data.is_dir() ? data.path + "/main.html" : data.path), &client);
        //logging.out("DEBUG", "Trying check by path: " + (string) (data.is_dir() ? data.path + "/main.html" : data.path));
        responser::response rep;
        if(main_data.machine_info) {
            struct utsname unameData;
            uname(&unameData);
            rep.header_body += "Architecture: " + (string) (unameData.machine) + "\r\n"
                               "System: " + (string) (unameData.sysname) + "\r\n"
                               "Version: " + (string) (unameData.version) + "\r\n"
                               "Release: " + (string) (unameData.release) + "\r\n"
                               "Name: nginx\r\n";
        }
        if(app.is_api(info.request_info.path)){
            try {
                //logging.out(log_type::DEBUG, "This is an api");
                app.routes[info.request_info.path](&compressed_data, &rep, info, app.access, static_cast< main_funcs* >(&main_data) );
            } catch(const std::exception err){}
        }
        else {
            rep.header_body += "Connection: close\r\nContent-Type: " + data.return_content() + "\r\n";
            //logging.out(log_type::DEBUG, "This is a resource");
            //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, "This is a resource",&client);
            rep.header_body += "Content-Encoding: gzip\r\nAccept-Encoding: gzip, deflate\r\n";
            // compress
            stringstream compressedBuffer;
            boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
            inbuf.push(boost::iostreams::gzip_compressor());
            std::ifstream file((data.is_dir() ? data.path + "/main.html" : data.path), std::ios::binary);
            inbuf.push(file);
            boost::iostreams::copy(inbuf, compressedBuffer);
            compressed_data = compressedBuffer.str(); // оно нахуй работает
            inbuf.pop();
            file.close();
            boost::iostreams::close(inbuf);
            // compress
        }
        rep.body_text += compressed_data;

        (secure) ? SSL_write(ssl,rep.make_request().c_str(), rep.make_request().length()) : send(client_handle, rep.make_request().c_str(), rep.make_request().length(), MSG_NOSIGNAL);
    } catch (const std::exception& ex) {
        //async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::ERROR,"Error: " + (string) ex.what() + "\n Try check page: " + info.request_info.path, &client);
        logging.out("ERROR", "Error: " + (string) ex.what() + "\n Try check page: " + info.request_info.path);
        compressed_data = "Error response";
    }
    //sleep(1);
    //write(client_handle, &message, strlen(timer));
    //logging.out("DEBUG", "count: " + to_string(client_map.clients.size()));
    stop();
    return;
    /*if(out.joinable()) {
        this_thread::sleep_for(chrono::seconds(1));
        out.request_stop();
        out.detach();
        out.join();
        //delete out;
    }*/
}