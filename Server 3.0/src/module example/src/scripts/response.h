//
// Created by firewolf304 on 13.11.23.
//

#ifndef SERVER_2_0_RESPONSE_H
#define SERVER_2_0_RESPONSE_H
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
//#include <mono-2.0/mono/jit/jit.h>
//#include <mono-2.0/mono/metadata/assembly.h>
//#include <mono-2.0/mono/metadata/debug-helpers.h>
#include <dlfcn.h>
#include <filesystem>

#include <sstream>
#include <vector>
#include <filesystem>
#include <zlib.h>
#include <fstream>
#include <functional>
//#include <Python.h>
#include "nlohmann/json.hpp"


typedef int sock_handle;
using namespace::nlohmann;


namespace firewolf::responser{

    struct [[maybe_unused]] body{
        std::vector<std::vector<std::string>> headers;
    };
    class response{
    public:
        response() { }
        response(bool secure) : user_secure(secure) {};
        std::map<int, std::string> code_comment = {
                //Information responses
                {100, "Continue"},
                {101, "Switching Protocols"},
                {102, "Processing"},
                {103, "Early Hints"},
                //Successful responses
                {200, "OK"},
                {201, "Created"},
                {202, "Accepted"},
                {203, "Non-Authoritative Information"},
                {204, "No Content"},
                {205, "Reset Content"},
                {206, "Partial Content"},
                {207, "Multi-Status"},
                {208, "Already Reported"},
                {226, "IM Used"},
                //Redirection messages
                {300, "Multiple Choices"},
                {301, "Moved Permanently"},
                {302, "Found"},
                {303, "See Other"},
                {304, "Not Modified"},
                {305, "Use Proxy"},
                {306, "unused"},
                {307, "Temporary Redirect"},
                {308, "Permanent Redirect"},
                //Client error responses
                {400, "Bad Request"},
                {401, "Unauthorized"},
                {402, "Payment Required"},
                {403, "Forbidden"},
                {404, "Not Found"},
                {405, "Method Not Allowed"},
                {406, "Not Acceptable"},
                {407, "Proxy Authentication Required"},
                {408, "Request Timeout"},
                {409, "Conflict"},
                {410, "Gone"},
                {411, "Length Required"},
                {412, "Precondition Failed"},
                {413, "Payload Too Large"},
                {414, "URI Too Long"},
                {415, "Unsupported Media Type"},
                {416, "Range Not Satisfiable"},
                {417, "Expectation Failed"},
                {418, "I'm a teapot"},
                {421, "Misdirected Request"},
                {422, "Unprocessable Content"},
                {423, "Locked"},
                {424, "Failed Dependency"},
                {425, "Too Early"},
                {426, "Upgrade Required"},
                {428, "Precondition Required"},
                {429, "Too Many Requests"},
                {431, "Request Header Fields Too Large"},
                {451, "Unavailable For Legal Reasons"},
                //Server error responses
                {500, "Internal Server Error"},
                {501, "Not Implemented"},
                {502, "Bad Gateway"},
                {503, "Service Unavailable"},
                {504, "Gateway Timeout"},
                {505, "HTTP Version Not Supported"},
                {506, "Variant Also Negotiates"},
                {507, "Insufficient Storage"},
                {508, "Loop Detected"},
                {510, "Not Extended"},
                {511, "Network Authentication Required"}
        };
        //"HTTP/1.1 200 OK\r\n"
        bool user_secure = false;
        std::map <bool, std::string> protocol {
                {true, "HTTPS"},
                {false, "HTTP"}
        };
        std::string header_body;
        std::string version = "1.1";
        int code_status = 200;
        std::string custom_comment;
        std::string body_text;
        std::string make_request(){
            return protocol[user_secure]+"/"+version+" "+std::to_string(code_status)+" "+(custom_comment.empty() ? code_comment[code_status] : custom_comment) + "\r\n" + header_body +
                   "Content-Length: " + std::to_string(body_text.length()) + "\r\n\r\n" + body_text;
        }
    };
}
using namespace::firewolf;
namespace firewolf::requester{
    struct request_data{
        bool empty = true;
        struct req{
            std::string path = "";
            std::string protocol = "";
            std::string method = "";
            std::string media_path = "";
        } request_info;
        struct file_info {

        };
        json headers;
        std::string body;
    };

    /*typedef struct about{
        vector<string> body;
        int lenght;
    } headers;

    typedef struct req{
        vector<string> data;
        int lenght;
    } request;

    struct request_data{
        bool empty = true;
        request request_info;
        headers headers_info;
    };*/


    /*char ** splitter(char * buffer, int size_buffer, int *lenght, char simb)    // утечка
    {
        *lenght = 0;
        char ** data = (char **)malloc(2);
        data[*lenght] = (char*)malloc(2);
        for(int i = 0, k = 0; i < ::strlen(buffer); i++, k++)
        {
            int s = sizeof(data);
            int j = sizeof(data[*lenght]);
            //cout  << i << " " << buffer[i] << " " << sizeof(buffer[i]) << " " << sizeof(data) + sizeof(buffer[i]) + 20 << " " << sizeof(data[*lenght]) + sizeof(buffer[i]) + 20 << endl;
            //data = (char**)realloc(data, sizeof(data) + sizeof(buffer[i]) + 20);
            //data[*lenght] = (char*)::realloc(data[*lenght], sizeof(data[*lenght]) + sizeof(buffer[i]) + 20);
            data = (char**)realloc(data, ((*lenght)+1) * sizeof(data) + 1);
            data[*lenght] = (char*)::realloc(data[*lenght], sizeof(data[*lenght]) * (k+1) + 1);
            if(buffer[i] == simb){
                k = -1;
                (*lenght)++;
                data[*lenght] = (char*)malloc(2);
            }
            else {

                data[*lenght][k] = buffer[i];
                //cout << "res-" << data[*lenght] << endl;
            }
        }
        (*lenght)++;
        return data;
    }*/

    inline std::vector<std::string> save_split (const std::string &s, char simp) {
        std::vector<std::string> result;
        std::stringstream ss (s);
        std::string item;
        while (getline (ss, item, simp)) {
            result.push_back (item);
        }
        return result;
    }
    inline std::string get_line_ready(std::string text, std::string simb) {
        return text.substr(text.find(simb)+simb.length(), text.length()); }
    inline std::string replacer(std::string str, const std::string from, const std::string to, int count = -1) {
        try{
            while(str.find(from) >= 0 && count != 0) {
                size_t start_pos = str.find(from);
                str = str.replace(start_pos, from.length(), to);
                count--;
            }
            return str;
        }
        catch(std::exception){ return str; }
    }
    inline json headers_to_json(std::string text) {
        std::string hed = text.substr(text.find("\r\n")+2, text.find("\r\n\r\n") - text.find("\r\n")-2); // input only headers
        json js;
        //cout << hed << endl;
        int pos = 0;
        //cout << "lenght = " << hed.length() << endl;
        while(true)
        {
            std::string result;
            result = hed.substr(pos, hed.find("\r\n", pos) - pos);
            result = replacer(result, "\r", "");
            result = replacer(result, "\n", "");
            //cout << "new_pose = " << pos << " " <<  pos << " " << result << endl;
            std::string save = result.substr(0, result.find(": ") );
            int l = result.find(": ");
            js.emplace(result.substr(0, l ), result.substr(l + 2, result.length() - l));
            //cout << result.substr(0, l ) << " => " << js[result.substr(0, l )].get<string>() << endl;
            pos = hed.find("\r\n", pos) + 1;
            if(hed.find("\r\n", pos-1) == std::string::npos) break;
        }
        //cout << "end" << endl;
        return js;
    }
    inline int get_request(char * buffer, request_data * data) {
        try {
            std::vector<std::string> body = save_split(buffer, '\n');
            std::vector<std::string> info = save_split(body[0], ' ');
            try {
                if (info.size() > 2) {
                    data->request_info.method = info[0];
                    data->request_info.path = info[1];
                    data->request_info.protocol = info[2];
                }
            }
            catch(const std::out_of_range& e) { return -1; } catch(std::exception e) { return -1; }
            std::vector<std::string> deflare_path = save_split(data->request_info.path, '?');
            if(deflare_path.size() > 1){
                data->request_info.path = deflare_path[0];
                data->request_info.media_path = deflare_path[1];
            }
            std::vector<std::string>().swap(deflare_path); // free up memory

            data->body = get_line_ready((std::string)buffer, "\r\n\r\n");
            data->headers = headers_to_json((std::string)buffer);
        }
        catch (const std::exception e) { return -1;}
        return 1;
    }
    inline std::string pop_line (std::string text, std::string from, std::string to) {
        size_t start_line = text.find(from);
        if(start_line != std::string::npos) {
            size_t end = text.find(to, start_line);
            if(end != std::string::npos) {
                //return text.substr(start_line, end - start_line);
                return text.erase(start_line, end - start_line + to.length());
            }
            else{ return text; }
        }
        else { return text; }
    }
    inline json media_to_json(std::string media) {
        json convert;
        std::vector<std::string> med = save_split(media, '&');
        for(std::string a : med)
        {
            std::vector<std::string> d = save_split(a, '=');
            //convert[d[0]] = d[1];
            convert.emplace(d[0], d[1]);
        }
        return convert;
    }
    inline json cookie_to_json(std::string cookie) {
        cookie.replace(cookie.begin(), cookie.end(), ";", "");
        json convert;
        std::vector<std::string> med = save_split(cookie, ' ');
        //for(int i = 1; i < med.size(); i++)
        int i = 1;
        while(i < med.size())
        {
            std::vector<std::string> d = save_split(med[i], '=');
            convert[d[0]] = d[1];
            i++;
        }
        return convert;
    }
    class check_path{
    public:
        std::unordered_map<std::string, std::string> format = {
                {".js", "text/javascript"},
                {".html", "text/html"},
                {".jpg", "image/jpg"},
                {".png", "image/png"},
                {".txt", "text/html"},
                {".xml","text/xml"},
                {".csv", "text/csv"},
                {".css", "text/css"},
                {".ogg", "audio/ogg"},
                {".pdf", "application/pdf"},
                {".json","text/json"},
                {".zip", "application/zip"},
                {".ico", "image/x-icon"}
        };
        check_path(std::string path) {
            if(this->is_dir(path)) {
                path+="/main.html";
            }
            this->path = path;
            info = std::filesystem::path(path);
        }
        std::filesystem::path info;
        std::string path = "";
        bool is_dir()
        {
            return is_directory(this->info);
        }
        bool is_dir(std::string value)
        {
            const std::filesystem::path information(value);
            return is_directory(information);
        }
        bool has_format() {
            return this->info.has_extension();
        }
        std::string return_content() {
            std::string value = this->info.filename().extension().string();
            if(!(this->is_dir()) && has_format(value)) {
                return this->format[value];
            }
            return format["txt"];
        }
        bool has_format(std::string value)
        {
            if(format.find(value) != format.end())
            {
                return true;
            }
            return false;
        }
    };
    enum reason : int {
        REQUEST_TO_API = 0, // goto api
        REQUEST_TO_URL = 1, // goto url
        CLOSE_CLIENT = 2,   // fuck client
        ALLOWED = 3,
    };
    using readyFunc = reason(requester::request_data&, json::iterator&, std::string pwd);
    /*class access_object {
    public:
        std::future<reason>& func;
        access_object (std::future<reason>& func ) : func(func) { }
        reason get_result() {
            this->func.wait();
            return this->func.get();
        }
    };*/
    class access_response {
    public:
        json data;          // content data from access.json
        std::string access;
        std::string pwd;
        std::unordered_map<std::string,std::function<readyFunc>> funcs = {
                {"basic", [](requester::request_data &request, json::iterator & info, std::string pwd)->reason {
                    /*
                     DB Example:
                        [
                            {"ID":"abcd1", "USER": "admin", "PASSWORD" : "admin", "LEVEL" : 20}
                        ]
                     Access DB Example:
                        [
                            {"TOKEN": "1234ABCD", "ID"}
                        ]
                     */
                    auto error = [](int sig) -> void {
                        throw std::runtime_error("something wrong with access");
                    };
                    signal(SIGSEGV, error);
                    signal(SIGILL, error);
                    json db;
                    std::ifstream file(pwd + "/access_db.json");
                    try {
                        if (info->at("ALLOW").get<bool>()) {
                            file.close();
                            return ALLOWED;
                        }
                        bool new_file = false;
                        // get db
                        if (file.fail()) {
                            new_file = true;
                            //global_ptr->log->out("DEBUG", "Creating access db");
                        } else {
                            std::string temp,line;
                            while (std::getline(file, line)) {
                                temp += line;
                            }
                            try {
                                db = json(temp);
                            } catch (std::exception &e) {
                                //global_ptr->log->out("ERROR", "Error read file: " + std::string(e.what()));
                            }
                        }
                        if(!new_file) {
                            auto it = request.headers.find("token");
                            if(it != request.headers.end()) {
                                auto session = std::find_if(db.begin(), db.end(), [&](const json &element){
                                    return it->get<std::string>() == element.at("TOKEN").get<std::string>();
                                });
                                if(session != db.end()) {
                                    file.close();
                                    return ALLOWED;
                                } else {
                                    file.close();
                                    return REQUEST_TO_API;
                                }
                            }
                        } else {
                            file.close();
                            return REQUEST_TO_API;
                        }
                        // processing db
                    } catch(std::runtime_error & e) {
                        db.clear();
                        file.close();
                        //global_ptr->log->out("ERROR", "Something wrong in access file");
                    }

                }},
        };
        access_response(const json& list) : data(list) {
            char pwd[PATH_MAX];
            memset(&pwd, 0, sizeof(pwd));
            getcwd(pwd, sizeof(pwd));
            this->pwd = std::string(pwd);
            memset(&pwd, 0, sizeof(pwd));
        }
        reason check(requester::request_data request) {
            json::iterator info = std::find_if(data.begin(), data.end(), [&](const json &element) {
                std::regex reg(element.at("PATH"));
                return std::regex_search(request.request_info.path, reg);
            });
            if(info == data.end()) {
                return ALLOWED;
            }
            return funcs[ info->at("METHOD").get<std::string>() ](request, info, this->pwd);

        }


        /*
         Example file:
            [
              {
                "PATH" : "/*",      // path to content
                "ALLOW" : false,    // allow this content
                "METHOD" : "basic", // processing method
                "REASON" : 0        // what to do in int, check "reason"
              }
            ]
         */


        access_response(){}
    };



}



class api{
private:
    typedef const char* (*InfoFunc)();
    typedef void (*startFunc)(std::string*, responser::response*, requester::request_data, void*  );
public:
    std::unordered_map<std::string, std::function<void(std::string*, responser::response*, requester::request_data, std::unordered_map<std::string, json>, void* )>> routes;    // functions run
    std::unordered_map<std::string, startFunc> map_funcs;                                                                                                                       // map of .so main functions
    std::unordered_map<std::string, void*> map_handles;                                                                                                                         // handles run

    std::string path_dirs = "/apis";
    std::string app_path = "";
    std::unordered_map<std::string, json> access;
    std::string read_file (std::string path) {
        std::ifstream file(path, std::ios::binary);
        std::string line;
        std::string text;
        if (file.is_open()) {
            while (getline(file, line)) {
                text += line + "\n";
            }
            file.close();
        }
        return text;
    }
    api() {  }
    void init();

    bool is_api(std::string path)
    {
        if(routes.find(path) != routes.end())
        {
            return true;
        }
        else{
            return false;
        }
    }
    void check_api(std::string path) { // need check .so files for load modules. Use test func for this.
        if (!std::filesystem::exists(path)) {
            return;
        }
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::cout << entry.path().filename() << std::endl;
            }
        }
    }

};


#endif //SERVER_2_0_RESPONSE_H
