//
// Created by firewolf304 on 02.11.23.
//

#ifndef MODULES_LOG_STREAM_H
#define MODULES_LOG_STREAM_H

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <future>

namespace firewolf::streams {
    class log_stream {
    public:
        /*======config======*/
        struct d {
            bool show_id_thread = false;
            bool show_time = true;
            std::string format_type = "%a %b %d %T %Y";
            //std::chrono::duration<int64_t, std::micro> wait = std::chrono::microseconds (1); // microseconds value
            int wait = 1;
        } config;
        std::jthread thread;
        log_stream();
        log_stream( const log_stream & streamer) {}
        ~log_stream();
        void async_loop(std::stop_token token);
        void init();
        void stop();
        static void write_async(log_stream * logger, const std::string & text );
        log_stream* operator=(const log_stream* rhs);
        log_stream* operator=(const log_stream rhs);
        void operator<<(const std::string & input);
    private:

        std::stringstream stream;
        std::mutex mtx;
        bool active = false;
    };
    class logger {
    private:
        log_stream stream;
    public:
        std::unordered_map<std::string, bool> allowed_type = {
                {"LOG", true},
                {"WARNING", true},
                {"ERROR", true},
                {"MESSAGE", true},
                {"DEBUG", true}
        };
        logger( const logger & log) {}
        logger();
        ~logger();
        std::string get_id_thread();
        logger set(std::string type);
        std::string value_type = "";
        logger operator[] (const std::string value);
        void operator<< (const std::string & text );
        void operator<=(const std::string & text);
        void out(std::string type, std::string text);
        void out(std::string text);
        void close ();
        log_stream::d * config = &this->stream.config;

    };
}
namespace firewolf::logger{
    enum class type_log : int{
        LOG = 0,
        DEBUG = 1,
        MESSAGE = 2,
        WARNING = 3,
        ERROR = 4,
        TEST = 5
    };
    inline char * check_type(type_log type)
    {
        switch (type) {
            case type_log::LOG:
                return "LOG";
                break;
            case type_log::DEBUG:
                return "DEBUG";
                break;
            case type_log::MESSAGE:
                return "MESSAGE";
                break;
            case type_log::WARNING:
                return "WARNING";
                break;
            case type_log::ERROR:
                return "ERROR";
                break;
            case type_log::TEST:
                return "TEST";
                break;

        }
    }
    inline void log_chat_async(std::string name, firewolf::logger::type_log type, std::basic_string<char> message, sockaddr_in * client_data = NULL) // make stream?
    /*Logging system, for type: LOG=0|DEBUG=1|MESSAGE=2|WARNING=3|ERROR=4|TEST=5*/
    {
        //jthread out;
        //co_await switch_to_new_thread(out);
        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char timer[100];
        strftime(timer, 100, "%a %b %d %T %Y", localtime(&time));
        std::cout << "[" << timer << "] " << firewolf::logger::check_type(type) << " ";
        if(client_data != NULL)
        {
            std::cout << "[" << inet_ntoa(client_data->sin_addr) << ":" << ntohs(client_data->sin_port) << "]";
        }
        std::cout << " => " << message << "\n";
        //out.detach();
        //out.join();
    }
}
#endif //MODULES_LOG_STREAM_H
