//
// Created by root on 25.04.23.
//
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <dlfcn.h>
#include <filesystem>
typedef int sock_handle;
#include <sstream>
#include <vector>
#include <filesystem>
#include <zlib.h>
#include <fstream>
#include <functional>

#include "nlohmann/json.hpp"
using namespace::nlohmann;
namespace firewolf::responser{
    typedef struct body{
        std::vector<std::vector<std::string>> headers;
    };
    class response{
    public:
        std::string header_body = "HTTP/1.1 200 OK\r\n";
        std::string body_text;
        std::string make_request(){
            return header_body + "Content-Length: " + std::to_string(body_text.length()) + "\r\n\r\n" + body_text;
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
            result = hed.substr(pos, hed.find('\r\n', pos) - pos);
            result = replacer(result, "\r", "");
            result = replacer(result, "\n", "");
            //cout << "new_pose = " << pos << " " <<  pos << " " << result << endl;
            std::string save = result.substr(0, result.find(": ") );
            int l = result.find(": ");
            js.emplace(result.substr(0, l ), result.substr(l + 2, result.length() - l));
            //cout << result.substr(0, l ) << " => " << js[result.substr(0, l )].get<string>() << endl;
            pos = hed.find('\r\n', pos) + 1;
            if(hed.find('\r\n', pos-1) == std::string::npos) break;
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
            catch(const std::out_of_range& e) {} catch(std::exception e) {}
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
                {".js", "text/html"},
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


}
class api{
private:
    typedef const char* (*InfoFunc)();
    typedef void (*startFunc)(std::string*, responser::response*, requester::request_data);

public:
    std::unordered_map<std::string, std::function<void(std::string*, responser::response*, requester::request_data, std::unordered_map<std::string, json>)>> routes;
    std::unordered_map<std::string, startFunc> map_funcs;
    std::unordered_map<std::string, void*> map_handles;
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
    api()
    {
        char buff[PATH_MAX];
        //app_path = local_path;
        ssize_t length = readlink("/proc/self/exe", buff, sizeof(buff));
        std::cout << "   Path: " << buff << std::endl;
        app_path = ((std::string)buff).substr(0, ((std::string)buff).rfind("/"));
        std::cout << "------------------\nLoading apis:\n";
        ::memset(buff, 0, sizeof(buff));
        for(auto file : std::filesystem::directory_iterator(app_path + path_dirs)) {
            if(file.is_directory()) {
                json info = info.parse(this->read_file(file.path().string() + "/config.json"));
                std::string path = info["path"].get<std::string>();
                std::cout << "\tfind => " << file.path().filename() << " to => " << path << std::endl;

                access[path] = info;


                std::unordered_map<std::string, std::function<void()>> switch_lang = {
                    {"cpp",  [this, file, path] () {
                            char * error;
                        std::cout << "\t\tpath - " << file.path().string() + "/main.so" << std::endl;
                            void *handle = dlopen((file.path().string() + "/main.so").c_str(),RTLD_LAZY);
                            if(!handle) {
                                std::cout << "\t\tcancel: cant open module\n";
                                return;
                            }
                            InfoFunc info = reinterpret_cast<InfoFunc>(dlsym(handle, "info"));
                            if ((error = dlerror()) != NULL) {
                                std::cout << "\t\tcancel: cant load info (" << error << ")\n";
                                return;
                            }
                        std::cout << "\t\tinfo: " << (std::string)info() << std::endl;

                            startFunc start = reinterpret_cast<startFunc>(dlsym(handle, "start"));
                            if ((error = dlerror()) != NULL) {
                                std::cout << "\t\tcancel: cant load start (" << error << ")\n";
                                return;
                            }
                            map_handles[path] = handle;
                            map_funcs[path] = start;
                            routes[path] =[this](std::string * s, responser::response* rep, requester::request_data req, std::unordered_map<std::string, json> access_info) {
                                map_funcs[req.request_info.path](s, rep, req);
                            };
                            //start(s, rep, req);
                            delete error;
                            free(error);
                        }
                    },
                    {"csharp", [this, file, path]() {
                            routes[path] = [this](std::string *s, responser::response *rep,
                                                                                   requester::request_data req,
                                                                                   std::unordered_map<std::string, json> access_info) {

                                //map_funcs[req.request_info.path](s, rep, req);
                            };
                        }
                    }

                };
                switch_lang[info["lang"].get<std::string>()]();
                //dlclose(handle);
                /*routes["/" + file.path().filename().string()] = [this](string * s, responser::response* rep, requester::request_data req, unordered_map<string, json> access_info) {
                    char * error;
                    cout << "\t\tpath - " << app_path + path_dirs + req.request_info.path + "/main.so" << endl;
                    void *handle = dlopen((app_path + path_dirs + req.request_info.path + "/main.so").c_str(),RTLD_LAZY);
                    if(!handle) {
                        cout << "\t\tcancel: cant open module\n";
                        return;
                    }

                    InfoFunc info = reinterpret_cast<InfoFunc>(dlsym(handle, "info"));
                    if( (error = dlerror()) != NULL) {
                        cout << "\t\tcancel: cant load info (" << error << ")\n";
                        return;
                    }
                    cout << "\t\tinfo: " << (string)info() << endl;

                    startFunc start = reinterpret_cast<startFunc>(dlsym(handle, "start"));
                    if((error = dlerror()) != NULL) {
                        cout << "\t\tcancel: cant load start (" << error << ")\n";
                        return;
                    }

                    start(s, rep, req);
                    delete error;
                    free(error);
                    dlclose(handle);

                };*/
            }
        }
        /*routes["/api"] = [](string * s, responser::response *rep, requester::request_data req){
            if(req.request_info.method == "POST")
            {
                rep->header_body += "Set-Cookie: token=";
            }
        };

        routes["/rand"] = [](string * s, responser::response* rep, requester::request_data req){
            json media;
            if(req.request_info.method == "POST") {
                unordered_map<string, function<void(json * media, string body)>> check{
                        {
                                "application/json", [](json * media, string str) {
                            (*media) = json::parse(str.c_str()); }},
                        {
                                "application/x-www-form-urlencoded",[](json * media, string str) {
                            *media = requester::media_to_json(str); }
                        }
                };
                try{
                    check[req.headers["Content-Type"].get<string>()](&media, req.body);
                    //media = json::parse(req.body);
                }
                catch(exception e){
                    media = requester::media_to_json(req.request_info.media_path);
                }

            }
            else if(req.request_info.method == "GET" || req.request_info.method == "PUT"){
                try {
                    media = requester::media_to_json(req.request_info.media_path);
                }
                catch (exception e) {media = NULL;}
            }
            try {
                *s = (media.size() > 0) ? to_string(1 + rand() % stoi(media["to"].get<string>())) : to_string(rand());
            }
            catch (exception e){
                *s = to_string(rand());
            }
        };*/
    }

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

