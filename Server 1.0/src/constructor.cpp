//
// Created by root on 18.07.23.
//
#include <vector>
#include <functional>
#include <source_location>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <openssl/ssl.h>

namespace firewolf::construct {
    using namespace::std;

    class product {
    public:
        vector<string> massive;
        void list() {
            for(auto i : massive) {
                cout << i << " ";
            }
            cout << endl;
        }
    };

    typedef struct arg_struct {
        int fd;
        int client_fd;
        SSL * ssl;
        int secure_handle;
        void * arg;
    } arg;
#define FUNCTION_NAME(func) #func
    class interface {
    public:
        //virtual void accepter() = 0;
        //virtual void set_fd_ssl() = 0;
        //virtual void selecter() = 0;
        //virtual void accept_ssl() = 0;
        //virtual void unblock_sock() = 0;
        //virtual void close_fd() = 0;
        virtual void test1(arg) = 0;
        virtual void test2(arg) = 0;
        virtual void test3(arg) = 0;
    };
    class constructer : interface {
    public:
        constructer() { clear(); }
        ~constructer() { delete this->prod; }
        void clear() { this->prod = new product; }

        void test1(arg) override {
            cout << "test1" << endl;
            this->prod->massive.push_back("test1");
        }
        void test2(arg) override {
            cout << "test2" << endl;
            this->prod->massive.push_back("test2");
        }
        void test3(arg) override {
            cout << "test3" << endl;
            this->prod->massive.push_back("test3");
        }
        product *GetProduct() {
            return this->prod;
        }
    private:
        product *prod;
    };

    class builder {
    public:
        std::unordered_map<std::string, function<void(arg data)>> map;
        builder(constructer * set_constructor) {
            map = {
                    {"test1", [this](arg data) {this->constructor->test1(data);}},
                    {"test2", [this](arg data) {this->constructor->test2(data);}},
                    {"test3", [this](arg data) {this->constructor->test3(data);}}
            };
            this->constructor = set_constructor;
        }
        ~builder() {
            this->map.clear();
        }
        void makeBuild(constructer * custom_constructor) { // calls for custom methods
            this->constructor = custom_constructor;
        }
        void make_static_funcs() { // calls for statics methods in this method
            this->constructor->test1({});
            this->constructor->test2({});
        }
        /*void set_constructor(constructer * set_constructor) {
            this->constructor = set_constructor;
        }*/
        template<typename T>
        std::string getFunctionName()
        {
            return typeid(T).name();
        }
    private:
        /*
         test1
         test2
         test3

         async:
            test1
            test2
            test3

         test1
         async:
            test2
            test3

         test1
         test2
         async:
            test3
        accept4
        check_client_handle
        check_secure_handle
        set_fd_ssl
        select_fd / poll_fd
        accept_ssl
        unblock_sock
        async:
            launch

         */
        bool is_map(string path) { if(map.find(path) != map.end()) { return true; } else { return false; } }

        int run_conf(arg data, vector<string> list) {
            if(!(this->file_data.empty())) {
                for(string func : list) {
                    if (is_map(func)) {
                        map[func](data);
                    } else { continue;};
                }
            }
        }
        inline vector<std::string> save_split (const std::string &s, char simp) {
            std::vector<std::string> result;
            std::stringstream ss (s);
            std::string item;
            while (getline (ss, item, simp)) {
                result.push_back (item);
            }
            return result;
        }
        inline string replacer(string str, const string from, const string to) {
            try{
                size_t start_pos = str.find(from);
                return str.replace(start_pos, from.length(), to);}
            catch(exception){ return str; }
        }
        int analyse_conf() {
            ifstream filestream (this->file_name, std::ios::binary);
            if (!filestream) {
                perror("Error load constructor config!");
                return -1;
            }
            string text;
            filestream.seekg(0, std::ios::end);
            getline(filestream, text, '\0');
            filestream.close();
            filestream.clear();
            this->file_data = save_split(text, '\n');
            vector<vector<string>> data;
            for(string text : this->file_data) {
                string text_d = replacer(text, "\t", "");
                if(is_map(text_d)) {
                    data[count(text.cbegin(), text.cend(), '+')].push_back(text_d);
                }
            }
            return 0;
        }

        vector<string> file_data;
        string file_name = "constructor.conf";
        constructer * constructor;
    };

}