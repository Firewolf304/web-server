//
// Created by firewolf304 on 13.11.23.
//

#ifndef SERVER_2_0_RESPONSE_H
#define SERVER_2_0_RESPONSE_H
/*#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
//#include <mono-2.0/mono/jit/jit.h>
//#include <mono-2.0/mono/metadata/assembly.h>
//#include <mono-2.0/mono/metadata/debug-helpers.h>
#include <dlfcn.h>
#include <filesystem>
//#include <mono-2.0/mono/jit/jit.h>
//#include <mono-2.0/mono/metadata/assembly.h>
//#include <mono-2.0/mono/metadata/debug-helpers.h>
#include <csignal>
#include <ucontext.h>*/
#include "../include.hpp"
#include "../global.hpp"


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
    public:

        bool empty = true;
        json headers;
        std::string body;
        struct req{
            enum enum_req {
                GET = 0,
                POST = 1,
                HEAD = 2,
                PUT = 3,
                DELETE = 4,
                CONNECT = 5,
                OPTIONS = 6,
                TRACE = 7,
                PATCH = 8,
                UNKNOWN = 9
            };
            std::string path = "";
            std::string protocol = "";
            std::string method = "";
            enum_req method_request = UNKNOWN;
            std::string media_path = "";
            std::unordered_map<enum_req, std::string> method_request_to_string = {
                    {GET, "GET"},
                    {POST, "POST"},
                    {HEAD, "HEAD"},
                    {PUT, "PUT"},
                    {DELETE, "DELETE"},
                    {CONNECT, "CONNECT"},
                    {OPTIONS, "OPTIONS"},
                    {TRACE, "TRACE"},
                    {PATCH, "PATCH"},
                    {UNKNOWN, "UNKNOWN"}
            };
        } request_info;

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
    //inline int get_request(char * buffer, request_data * data) {
    inline int get_request(char * buffer, request_data * data) {
        try {
            std::vector<std::string> body = save_split(buffer, '\n');
            if(body.empty() || body.size() < 3) {
                throw std::runtime_error("no body");
            }
            std::vector<std::string> info = save_split(body[0], ' ');
            try {
                if (info.size() > 2) {
                    data->request_info.method = info[0];
                    auto it = std::find_if( data->request_info.method_request_to_string.begin(), data->request_info.method_request_to_string.end(), [&info](auto element) -> bool{
                        return element.second == info[0];
                    });
                    data->request_info.method_request = it->first;
                    data->request_info.path = info[1];
                    data->request_info.protocol = info[2];
                }
            }
            catch(const std::out_of_range& e) { return -1; }
            catch(std::exception const & e) { return -1; }
            std::vector<std::string> deflare_path = save_split(data->request_info.path, '?');
            if(deflare_path.size() > 1){
                data->request_info.path = deflare_path[0];
                data->request_info.media_path = deflare_path[1];
            }
            std::vector<std::string>().swap(deflare_path); // free up memory

            data->body = get_line_ready((std::string)buffer, "\r\n\r\n");
            data->headers = headers_to_json((std::string)buffer);
        }
        catch (const std::runtime_error & e) { return -1;}
        catch (const std::exception & e) { return -1;}
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
        check_path(std::string & path) {
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
        bool is_dir(std::string &value)
        {
            const std::filesystem::path information(value);
            return is_directory(information);
        }
        bool is_dir(const std::string &value)
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
        bool has_format(std::string & value)
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
    using readyFunc = std::pair<reason, std::string>(requester::request_data&, json::iterator&, std::string & pwd);
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
                {"basic", [](requester::request_data &request, json::iterator & info, std::string & pwd)->std::pair<reason, std::string> {
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
                            return {ALLOWED, ""};
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
                                    return {ALLOWED, ""};
                                } else {
                                    file.close();
                                    return {REQUEST_TO_API, info->at("GOTO").get<std::string>()} ;
                                }
                            }
                        } else {
                            file.close();
                            return {REQUEST_TO_API, info->at("GOTO").get<std::string>()} ;
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
        std::pair<reason, std::string> check(requester::request_data & request) {
            if(!this->data.is_array()) {
                return {ALLOWED, ""};
            }
            json::iterator info = std::find_if(data.begin(), data.end(), [&](const json &element) {
                std::regex reg(element.at("PATH"));
                return std::regex_search(request.request_info.path, reg);
            });
            if(info == data.end() ) {
                return {ALLOWED, ""};
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
    ~api() {
        for(auto d : map_handles) {
            dlclose(d.second);
        }
    }
    void init() {
        char buff[PATH_MAX];
        //std::string buff;
        //app_path = local_path;
        ssize_t length = readlink("/proc/self/exe", buff, sizeof(buff));
        std::cout << "------------------\n   Path: " << buff << std::endl;
        app_path = ((std::string)buff).substr(0, ((std::string)buff).rfind("/"));
        std::cout << "Loading apis:\n";

        //::memset(buff, 0, sizeof(buff));
        for(auto file : std::filesystem::directory_iterator(app_path + path_dirs)) {
            if(file.is_directory()) {
                json info = nlohmann::json::parse(this->read_file(file.path().string() + "/config.json"));
                std::string path = info["path"].get<std::string>();
                std::cout << "\tfind => " << file.path().filename() << " to => " << path << std::endl;

                access[path] = info;

                std::unordered_map<std::string, std::function<void()>> switch_lang = {
                        {"cpp",  [this, &file, &path] () {

                            char * error;
                            std::cout << "\t\tpath - " << file.path().string() + "/main.so" << std::endl;
                            try {
                                void *handle = dlopen((file.path().string() + "/main.so").c_str(), RTLD_LAZY | RTLD_DEEPBIND);
                                if (!handle) {
                                    std::cout << "\t\tcancel: cant open module\n";
                                    return;
                                }
                                InfoFunc info = reinterpret_cast<InfoFunc>(dlsym(handle, "info"));
                                if ((error = dlerror()) != NULL) {
                                    std::cout << "\t\tcancel: cant load info (" << error << ")\n";
                                    return;
                                }
                                const char * dd = info();
                                std::string ddstr(dd);
                                std::cout << "\t\tinfo: " << ddstr << std::endl;

                                startFunc start = reinterpret_cast<startFunc>(dlsym(handle, "start"));
                                if ((error = dlerror()) != NULL) {
                                    std::cout << "\t\tcancel: cant load start (" << error << ")\n";
                                    return;
                                }
                                map_handles[path] = handle;
                                map_funcs[path] = start;
                                routes[path] = [this](std::string *s, responser::response *rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void *funcs) {
                                    try {
                                        auto handshake = [](int sig) -> void {
                                            throw std::runtime_error("something wrong with api");
                                        };
                                        signal(SIGSEGV, handshake);
                                        map_funcs[req.request_info.path](s, rep, req, funcs);
                                    } catch (std::exception &e) {
                                        *s += "[api error]";
                                        std::cerr << e.what() << "\n\tIt is -> " + req.request_info.path << std::endl;
                                    }
                                };
                                //start(s, rep, req);
                                delete error;
                                free(error);
                            }
                            catch (const std::runtime_error& e) {
                                std::cout << "Error run: " << e.what() << std::endl;
                            }
                        }},
                        {"py", [this, &file, &path]() {
                            Py_Initialize();
                            try {
                                auto error = [this]() {
                                    if (PyErr_Occurred()) {
                                        PyObject *pType, *pValue, *pTraceback;
                                        PyErr_Fetch(&pType, &pValue, &pTraceback);
                                        PyErr_NormalizeException(&pType, &pValue, &pTraceback);
                                        if (pValue != NULL) {
                                            PyObject *pStrErrorMessage = PyObject_Str(pValue);
                                            throw std::runtime_error("Python error run: " + (std::string)(PyUnicode_AsUTF8(pStrErrorMessage)));
                                            Py_XDECREF(pStrErrorMessage);
                                        }
                                        Py_XDECREF(pType);
                                        Py_XDECREF(pValue);
                                        Py_XDECREF(pTraceback);
                                    }
                                    else {
                                        throw std::runtime_error("Error run");
                                    }
                                };

                                std::cout << "\t\tpath - " << file.path().string() << "/main.py" << std::endl;
                                PyObject * sysPath = PySys_GetObject("path");
                                if (sysPath) {
                                    PyObject * pather = PyUnicode_DecodeFSDefault(
                                            (file.path().string()).c_str());
                                    PyList_Append(sysPath, pather);
                                    map_handles[path] = static_cast<void*>(pather);
                                    Py_INCREF(pather);
                                    //Py_XDECREF(path);
                                } else {
                                    error();
                                }
                                PyObject* handle = PyImport_ImportModule("main");

                                if (handle != NULL) {
                                    /*=========================INFO CHECK=========================*/
                                    PyObject* pDict = PyModule_GetDict(handle);
                                    PyObject* pFunction = PyDict_GetItemString(pDict, "info");
                                    if (pFunction != NULL) {
                                        if(PyCallable_Check(pFunction))
                                        {
                                            PyObject* pResult;
                                            pResult = PyObject_CallFunction(pFunction, NULL);
                                            PyObject* pResultStr = PyObject_Repr(pResult);          // run
                                            std::cout << "\t\tinfo: " << PyUnicode_AsUTF8(pResultStr)  << std::endl;
                                            Py_DECREF(pResult);
                                            Py_DECREF(pResultStr);
                                        }
                                    } else {
                                        error();
                                    }
                                    /*=========================INFO CHECK=========================*/
                                    /*=========================START=========================*/
                                    routes[path] = [this, file](std::string *s, responser::response *rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void* funcs ) {
                                        Py_Initialize();
                                        try {
                                            auto handshake = [](int sig) -> void {
                                                throw std::runtime_error("something wrong with api");
                                            };
                                            signal(SIGSEGV, handshake);
                                            signal(SIGABRT, handshake);
                                            PyObject * sysPath = PySys_GetObject("path");
                                            if (sysPath) {
                                                PyList_Append(sysPath, static_cast<PyObject *>(map_handles[req.request_info.path]));
                                            } else {
                                                throw std::runtime_error("Error link path");
                                            }
                                            //PyObject * pDict = PyModule_GetDict(reinterpret_cast<PyObject *>(map_handles[req.request_info.path]));
                                            PyObject* handle = PyImport_ImportModule("main");
                                            if (handle != NULL) {
                                                PyObject * pDict = PyModule_GetDict(handle);
                                                /*std::cout << "find: " << PyDict_Size(pDict) << " values" << std::endl;
                                                PyObject * keys = PyDict_Keys(pDict);
                                                for (Py_ssize_t i = 0; i < PyList_Size(keys); ++i) {
                                                    PyObject * key = PyList_GetItem(keys, i);
                                                    if (PyUnicode_Check(key)) {
                                                        std::cout << "\t\tKey: " << PyUnicode_AsUTF8(key) << std::endl;
                                                    }
                                                    Py_DECREF(key);
                                                }
                                                Py_DECREF(keys);*/
                                                PyObject * start = PyDict_GetItemString(pDict, "start");
                                                Py_DECREF(pDict);
                                                if (start != NULL) {
                                                    if (PyCallable_Check(start)) {
                                                        PyObject * pResult;
                                                        PyObject * d = PyDict_New();
                                                        /*struct convert*/

                                                        PyDict_SetItemString(d, "request_info", PyUnicode_FromString(
                                                                ("{\"path\":\"" + req.request_info.path + "\","
                                                                                                          "\"media_path\":\"" +
                                                                 req.request_info.media_path + "\","
                                                                                               "\"method\":\"" +
                                                                 req.request_info.method + "\","
                                                                                           "\"protocol\":\"" +
                                                                 req.request_info.protocol + "\"}").c_str()));
                                                        PyDict_SetItemString(d, "body",
                                                                             PyUnicode_FromString(req.body.c_str()));
                                                        PyDict_SetItemString(d, "headers", PyUnicode_FromString(
                                                                req.headers.dump().c_str()));
                                                        PyDict_SetItemString(d, "json_data", PyUnicode_FromString(("{\"pwd\": \"" + file.path().string() + "\"}").c_str()));
                                                        //pResult = PyObject_CallFunctionObjArgs(start, d);       // run
                                                        pResult = PyObject_CallOneArg(start, d);       // run
                                                        //PyObject * keys = PyDict_Keys(pResult);
                                                        if (PyDict_Check(pResult)) {
                                                            /*for (Py_ssize_t i = 0; i < PyList_Size(keys); ++i) {
                                                                PyObject * key = PyList_GetItem(keys, i);
                                                                if (PyUnicode_Check(key)) {
                                                                    std::cout << "\t\tKey: " << PyUnicode_AsUTF8(key) << std::endl;
                                                                }
                                                                Py_DECREF(key);
                                                            }*/
                                                            /*for (Py_ssize_t i = 0; i < PyList_Size(keys); ++i) {
                                                                PyObject* key = PyList_GetItem(keys, i);
                                                                PyObject* value = PyDict_GetItem(pResult, key);
                                                                std::cout << PyUnicode_AsUTF8(key) << ": " << PyUnicode_AsUTF8(value) << std::endl;
                                                            }*/
                                                            *s = static_cast<std::string>(PyUnicode_AsUTF8(PyDict_GetItemString(pResult, ((std::string)("body_text")).c_str())));
                                                            rep->header_body += static_cast<std::string>(PyUnicode_AsUTF8(PyDict_GetItemString(pResult, "header_body")));
                                                        }
                                                        Py_DECREF(pResult);
                                                        //Py_DECREF(keys);
                                                        Py_DECREF(d);
                                                    }
                                                } else {
                                                    throw std::runtime_error("error find start");
                                                }
                                                //Py_DECREF(handle);
                                                Py_DECREF(start);
                                            }
                                            else {
                                                throw std::runtime_error("handle is null");
                                            }
                                        } catch (std::exception &e) {
                                            *s = "[api error]";
                                            std::cerr << e.what() << "\n\tIt is -> " + req.request_info.path << std::endl;
                                        }
                                        Py_Finalize();
                                    };
                                    /*=========================START=========================*/
                                }
                                else {
                                    error();
                                }
                                Py_Finalize();
                            }
                            catch (const std::runtime_error& err) {
                                std::cout << "\t\tError run: " << err.what() << std::endl;
                            }
                        }
                        }

                };
                switch_lang[info["lang"].get<std::string>()]();
            }
        }
        std::cout << "------------------" << std::endl;
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

    bool is_api(const std::string &path)
    {
        if(routes.find(path) != routes.end())
        {
            return true;
        }
        else{
            return false;
        }
    }
    void check_api(const std::string &path) { // need check .so files for load modules. Use test func for this.
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
