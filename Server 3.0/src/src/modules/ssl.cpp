//
// Created by firewolf304 on 13.11.23.
//
#include "../../include/modules/ssl.hpp"
namespace firewolf::ssl_space {
    SSL_CTX *SSL::operator()() const {
        return ctx;
    }
    SSL::SSL(bool use_default, bool *handle)  {
        secure_status = handle;
        *handle = 0;
        use_default = use_default;
        InitSSL();
    }

    void SSL::init() {
        switch (use_SSL_file()) {
            case -1:
                ::perror("Cant init crt file, recommended to use default");
                break;
            case -2:
                ::perror("Cant init key file, recommended to use default");
                break;
            case -3:
                ::perror("Cant check private key");
                break;
            default:
                *secure_status = 1;
                std::cout << "Used SSL files, info:\n------------------" << std::endl;
                //print_json(Read_SSL(ctx));
                std::cout << Read_SSL(ctx).dump(4) << std::endl;
                std::cout << "------------------" << std::endl;
                break;
        }
    }

    string SSL::publicKeyToString(EVP_PKEY *pubkey) {
        std::string result;
        if (pubkey) {
            unsigned char* der = nullptr;
            int len = i2d_PUBKEY(pubkey, &der);
            if (len > 0 && der) {
                BIO* bio = BIO_new(BIO_s_mem());
                BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
                BIO_write(bio, der, len);
                BIO_flush(bio);

                BUF_MEM* mem = nullptr;
                BIO_get_mem_ptr(bio, &mem);
                if (mem && mem->length > 0) {
                    result.assign(mem->data, mem->length);
                }
                BIO_free_all(bio);
                OPENSSL_free(der);
            }
        }
        return result;
    }

    string SSL::replacer(std::string str, const std::string from, const std::string to) {
        try {
            //size_t start_pos = str.find(from);
            for(size_t start_pos = str.find(from); start_pos != string::npos; start_pos = str.find(from)) {
                str.replace(start_pos, from.length(), to);
            }
        }
        catch(std::exception& e){ return str; }
        return str;
    }

    json SSL::Read_SSL(SSL_CTX *ssl_ctx) {
        X509* x509 = SSL_CTX_get0_certificate(ssl_ctx);
        EVP_PKEY * pubkey = X509_get_pubkey(x509);
        auto flags = [](X509 *x509) -> ASN1_INTEGER* {
            ASN1_INTEGER *serial_number = X509_get_serialNumber(x509); // возможна утечка
            return serial_number; };
        auto pubkey_value = [this](EVP_PKEY * pubkey) -> string {
            RSA* rsa = EVP_PKEY_get1_RSA(pubkey);
            BIO *bio = BIO_new_mem_buf(cert.c_str(), -1);
            stringstream dd;
            dd << BN_bn2dec(RSA_get0_n(rsa));
            return dd.str();
            /*string dd;
            BUF_MEM* mem = NULL;
            BIO_get_mem_ptr(bio, &mem);
            if(mem && mem->data && mem->length)
                dd.assign(mem->data, mem->length);
            if(!dd.empty()) {return dd;}*/ };
        auto pub_info = [this](char * text) -> string {
            //(string)X509_NAME_oneline(X509_get_issuer_name(x509), NULL, 0)
            string data (text);
            return this->replacer(data, "/", "\n");
        };
        EVP_PKEY_free (pubkey);
        return (json){
                {"version", X509_get_version(x509)},
                {"pubkey", pubkey_value(pubkey)},
                {"publisher info", pub_info( X509_NAME_oneline(X509_get_issuer_name(x509), NULL, 0) )},
                {"version", X509_get_version(x509)},
                {"serial", BN_bn2hex(ASN1_INTEGER_to_BN(flags(x509), nullptr))},
                {"verify", (bool)X509_verify(x509, pubkey)},
        };
    }

    void SSL::print_json(const nlohmann::json &j, int indent) {
        if (j.is_object()) {
            for (auto it = j.begin(); it != j.end(); ++it) {
                std::cout << std::setw(indent) << '"' << it.key() << '"' << ":" << std::endl;
                print_json(it.value(), indent + 4);
            }
        } else if (j.is_array()) {
            for (auto& element : j) {
                print_json(element, indent + 4);
            }
        } else if(j.is_string()) {
            for(int i = 0; i < indent; i++) { cout << "="; }
            //std::cout << ">" << j.dump() << std::endl;
            std::cout << ">" << '"' << j.get<string>() << '"' << std::endl;
        }    else {
            for(int i = 0; i < indent; i++) { cout << "="; }
            //std::cout << ">" << j.dump() << std::endl;
            std::cout << ">" << j.dump() << std::endl;
        }
    }

    int SSL::use_SSL_file() {
        try {
            if (SSL_CTX_use_certificate_file(ctx, (path + (string) "server.crt").c_str(), SSL_FILETYPE_PEM) <= 0) {
                if (use_default) {
                    BIO *bio = BIO_new_mem_buf(cert.c_str(), -1);
                    X509 *cert = PEM_read_bio_X509(bio, NULL, 0, NULL);
                    BIO_free(bio);
                    SSL_CTX_use_certificate(ctx, cert);
                } else {
                    return -1;
                }
            }
        } catch (...) {

        }
        if (SSL_CTX_use_PrivateKey_file(ctx, (path + (string)"server.key").c_str(), SSL_FILETYPE_PEM) <= 0) {
            if(use_default){
                BIO *bio = BIO_new_mem_buf(key.c_str(), -1);
                EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, 0, NULL);
                BIO_free(bio);
                SSL_CTX_use_PrivateKey(ctx, pkey);
            }
            else {
                return -2;
            }
        }
        if (!SSL_CTX_check_private_key(ctx)) {
            return -3;
        }
        return 1;

    }

    void SSL::InitSSL() {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
    }
}