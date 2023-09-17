#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "coroutine_source.cpp"
#include <iostream>
#include <time.h>
using namespace::std;
#include "log_stream.cpp"
typedef int sock_handle;
#include <sstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <zlib.h>
#include <fstream>
#include <future>
#include "response.cpp"
#include <sys/epoll.h>
#include <memory>
sockaddr_in servAddr;           // address data
sock_handle sock;               // socket handle
std::shared_ptr<unsigned int> port = std::make_shared<unsigned int>(81);                 // port
string ipAddr = "127.0.0.1";    // ip
bool secure_handle = NULL;
bool enable_ssl = false;
int nfds = 2000;
#include "openssl/ssl.h"
#include "ssl.cpp"
#include <fcntl.h>
#include <unistd.h>
//#include <poll.h>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/ssl.hpp>
#include <regex>
using namespace::firewolf;
void check( sockaddr_in client, int client_handle, ssl_st * ssl, bool secure);
api app;
string PAGE_PATH = "/pages";
int timeout_client = -1;
ssl_space::SSL space(false, &secure_handle);
vector<json> proxy_path;

struct main_funcs {
    int * timeoutclient = &timeout_client;
    api * app_ptr = &app;
    string * ip = &ipAddr;
    std::shared_ptr<unsigned int> portptr = port;
    ssl_space::SSL * ssl_info = &space;
    std::shared_ptr<int> active_epols = make_shared<int>(nfds);
    string * pages = &PAGE_PATH;
    json config_Data;
    vector<json> proxy_path;
    //std::shared_ptr<firewolf::streams::logger> output = output;
} main_data;
void read_cfg() {


    ifstream file("config.json", ios::binary);
    std::string line;
    std::string text;
    if (file.is_open()) {
        while (getline(file, line)) {
            text += line + "\n";
        }
        file.close();
    }
    //cout << "LINE PARSE " + text;
    json config = config.parse(text);
    *(main_data.active_epols) = config["SERVER"]["CONFIG"]["EPOLLS_COUNT"].get<int>();
    *(main_data.ip) = config["SERVER"]["CONFIG"]["IP"].get<string>();
    *(main_data.portptr) = config["SERVER"]["CONFIG"]["PORT"].get<uint>();
    *(main_data.pages) = config["SERVER"]["CONFIG"]["PATH_PAGES"].get<string>();
    enable_ssl = config["SERVER"]["SSL"]["ENABLE"].get<bool>();
    main_data.proxy_path = config["PROXY_SERVER"]["PROXY"];
    cout<< "Proxy pathes:" << endl;
    for(json data : main_data.proxy_path) {
        cout << "\t" + data["PATH"].get<string>() + (string)" to " + data["ADRESS"].get<string>() << endl;
        main_data.app_ptr->access[data["PATH"]] = data;
        main_data.app_ptr->routes[data["PATH"]] = [](string * s, responser::response* rep, requester::request_data req, unordered_map<string, json> access_info) {
            //std::async(launch::async, logger::log_chat_async, "SERVER", logger::type_log::DEBUG, "RUNNING PROXY PATH");
            boost::asio::io_context ioContext;
            boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23);
            ssl_context.set_default_verify_paths();
            boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(ioContext, ssl_context);
            boost::asio::ip::tcp::resolver resolver(ioContext);
            std::regex pattern (R"(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})");
            boost::asio::ip::tcp::resolver::query query("NULL", "NULL");
            if(regex_match(access_info[req.request_info.path]["ADRESS"].get<string>(), pattern)) {
                query = boost::asio::ip::tcp::resolver::query(access_info[req.request_info.path]["ADRESS"].get<string>(), access_info[req.request_info.path]["PORT"].get<string>());
            }
            else {
                query = boost::asio::ip::tcp::resolver::query( access_info[req.request_info.path]["ADRESS"].get<string>(), access_info[req.request_info.path]["PROTOCOL"].get<string>() );
            }
            boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            socket.next_layer().open(boost::asio::ip::tcp::v4());
            socket.next_layer().non_blocking(true);
            //boost::asio::deadline_timer timer(ioContext);
            //timer.expires_from_now(boost::posix_time::seconds(6)); // 5 секундное ожидание
            /*timer.async_wait([&socket, endpoint_iterator, &access_info, &req, &s](const boost::system::error_code& error){
                if (!error){

                }
            }) ;

            ioContext.run();*/
            boost::asio::streambuf();
            boost::asio::connect(socket.next_layer(), endpoint_iterator);
            socket.handshake(boost::asio::ssl::stream_base::client);
            boost::asio::streambuf request;
            std::ostream requestStream{&request};
            requestStream << req.request_info.method << " " <<
                          access_info[req.request_info.path]["PROXY_PATH"].get<string>() << (req.request_info.media_path.empty() ? "" : "?" + req.request_info.media_path) << " HTTP/1.1\r\n";
            requestStream << "Host: " << access_info[req.request_info.path]["ADRESS"].get<string>() << "\r\n";
            bool client = access_info[req.request_info.path]["CONFIG_HEADERS"]["USE_CLIENT_HEADERS"].get<bool>();
            for(auto it = access_info[req.request_info.path]["HEADERS"].begin(); it != access_info[req.request_info.path]["HEADERS"].end(); ++it) { // config
                requestStream << it.key() << ": " << it.value() << "\r\n";
                if(client && (req.headers.contains(it.key())) ) {
                    req.headers.erase(it.key());
                }
            }
            if(client) {
                for (auto it = req.headers.begin(); it != req.headers.end(); ++it) { // client
                    auto rem = access_info[req.request_info.path]["CONFIG_HEADERS"]["REMOVE_CLIENT_HEADERS"];
                    if( std::find(rem.begin(), rem.end(), it.key()) != rem.end() ) continue;

                    requestStream << it.key() << ": " << it.value() << "\r\n";
                }
            }
            requestStream << "\r\n";
            if(!req.body.empty()) {
                requestStream << req.body;
            }
            write(socket, request);
            ioContext.run();
            boost::asio::streambuf response;
            //boost::asio::read_until(socket, response, "\r\n");
            boost::system::error_code errorCode;
            boost::asio::read(socket, response, errorCode);
            stringstream out;
            out << (&response);
            string output = out.str();
            //cout << firewolf::requester::get_line_ready(output, "\r\n\r\n") << endl;
            std::size_t pos = output.find('\n');
            if (pos != std::string::npos) {
                output.erase(0, pos+1);
            }
            if(access_info[req.request_info.path]["RESPONSE_CONFIG"]["REMOVE_HEADERS"].get<bool>()) {
                auto array = access_info[req.request_info.path]["RESPONSE_CONFIG"]["REMOVE_HEADERS_NAME"];
                for (auto d = array.begin(); d != array.end(); ++d) {// format to array?
                    output = firewolf::requester::pop_line(output, d.value().get<string>() + ": ", "\r\n");
                }
            }
            rep->header_body += output.substr(0, output.find("\r\n\r\n")+2 );
            *s += firewolf::requester::get_line_ready(output, "\r\n\r\n");
            /*if(!errorCode) {

            }
            else {
                rep->header_body += "Connection: close\r\nContent-Type: text/html\r\n";
                *s += "[Error api]";
            }*/
        };
    }

    main_data.config_Data = config;
}
string path;
/*----epoll----*/
int save_nfds = nfds;
int epfd;
int ret;
struct epoll_event ev;
struct epoll_event * evlist; //[NFDS];
void close_epolls() {
    for (int i = 0; i < save_nfds; i++)
    {
        close(evlist[i].data.fd);
    }
}
/*----epoll----*/

void accept_for(int client_handle) {
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
            async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::WARNING, "Nonsecure socket!",&client);
            //cout << "nonsecure\n";
            secure = 0;
            SSL_shutdown(ssl); SSL_free(ssl); //close(client_handle);
            //continue;
        }

    }
    std::async(launch::async, check, client, client_handle, ssl, secure);//check(client, client_handle);
}

int main(int len, char * argv[]) {
    firewolf::streams::logger output;
    output.config->show_id_thread = true;
    cout << "THREAD: " << this_thread::get_id << endl;
    output.config->wait = chrono::milliseconds(200);
    read_cfg();
    evlist = new epoll_event[nfds];
    if(enable_ssl) {
        space.init();
    }
    char pwd[PATH_MAX];
    getcwd(pwd, sizeof(pwd));
    path = (string)pwd;
    //ipAddr = argv[1];
    //port = ::atoi(argv[2]);
    cout << "Hello server!\n";
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

    cout << "Binded ip address: " + (string)((secure_handle) ? "https://" : "http://") + (string)inet_ntoa(servAddr.sin_addr) + (string)":" + to_string(ntohs(servAddr.sin_port)) << endl;
    listen(sock, 1000);
    //auto accept_for = [] (int client_handle, sockaddr_in client) { // возможно не спасет, утечка
    /*auto accept_for = [] (int client_handle) { // возможно не спасет, утечка
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
                async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::WARNING, "Nonsecure socket!",&client);
                //cout << "nonsecure\n";
                secure = 0;
                SSL_shutdown(ssl); SSL_free(ssl); //close(client_handle);
                //continue;
            }

        }
        std::async(launch::async, check, client, client_handle, ssl, secure);//check(client, client_handle);
    };*/

    /*struct pollfd fds[nfds];
    for(int i = 0; i < nfds/2; i++) {
        fds[i].fd = sock;
        fds[i].events = POLLIN;
    }*/
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
                int client_handle = accept4(sock, (sockaddr*)&client, &clientSize, SOCK_NONBLOCK); //accept(sock, (sockaddr *)&client, &clientSize);
                if(client_handle < 0) { close(client_handle); continue; }
                ev.events = EPOLLOUT | EPOLLET;
                ev.data.fd = client_handle;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_handle, &ev);
            }
            else {
                int cli = evlist[i].data.fd;
                std::async(launch::async, accept_for, cli);
            }
            i++;
        }
        usleep(200);
    }
    space.close();
    return 0;
}



#include <fstream>
#include <vector>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <sys/utsname.h>

void check( sockaddr_in client, int client_handle, ssl_st * ssl, bool secure)
{
    ::requester::request_data info;
    auto stop = [ssl, client_handle, secure]() {
        if(secure) {
        SSL_shutdown(ssl); SSL_free(ssl); } close(client_handle);
    };
    //cout << "Coroutine started on thread: " << this_thread::get_id() << '\n';
    //jthread out;
    //co_await switch_to_new_thread(out);
    //out.detach();
    char buffer[2048];
    try {
        (secure) ? SSL_read(ssl, buffer, sizeof(buffer) - 1) : recv(client_handle, &buffer, sizeof(buffer), 0);
        async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, (string) "Connected client, status - " + ((secure) ? (string) "true" : (string) "false"), &client);
    } catch(exception ) {
        stop(); return;
    }
    try{
        if (std::strlen(buffer) <= 0) {
            async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, "Detect no data", &client);
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

        async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG,
              (string) "Request data: Method - " + (string) (info.request_info.method) +
              ", path - " + (string) (info.request_info.path) + ", media_path - " +
              (string) (info.request_info.media_path) + ", body - " + (string) (info.body), &client);
        if (info.headers.contains("Content-Type"))
            async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG,
                  (string) "\t\t---------------\nData info: \n" +
                  (string) "\t\tContent type: " + (string) info.headers["Content-Type"].get<string>(), &client);
    }
    catch(std::exception e){
        async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, (string) "Error parsing! " +
                                                                                        (string) "\n Try check page: " +
                                                                                        info.request_info.path +
                                                                                        "\n Headers: " +
                                                                                        (string) info.headers, &client);
    }
    string compressed_data;
    try {
        requester::check_path data(path + PAGE_PATH + info.request_info.path);
        async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::LOG,"Trying check by path: " + (string) (data.is_dir() ? data.path + "/main.html" : data.path), &client);
        responser::response rep;
        if(main_data.config_Data["SERVER"]["CONFIG"]["MACHINE_INFO"].get<bool>()) {
            struct utsname unameData;
            uname(&unameData);
            rep.header_body += "Architecture: " + (string) (unameData.machine) + "\r\n"
                               "System: " + (string) (unameData.sysname) + "\r\n"
                               "Version: " + (string) (unameData.version) + "\r\n"
                               "Release: " + (string) (unameData.release) + "\r\n"
                               "Name: C++ linux server\r\n";
        }
        int proxy_ind = -1;
        if(app.is_api(info.request_info.path)){
            try {
                async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, "This is a api",&client);
            } catch(const exception err){}
            app.routes[info.request_info.path](&compressed_data, &rep, info, app.access);
        }
        else {
            rep.header_body += "Connection: close\r\nContent-Type: " + data.return_content() + "\r\n";
            async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::DEBUG, "This is a resource",&client);
            rep.header_body += "Content-Encoding: gzip\r\nAccept-Encoding: gzip, deflate\r\n";
            // compress
            stringstream compressedBuffer;
            boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
            inbuf.push(boost::iostreams::gzip_compressor());
            ifstream file((data.is_dir() ? data.path + "/main.html" : data.path), ios::binary);
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
        async(launch::async, logger::log_chat_async, "CLIENT", logger::type_log::ERROR,
              "Error: " + (string) ex.what() + "\n Try check page: " + info.request_info.path, &client);
        compressed_data = "Error response";
    }
    //sleep(1);
    //write(client_handle, &message, strlen(timer));
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