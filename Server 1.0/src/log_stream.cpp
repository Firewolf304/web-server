//
// Created by root on 31.08.23.
//
#include <iostream>
#include <sstream>
#include "string"
#include <thread>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace firewolf::streams {
    class log_stream {
    public:
        /*======config======*/
        struct d {
            bool show_id_thread = false;
            bool show_time = true;
            std::chrono::duration<int64_t, std::micro> wait; // microseconds value
        } config;
        /*======config======*/
        std::jthread thread;
        log_stream() {
            init();
        }
        log_stream( const log_stream & streamer) {}
        ~log_stream() {
            stop();
        }
        void async_loop(std::stop_token token) {
            while (!token.stop_requested()) {
                //std::cout << "Thread iteration " << i << std::endl;
                if(!this->stream.str().empty()) {
                    std::stringstream id; id << std::this_thread::get_id();
                    std::cout << ((this->config.show_id_thread) ? id.str() + " " : "" ) << this->stream.rdbuf() << std::endl;
                    //std::cout << stream.str() << std::endl;
                    this->stream.str(std::string());
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds (10));
        }
        void init() { // run async thread
            this->active = true;
            this->thread = std::jthread([this](std::stop_token token) {async_loop(token);});
        }
        void stop() {
            this->active = false;
            if (this->thread.joinable()) {
                this->thread.request_stop();
                this->thread.join();
            }
        }

        void write_async(const std::string & text ) {
            this->mtx.lock();
            this->stream << text;
            std::this_thread::sleep_for(this->config.wait);
            this->mtx.unlock();
        }


        log_stream* operator=(const log_stream* rhs) { *this = *rhs; };
        log_stream* operator=(const log_stream rhs) { *this = rhs; };
        log_stream& operator<<(const std::string & input) {
            //std::cout << input;
            auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char timer[100];
            strftime(timer, 100, "%a %b %d %T %Y", localtime(&time));
            this->mtx.lock();
            this->stream << ((this->config.show_time) ? ("[" + (std::string)timer + "] ") : "") + input;
            std::this_thread::sleep_for(this->config.wait);
            this->mtx.unlock();
            return *this;
        }
    private:

        std::stringstream stream;
        std::mutex mtx;
        bool active = false;
    };
    class logger {
        using string=std::string;
    public:
        enum types : int {
            NONE = -1,
            LOG = 0,
            WARNING = 1,
            ERROR = 2,
            MESSAGE = 3,
            DEBUG = 4
        };
        /*======config======*/
        std::unordered_map<types, bool> allowed_type = {
                {types::LOG, true},
                {types::WARNING, true},
                {types::ERROR, true},
                {types::MESSAGE, true},
                {types::DEBUG, true}
        };
        /*======config======*/
    private:
        log_stream stream;
        std::vector<string> string_value = { "LOG", "WARNING", "ERROR", "MESSAGE", "DEBUG" };
        template <typename t>

        constexpr auto to_int(t e) noexcept
        {
            return static_cast<int>(e);
        }
        template <typename t>
        bool is_not_none(t e) {
            return (bool)to_int(e);
        }
        template <typename t>
        inline string to_string(t value) {
            return string_value[to_int(value)];
        }
        logger make_type(std::string type, log_stream * active_stream) {

        }
        string message;
        types set_type = NONE;
    public:
        void ShowInfo () {
            auto tostringbool = [this] (bool value) -> string {
                return (value) ? "true" : "false";
            };
            nlohmann::json data = data.parse(
                    "([ { \"CONFIG\" :  { \"show_id_thread\" : " + tostringbool(this->config->show_id_thread) +
                    "\"show_time\" : " + tostringbool(this->config->show_time) +
                    ",  }} ])" );
            std::cout << data << std::endl;
        }
        logger( const logger & log) {}
        logger() {

        }
        logger(logger::types type, std::shared_ptr<log_stream> active_streamer) {
            set_type = type;
            //this->stream = *(active_streamer);
        }
        logger(logger::types type, std::shared_ptr<log_stream> active_streamer, logger *info) {
            set_type = type;
            this->allowed_type = (*info).allowed_type;
            //this->stream = *(active_streamer);
        }
        logger(logger::types type, logger active_logger) {
            set_type = type;
            *this = active_logger;
        }

        logger(log_stream active_streamer) {
            this->stream = active_streamer;
        }
        ~logger() {
            this->stream.stop();
        }


        void operator<= (const std::string & text ) {
            this->stream << text;
        }
        logger operator[] (types type) {
            //return "[" + to_string(to_int(type)) + "] ";
            //return this->stream << "[" + to_string(to_int(type)) + "] ";
            return logger (type, std::make_shared<log_stream>(this->stream), this);
            //return logger (type, *this);
        }
        /*logger operator<< (const std::string & text) {
            this->stream << text ;
        }*/
        friend void operator<< (logger  logger_gen, const std::string & text ) {
            //logger_gen.ShowInfo();
            if(logger_gen.to_int(logger_gen.set_type) >= 0) {
                if( logger_gen.allowed_type[logger_gen.set_type] ) {
                    logger_gen.stream << "[" + logger_gen.to_string(logger_gen.to_int(logger_gen.set_type)) + "] " + text;
                }
            }
            else {
                logger_gen.stream << text;
            }
        }
        void close () {
            delete &stream;
        }

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