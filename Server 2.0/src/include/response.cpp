#include "response.h"

void api::init() {
    char buff[PATH_MAX];
    //app_path = local_path;
    ssize_t length = readlink("/proc/self/exe", buff, sizeof(buff));
    std::cout << "------------------\n   Path: " << buff << std::endl;
    app_path = ((std::string)buff).substr(0, ((std::string)buff).rfind("/"));
    std::cout << "Loading apis:\n";
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
                    }
                    },
                    {"csharp", [this, file, path]() {
                        routes[path] = [this](std::string *s, responser::response *rep, requester::request_data req, std::unordered_map<std::string, json> access_info, void* funcs ) {
                            MonoDomain *domain;
                            try {
                                //domain = mono_jit_init(file.path().string().c_str());
                                domain = mono_jit_init((app_path + path_dirs + req.request_info.path).c_str());
                                std::cout << app_path + path_dirs + req.request_info.path + "/main.dll" << std::endl;
                                MonoAssembly *assembly = mono_domain_assembly_open(domain, "/main.dll");
                                if (assembly) {
                                    MonoImage *image = mono_assembly_get_image(assembly);
                                    std::string functionName = "DLLcs.Clas2:Hello";
                                    MonoMethodDesc *methodDesc = mono_method_desc_new(functionName.c_str(), 0);
                                    MonoMethod *method = mono_method_desc_search_in_image(methodDesc, image);
                                    mono_method_desc_free(methodDesc);
                                    if (method) {
                                        void *args[3]{
                                                (void *) (s),
                                                (void *) rep,
                                                (void *) &req
                                        };
                                        MonoObject *result = mono_runtime_invoke(method, NULL, args, NULL);
                                    } else {
                                        std::cout << "\t\tcancel: cant open func\n";
                                        mono_jit_cleanup(domain);
                                        return;
                                    }
                                    mono_jit_cleanup(domain);
                                } else {
                                    std::cout << "\t\tcancel: cant open module\n";
                                    mono_jit_cleanup(domain);
                                    return;
                                }
                            }
                            catch (const std::exception &exp) { mono_jit_cleanup(domain); }
                        };
                    }
                    },
                    {"py", [this, file, path]() {
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
                                                        *s = (std::string) PyUnicode_AsUTF8(PyDict_GetItemString(pResult, ((std::string)("body_text")).c_str()));
                                                        rep->header_body += (std::string)PyUnicode_AsUTF8(PyDict_GetItemString(pResult, "header_body"));
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