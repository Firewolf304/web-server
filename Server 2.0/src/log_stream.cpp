#include <iostream>
#include <sstream>
#include "string"
#include <thread>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <future>

namespace firewolf::streams {
    class log_streamer {
    public:
        /*======config======*/
        struct d {
            bool show_id_thread = false;
            bool show_time = true;
            std::string format_type =  "%a %b %d %T %Y";
            //std::chrono::duration<int64_t, std::micro> wait = std::chrono::microseconds (1); // microseconds value
            int wait = 1;
        } config;
        /*======config======*/
        std::jthread thread;
        log_streamer() {
            init();
        }
        log_streamer(const log_streamer & streamer) {}
        ~log_streamer() {
            stop();
        }
        void async_loop(std::stop_token token) {
            while (!token.stop_requested()) {
                //std::cout << "Thread iteration " << i << std::endl;
                if(!this->stream_cout.str().empty()) {
                    std::stringstream id; id << std::this_thread::get_id();
                    std::cout << ((this->config.show_id_thread) ? "[" + id.str() + "] " : "" ) << this->stream_cout.rdbuf() << std::endl;
                    id.clear();
                    /*std::stringstream text;
                    if(this->config.show_id_thread) {
                        text << "[" << std::this_thread::get_id() << "]";
                    }
                    text << this->stream_cout.rdbuf();
                    std::cerr << text.rdbuf() << std::endl;
                    text.clear();*/
                    //std::cout << stream_cout.str() << std::endl;
                    this->stream_cout.str(std::string());
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

        static void write_async(log_streamer * logger, const std::string & text ) {
            auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char timer[100];
            strftime(timer, 100, logger->config.format_type.c_str(), localtime(&time));
            logger->mtx.lock();
            logger->stream_cout << ((logger->config.show_time) ? ("[" + (std::string)timer + "] ") : "") + text;
            //std::this_thread::sleep_for(this->config.wait);
            usleep(logger->config.wait);
            logger->mtx.unlock();
        }


        log_streamer* operator=(const log_streamer* rhs) { *this = *rhs; };
        log_streamer* operator=(const log_streamer rhs) { *this = rhs; };
        void operator<<(const std::string & input) {
            /*auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char timer[100];
            strftime(timer, 100, "%a %b %d %T %Y", localtime(&time));
            this->mtx.lock();
            this->stream_cout << ((this->config.show_time) ? ("[" + (std::string)timer + "] ") : "") + input;
            usleep(this->config.wait);
            this->mtx.unlock();*/
            std::async(std::launch::async, [this, input]() {
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                char timer[100];
                strftime(timer, 100, this->config.format_type.c_str(), localtime(&time));
                this->mtx.lock();
                this->stream_cout << ((this->config.show_time) ? ("[" + (std::string)timer + "] ") : "") + input;
                usleep(this->config.wait);
                this->mtx.unlock();
            });
        }
    private:

        std::stringstream stream_cout;
        std::mutex mtx;
        bool active = false;
    };
    class logger {
    private:
        log_streamer stream;
        //std::vector<std::string> string_value = { "LOG", "WARNING", "ERROR", "MESSAGE", "DEBUG" };
        /*int to_int(types e)
        {
            return static_cast<int>(e);
        }
        bool is_not_none(types e) {
            return (bool)e;
        }
        string to_stringer(types value) {
            return string_value[value];
        }
        logger make_type(std::string type, log_streamer * active_stream) {

        }*/
    public:
        /*enum types : int {
            NULLER = -2,
            NONE = -1,
            LOG = 0,
            WARNING = 1,
            ERROR = 2,
            MESSAGE = 3,
            DEBUG = 4
        };*/
        /*======config======*/
        std::unordered_map<std::string, bool> allowed_type = {
                {"LOG", true},
                {"WARNING", true},
                {"ERROR", true},
                {"MESSAGE", true},
                {"DEBUG", true}
        };
        std::string get_id_thread() {
            std::stringstream id; id<< this->stream.thread.get_id();
            return  id.str();
        }
        /*======config======*/
        /*void ShowInfo () {
            auto tostringbool = [this] (bool value) -> std::string {
                return (value) ? "true" : "false";
            };
            nlohmann::json data = data.parse(
                    "([ { \"CONFIG\" :  { \"show_id_thread\" : " + tostringbool(this->config->show_id_thread) +
                    "\"show_time\" : " + tostringbool(this->config->show_time) +
                    ",  }} ])" );
            std::cout << data << std::endl;
        }*/
        logger( const logger & log) {}
        logger() {

        }
        /*logger(logger::types type, std::shared_ptr<log_streamer> active_streamer) {
            set_type = type;
            //this->stream_cout = *(active_streamer);
        }
        logger(logger::types type, std::shared_ptr<log_streamer> active_streamer, logger *info) {
            set_type = type;
            this->allowed_type = (*info).allowed_type;
            this->stream_cout.config = *((*info).config);
        }
        logger(logger::types type, logger active_logger) {
            set_type = type;
            *this = active_logger;
        }*/

        ~logger() {
            this->stream.stop();
        }
        std::string value_type = "";
        logger operator[] (const std::string value) {
            this->value_type = value;
            return *this;
        }
        void operator<< (const std::string & text ) {
            this->stream << text;
        }
        /*friend void operator << (logger type, const std::string& text) {
            if(type.value_type != "") {
                if( type.allowed_type[type.value_type] ) {
                    type.stream_cout << "[" + type.value_type + "] " + text;
                }
            }
            else {
                type.stream_cout << text;
            }
        }*/
        void operator<=(const std::string & text) {
            if(value_type != "") {
                if( this->allowed_type[value_type] ) {
                    this->stream << "[" + value_type + "] " + text;
                }
            }
            else {
                this->stream << text;
            }
        }

        void out(std::string type, std::string text) {
            if( !this->allowed_type[type] ) {return;}
            if(type != "") {
                this->stream << "[" + type + "] " + text;
            }
            else {
                this->stream << text;
            }

        }
        void out(std::string text) {
            this->stream << text;
        }

        void close () {
            this->stream.stop();
        }
        log_streamer::d * config = &this->stream.config;
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
    inline void log_chat_async(std::string name, firewolf::logger::type_log type, std::basic_string<char> message, sockaddr_in * client_data = NULL) // make stream_cout?
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