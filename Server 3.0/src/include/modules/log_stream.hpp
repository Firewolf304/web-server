//
// Created by firewolf304 on 24.01.24.
//

#ifndef SERVER_3_0_LOG_STREAM_HPP
#define SERVER_3_0_LOG_STREAM_HPP


#include "../include.hpp"
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
        void async_loop(std::stop_token token);
        void init();
        void stop() ;
        static void write_async(log_streamer * logger, const std::string & text ) ;

        void operator=(const log_streamer* rhs) { *this = *rhs; };
        void operator=(const log_streamer rhs) { *this = rhs; };
        void operator<<(const std::string & input) ;
    private:

        std::stringstream stream_cout;
        std::mutex mtx;
        bool active = false;
    };

    class Messager {
        enum LogLevel { DEBUG, INFO, WARNING, ERROR };

    };

    class logger {
    private:
        log_streamer stream;
    public:
        /*======config======*/
        std::unordered_map<std::string, bool> allowed_type = {
                {"LOG", true},
                {"WARNING", true},
                {"ERROR", true},
                {"MESSAGE", true},
                {"DEBUG", true}
        };
        std::string get_id_thread() ;
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
        void operator<=(const std::string & text) {
            auto d = std::move( std::async(std::launch::async, [this, text] {
                if (value_type != "") {
                    if (this->allowed_type[value_type]) {
                        this->stream << "[" + value_type + "] " + text;
                    }
                } else {
                    this->stream << text;
                }
            }));
        }

        void out(std::string type, std::string text) {
            auto d = std::move( std::async(std::launch::async, [this, text, type] {
                if( !this->allowed_type[type] ) {return;}
                if(type != "") {
                    this->stream << "[" + type + "] " + text;
                }
                else {
                    this->stream << text;
                }
            }));
        }
        void out(std::string text) {
            auto d = std::move(std::async(std::launch::async, [this, text] {
                this->stream << text;
            }));
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
    inline char * check_type(type_log type);
    void log_chat_sync(std::string name, firewolf::logger::type_log type, std::basic_string<char> message, sockaddr_in * client_data = NULL); // make stream_cout?
}
#endif //SERVER_3_0_LOG_STREAM_HPP
