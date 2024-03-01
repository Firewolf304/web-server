//
// Created by firewolf304 on 24.01.24.
//
#include "../../include/modules/log_stream.hpp"

namespace firewolf::streams {
    void log_streamer::async_loop(std::stop_token token) {
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
    void log_streamer::init() { // run async thread
        this->active = true;
        this->thread = std::jthread([this](std::stop_token token) {async_loop(token);});
    }
    void log_streamer::stop() {
        this->active = false;
        if (this->thread.joinable()) {
            this->thread.request_stop();
            this->thread.join();
        }
    }
    void log_streamer::write_async(firewolf::streams::log_streamer *logger, const std::string &text) {
        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char timer[100];
        strftime(timer, 100, logger->config.format_type.c_str(), localtime(&time));
        logger->mtx.lock();
        logger->stream_cout << ((logger->config.show_time) ? ("[" + std::string(timer) + "] ") : "") + text;
        //std::this_thread::sleep_for(this->config.wait);
        usleep(logger->config.wait);
        logger->mtx.unlock();
    }
    void log_streamer::operator<<(const std::string & input) {
        /*auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char timer[100];
        strftime(timer, 100, "%a %b %d %T %Y", localtime(&time));
        this->mtx.lock();
        this->stream_cout << ((this->config.show_time) ? ("[" + (std::string)timer + "] ") : "") + input;
        usleep(this->config.wait);
        this->mtx.unlock();*/
        auto d = std::move( std::async(std::launch::async, [this, input]() {
            auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char timer[100];
            strftime(timer, 100, this->config.format_type.c_str(), localtime(&time));
            this->mtx.lock();
            this->stream_cout << ((this->config.show_time) ? ("[" + (std::string)timer + "] ") : "") + input;
            usleep(this->config.wait);
            this->mtx.unlock();
        }));
    }
    std::string logger::get_id_thread() {
        std::stringstream id; id<< this->stream.thread.get_id();
        return  id.str();
    }

}
/*namespace firewolf::logger {
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
    inline void log_chat_sync(std::string name, firewolf::logger::type_log type, std::basic_string<char> message, sockaddr_in * client_data) // make stream_cout?
    //Logging system, for type: LOG=0|DEBUG=1|MESSAGE=2|WARNING=3|ERROR=4|TEST=5
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
}*/