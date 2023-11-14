//
// Created by firewolf304 on 13.11.23.
//

#ifndef SERVER_2_0_MODULES_H
#define SERVER_2_0_MODULES_H
#include "variables.h"

namespace firewolf::modules {
    class main_modules;
    struct main_funcs {
        firewolf::streams::logger* log;
        int * timeoutclient;
        api * app_ptr;
        string * ip;
        std::shared_ptr<unsigned int> portptr;
        ssl_space::SSL * ssl_info;
        std::shared_ptr<int> active_epols;
        string * pages;
        json config_Data;
        bool machine_info = false;
        vector<json> proxy_path;
        firewolf::sql::sql_dump * server;
        std::shared_ptr<sock_handle> sockserver;
        firewolf::monitor::collect_connection * client_map;
        firewolf::modules::main_modules * module_map;
        //std::shared_ptr<firewolf::streams::logger> output = output;
    };
    class main_modules {
    private:
        typedef void (*startFunc)(nlohmann::json, void *);               // typedef of main function
        typedef char *(*infoFunc)();                                   // typedef of info function

        std::string read_file(std::string path) {
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

    public:
        struct info_module {    // rule struct
            string name;                // module name
            string pwd;                 // path to module
            void *handle{};              // dlopen of module
            nlohmann::json config;      // config file
            startFunc function{};         // function
            infoFunc info{};              // info about module
        };
        string path = "/modules";
        string pwd = "";
        std::unordered_map<std::string, info_module> modules_map;     // struct of info
        std::unordered_map<std::string, std::jthread> thread_map;     // struct of info

        main_modules(string app_path) {
            pwd = app_path;
        }

        void load() {
            std::cout << "------------------" << std::endl;
            cout << "Modules pathes:" << endl;
            for (auto file: std::filesystem::directory_iterator(pwd + path)) {
                if (file.is_directory()) {
                    if (std::ifstream(file.path().string() + "/config.json").fail()) {continue;}
                    json js = js.parse(this->read_file(file.path().string() + "/config.json"));
                    cout << "\t" << file.path().filename() << endl;
                    char *error;
                    void *handle = dlopen((file.path().string() + "/mod.so").c_str(), RTLD_LAZY | RTLD_DEEPBIND);
                    if (!handle) {
                        std::cout << "\t\tcancel: cant open module\n";
                        return;
                    }
                    info_module input;
                    infoFunc info = reinterpret_cast<infoFunc>(dlsym(handle,"info"));                            // НЕ ЗАВЕРШЕН!
                    if ((error = dlerror()) != NULL) {
                        std::cerr << "\t\tcancel: cant load info (" << error << ")\n";
                        return;
                    }
                    startFunc start = reinterpret_cast<startFunc>(dlsym(handle, "start"));
                    if ((error = dlerror()) != NULL) {
                        std::cout << "\t\tcancel: cant load start (" << error << ")\n";
                        return;
                    }
                    if (js.contains("name")) {
                        input.name = js["name"];
                    } else {
                        input.name = "module_" + to_string(modules_map.size());
                    }
                    input.config = js;
                    input.handle = handle;
                    input.pwd = file.path().string();
                    input.function = start;
                    input.info = info;
                    modules_map[input.name] = input;
                    //modules_map[name].handle = handle;
                    //modules_map[name].info = info;
                    //modules_map[name].function = start;
                    //modules_map[name].config = js;
                    //std::async(std::launch::async, start, main_data);
                    delete error;
                    free(error);
                }
            }
        }

        void init_units(main_funcs * data) {
            if (modules_map.size() == 0) {
                std::cerr << "no modes to start" << std::endl;
                return;
            }
            for (auto d: modules_map) {
                thread_map[d.first] = std::jthread([this, d, data](std::stop_token token) {
                    try {
                        auto handshake = [](int sig) -> void {
                            throw std::runtime_error("something wrong with module");
                        };
                        signal(SIGSEGV, handshake);
                        //signal(SIGABRT, handshake);
                        d.second.function(d.second.config, static_cast<main_funcs*>(data));
                    }
                    catch (const std::exception &e) {
                        data->log->out("ERROR", "MODULE ERROR: " + (string) e.what() + "\n\tIt is -> " + d.first);
                        //std::cerr << "Module error" << e.what() << "\n\tIt is -> " + d.first << "" << std::endl;
                    }
                });
            }
        }

        void close_unit() {

        }
        main_modules() {}
    };
}

#endif //SERVER_2_0_MODULES_H
