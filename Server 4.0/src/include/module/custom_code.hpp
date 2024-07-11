//
// Created by firewolf304 on 30.03.24.
//

#ifndef SERVER4_0_CUSTOM_CODE_H
#define SERVER4_0_CUSTOM_CODE_H
#include <iostream>
#include <filesystem>
namespace firewolf::code_error {
    class no_file_error: public std::exception
    {
    public:
        no_file_error(const std::string& message): message{message}
        {}
        const char* what() const noexcept override
        {
            return message.c_str();
        }
    private:
        std::string message;
    };
    class run_error : public std::exception
    {
    public:
        run_error(std::string const & message): message{message}
        {}
        const char* what() const noexcept override
        {
            return message.c_str();
        }
    private:
        std::string message;
    };
    class signal_error: public std::exception
    {
    public:
        int code;
        signal_error(int code, const std::string& message): code(code), message{message}
        {}
        const char* what() const noexcept override
        {
            return ("[" + std::to_string(code) + "] " + message).c_str();
        }
    private:
        std::string message;
    };
}
#endif //SERVER4_0_CUSTOM_CODE_H
