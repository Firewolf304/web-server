//
// Created by firewolf on 18.09.23.
//
#include "variables.h"

class module {
    string name;
    string path;
    typedef const char* (*InfoFunc)();
    typedef void (*startFunc)();
    void* handle;
};

class main_modules {
private:
    typedef void (*startFunc)(main_funcs);
    std::unordered_map<std::string, void*> map_handles;
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
public:
    string path = "/modules";
    vector<module> modules;
    main_modules(string app_path) {
        cout<< "Modules pathes:" << endl;
        for(auto file : std::filesystem::directory_iterator(app_path + path)) {
            if (file.is_directory()) {
                json js = js.parse(this->read_file(file.path().string() + "/config.json"));
                cout<< "\t" << file.path().filename() << endl;
                char * error;
                void *handle = dlopen((file.path().string() + "/main.so").c_str(),RTLD_LAZY | RTLD_DEEPBIND);
                if(!handle) {
                    std::cout << "\t\tcancel: cant open module\n";
                    return;
                }
                startFunc start = reinterpret_cast<startFunc>(dlsym(handle, "start"));
                if ((error = dlerror()) != NULL) {
                    std::cout << "\t\tcancel: cant load start (" << error << ")\n";
                    return;
                }
                map_handles[file.path().filename()] = handle;
                std::async(std::launch::async, start, main_data);
                delete error;
                free(error);
            }
        }
    }
};