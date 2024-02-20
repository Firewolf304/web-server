//
// Created by firewolf304 on 02.11.23.
//

#ifndef MODULES_SSL_H
#define MODULES_SSL_H

#include <string>
#include <nlohmann/json.hpp>
#include "openssl/ssl.h"
namespace firewolf::ssl_space {
    class SSL {
    public:
        bool *secure_status;
        SSL_CTX * operator() () const;
        bool use_default;
        std::string path;
        SSL(bool use_default, bool *handle);
        ~SSL();
        void init();
        void close();
        std::string publicKeyToString(EVP_PKEY* pubkey);
        std::string replacer(std::string str, const std::string from, const std::string to);
        nlohmann::json Read_SSL(SSL_CTX * ssl_ctx);
        void print_json(const nlohmann::json& j, int indent = 0);
        int use_SSL_file();
        void InitSSL();
        SSL_CTX * ctx;
        std::string key = "";
        std::string cert = "";
    };
}
#endif //MODULES_SSL_H
