//
// Created by root on 19.06.23.
//
#include <iostream>
#include <string>
#include <sstream>
#include "openssl/ssl.h"
#include "openssl/aes.h"
#include <nlohmann/json.hpp>
namespace firewolf::ssl_space {

    using namespace::std;
    using namespace::nlohmann;
    class SSL {
    public:
        bool *secure_status = 0;
        SSL_CTX * operator() () const{
            return ctx;
        }
        bool use_default = false;
        string path = "";
        SSL(bool use_default, bool *handle){
            secure_status = handle;
            *handle = 0;
            use_default = use_default;
            InitSSL();
        }
        ~SSL() {
            close();
        }
        void init() {
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
                    cout << "Used SSL files, info:\n------------------" << endl;
                    //print_json(Read_SSL(ctx));
                    cout << Read_SSL(ctx).dump(4) << endl;
                    cout << "------------------" << endl;
                    break;
            }
        }
        void close() {
            SSL_CTX_free(ctx);
        }
        string publicKeyToString(EVP_PKEY* pubkey)
        {


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
        inline string replacer(string str, const string from, const string to) {
            try {
                //size_t start_pos = str.find(from);
                for(size_t start_pos = str.find(from); start_pos != string::npos; start_pos = str.find(from)) {
                    str.replace(start_pos, from.length(), to);
                }
            }
            catch(exception){ return str; }
            return str;
        }
        json Read_SSL(SSL_CTX * ssl_ctx) {
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
        void print_json(const json& j, int indent = 0)
        {
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
    private:

        int use_SSL_file() {
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

        void InitSSL() {
            SSL_library_init();
            OpenSSL_add_all_algorithms();
        }
    protected:
        SSL_CTX * ctx = SSL_CTX_new(SSLv23_method());
        string key = "-----BEGIN PRIVATE KEY-----\n"
                      "MIIJQwIBADANBgkqhkiG9w0BAQEFAASCCS0wggkpAgEAAoICAQDdg8gWbgPHWJO0\n"
                      "zYUfmGnCqyb5jTwBZfra/2VOWzdXd8htxKGNJAEQx3Ld3Vl8nKz+GWdZGYDm8Wa2\n"
                      "DOJt0EWxRfhY8ieXGw7ganHuL8Hjuhj/O8aIB4XaJpYFaSwlSeO/7efjfgYC15MP\n"
                      "KXtwBSJ4E+c7AwzOqlmJ1QsyxqfLN23dzu6s+LibC4na42bxte1bdkMaCjek6il5\n"
                      "/zJX5WojoT3rZvsLqseuHaejgoCfsabEQYjOcavQEpu2RV0bYqgq34Az9habYUpM\n"
                      "EWnpg0vXIXOhrooFoTfbYTcvk7Bb4KeRzHP4zmmkIFZLE4B/9l0FYVTlQNF/b+wC\n"
                      "WLoWcGqUzn/2Md4cdtn+yH8pxrWncTPiqL0X7CdpZVnJlEL2G6gkhG3ppbb0R7jY\n"
                      "JHq72uJwn84lJw2/lmRDHdqRoOZAd+tc/mDJZsjoPPlqQtdOFEc9GaoEJsaszcNe\n"
                      "T2lXtki55w4l6hmmnBPYV9GYZGOyXDAq1DAXuLagqw8jSIDOIjdy+f0fbJvXdQdt\n"
                      "2tPin6qd6Z5RL7m90RgZAAlRmeinrNr8qLhQE9KmX77KxLmIUaz84TdC8hGCIw70\n"
                      "7TTMOZiO6DEnYTBKP5DqsPRNYcaKoFs8u2Qetm5X8tf02HUoP+opf40MWTmB/BrM\n"
                      "TZEb457yX0Z/lCJpGfPAov4XDIytcQIDAQABAoICACJNtx2qGdG0KOOV9CipbK9l\n"
                      "29pHgeRhi5CAfoq823wjI5nEGpSF39heWq57CdGfXbbX7kZBCdI5zgMhSath1nkW\n"
                      "cdgsX+8plIsyUMRnBuqiNh7Iff0w1Qysbw0VdW2s3KUrTX7QVFP6WF6/uMrnMKhR\n"
                      "xLxBw6j5fnY21M4fL3rr8Yqo8hM9TTFNfyKzloyV9NT2InwU2vsh/BxVZdFwyJgN\n"
                      "703WG8h5wQR4uS48FnkkhMPdtjbqKYZGa3C1my6Tf6FsDSl+0Y9saJoRFErCN2tw\n"
                      "VbzTD7LBfanuiyJGB8iiqLCObeM46+krzNFBiuPGHi+oHAKIoCjRB0ZxwReafxaU\n"
                      "gJ2SyQt5Jooi46tg7wX3ggJ9HHu9RRJCo5wo5i4dWrh2fL0l3SkDRZbjBFlUNxAO\n"
                      "aNkb71/nGviyB5XQC6xLhCESC9WcGNpzB2VL3qjuJEsw25+Udzv1RZnCFsFur5iz\n"
                      "NTZkd0FRSrUSsztNbVmAc6RFrHNd81xloqztkTJUllQSViBCro0R4GUZRo1Yo+MJ\n"
                      "jX6QTgVRQOqtwITQBS/9TGHIzd0jzvjzyjR4vYEBoBoFV/tkhmR3+S7hc8A1NXj8\n"
                      "xcAQEciSlwJGXuvmpQDY6HnHdIchRN2FbCCEMkH5W4cFktpVx4wMwGugwZ2sfblQ\n"
                      "yEZaUSC6a9OcfTZn+psxAoIBAQDoN7BpN9NLl8cxqvvt8n6RFkVfXx/39VtxDsPq\n"
                      "xTUhv83BF9F6XyzpECPEkmPg9r4vND4KD5HgmlhEvhe9sTsGow3pxo1P8sn/7GLK\n"
                      "CwOkUeyX97epBJg1H2jj+++ylGfnfTF2j9jHOhaTnrG8h3Fua2/y0FTxGI4TjNLx\n"
                      "Tk3IS34gyZHBd/GWmtpx33EE0TGlMtwzwAUqCcDt7DRjkoujqgLSNEslXJb2Mp3b\n"
                      "LN1OIQlTYaDLRQ43QWRQSEYtyRrt7N/0slJ2y++aQ10hJbd4SSGkanQ4DBJYI3+e\n"
                      "YHgzz4/N6ZMUegY291RaCE+4tpq6kwn5F3KlvUokvpe4H+ANAoIBAQD0M3xowcYM\n"
                      "gpT7wW5HfHvs2r7H/jivcHIZ+zFWZO7R+1zPxEhh+HwqZ5Jh5PRz5RTaRIGEC+4I\n"
                      "P+vIBwOr0+zEvQ6HBYwGu1D3nNlhcQjo8/Go8zCD8Vb/qsS4ONN1UDIlJwYD9JvM\n"
                      "UwAPTwafkgVNyxF3il0KZIcNR2BAPXn5mWgHyRExL6idIdzLnYfl74159Q1EgWLo\n"
                      "ogh/NAdUTXuMK8cVDiXu1jINEmL4FEeNPh5ZJCM8a9G4dJPmvTYv/WgQzGr+ka1Q\n"
                      "KHfjYVlyuc8/v4EsdWAwPVLj45p5jO1NHoAjp6nIrF4ssnRqW/ad/tJr+3IcGkxA\n"
                      "sOZuFXOLlwX1AoIBAHeNFBlXUES+oyKC8KiUC20OljB/fKnfe3A9NcgMN0pkwL0E\n"
                      "Uig6D5j9Yiy1YQ5UeZ07vCTZ1zRWwdLCOt7B+61zDJqqSd47FJvZSxLRkNJsCu5B\n"
                      "7h/p6aKPxSR6c/DwTn3zCiAL9oK6+RyOutpgsJk0XrIXZ2OoTJzsBz2MHqdblley\n"
                      "7oM4CPyMRKITctAUEwcFpDBQT1hRJS5YSLBqpdFgmYs2uOlHPcEKVhKcimLKx05S\n"
                      "GMINj0ALPmXGj7rKqj6LkNEdbS8T1wsIW03RaR9YJyXKWILyVA9kVzNmXvvthA9y\n"
                      "jPIh0ElEE/02AjUKbaZYRO4kkDWbuiwoo1t7rZECggEBAN0pi/DsHcvSr89aHeBM\n"
                      "U4ftjH9gNEJfmlcBcmuGOeyetC3O3Kct/nYBG0fhrasQu9P52zqV+1wpWAbuo1qJ\n"
                      "MpPzjSan1Wt7VhcLTKy9TI598E1NApFcj6g5JrSZcpl6qg+Pyhjm/fsM6K70qVD3\n"
                      "VCpiUU1sWFtDNDTIgdlsU/3D+72hUhqzyQeWVXtrDlsmm4DqeLukWsnJhdx+aGzn\n"
                      "R49pToI5MA2gm+/OySdYZnz6101wNyFPnL5rhk+g6WSJIiikmfo4QTipjfq2feYz\n"
                      "dgmMDRcQrq7Bu0J9LbgL9Ba2GuMvw8rkoN3mORzicPYihLwbG/trrceg7vCgKOcx\n"
                      "PckCggEBAOCI4GdmYti+Cda5BH553PBa1TFi0L8t0yKA+8g3I3wBhp7mtO1CV58A\n"
                      "+qynTpmpTacxdwEZOInb4wc8Z1COm31bQL/8avQaTvj2uPoo6yLraOcvv/KNcf8e\n"
                      "KBXm8oKTI2lJDzvyehsVTJ8HLf9O0cPiclB8hu3IaEqWnXSeVlu0dLmaTf2DO0QK\n"
                      "MmL6kMJdFjlx/lfR8a+IbLolquWOq5NXq6jzAnQSaEKPVLvj9qhV2PzhhGe6e9qN\n"
                      "dHI9tH90zQYWyu6OL9z1VOenfQYUlSlyFWIJ6zYh1Z6sJATj+WA+Mj3hFoHUOWTe\n"
                      "AxKDdy7qurkwPWr4w8udwVzKUc5pTRs=\n"
                      "-----END PRIVATE KEY-----";
        string cert = "-----BEGIN CERTIFICATE-----\n"
                      "MIIF9zCCA9+gAwIBAgIUCcfI54kAkmK5ZF0I4exDFSgDCNowDQYJKoZIhvcNAQEL\n"
                      "BQAwgYoxCzAJBgNVBAYTAlJVMREwDwYDVQQIDAhGaXJld29sZjERMA8GA1UEBwwI\n"
                      "RmlyZXdvbGYxETAPBgNVBAoMCEZpcmV3b2xmMREwDwYDVQQLDAhGaXJld29sZjER\n"
                      "MA8GA1UEAwwIRmlyZXdvbGYxHDAaBgkqhkiG9w0BCQEWDTEyM0BnbWFpbC5jb20w\n"
                      "HhcNMjMwNjIyMDQ0MjAxWhcNMjQwNjIxMDQ0MjAxWjCBijELMAkGA1UEBhMCUlUx\n"
                      "ETAPBgNVBAgMCEZpcmV3b2xmMREwDwYDVQQHDAhGaXJld29sZjERMA8GA1UECgwI\n"
                      "RmlyZXdvbGYxETAPBgNVBAsMCEZpcmV3b2xmMREwDwYDVQQDDAhGaXJld29sZjEc\n"
                      "MBoGCSqGSIb3DQEJARYNMTIzQGdtYWlsLmNvbTCCAiIwDQYJKoZIhvcNAQEBBQAD\n"
                      "ggIPADCCAgoCggIBAN2DyBZuA8dYk7TNhR+YacKrJvmNPAFl+tr/ZU5bN1d3yG3E\n"
                      "oY0kARDHct3dWXycrP4ZZ1kZgObxZrYM4m3QRbFF+FjyJ5cbDuBqce4vweO6GP87\n"
                      "xogHhdomlgVpLCVJ47/t5+N+BgLXkw8pe3AFIngT5zsDDM6qWYnVCzLGp8s3bd3O\n"
                      "7qz4uJsLidrjZvG17Vt2QxoKN6TqKXn/MlflaiOhPetm+wuqx64dp6OCgJ+xpsRB\n"
                      "iM5xq9ASm7ZFXRtiqCrfgDP2FpthSkwRaemDS9chc6GuigWhN9thNy+TsFvgp5HM\n"
                      "c/jOaaQgVksTgH/2XQVhVOVA0X9v7AJYuhZwapTOf/Yx3hx22f7IfynGtadxM+Ko\n"
                      "vRfsJ2llWcmUQvYbqCSEbemltvRHuNgkerva4nCfziUnDb+WZEMd2pGg5kB361z+\n"
                      "YMlmyOg8+WpC104URz0ZqgQmxqzNw15PaVe2SLnnDiXqGaacE9hX0ZhkY7JcMCrU\n"
                      "MBe4tqCrDyNIgM4iN3L5/R9sm9d1B23a0+Kfqp3pnlEvub3RGBkACVGZ6Kes2vyo\n"
                      "uFAT0qZfvsrEuYhRrPzhN0LyEYIjDvTtNMw5mI7oMSdhMEo/kOqw9E1hxoqgWzy7\n"
                      "ZB62blfy1/TYdSg/6il/jQxZOYH8GsxNkRvjnvJfRn+UImkZ88Ci/hcMjK1xAgMB\n"
                      "AAGjUzBRMB0GA1UdDgQWBBQswY6v6vJL39/9tO8w7Gd8VcRG/TAfBgNVHSMEGDAW\n"
                      "gBQswY6v6vJL39/9tO8w7Gd8VcRG/TAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3\n"
                      "DQEBCwUAA4ICAQAGGyivHIz0Z3wM05fXjE9gKc4oHCygKIyH8xk9Go4UnaQlrm08\n"
                      "BhYeqkwV+CZS6cRmsFgbsPbczEW7i7Hh0qG6cBNTjAEDPyj4ZsvGrdBOYBTw+zai\n"
                      "2gg+ipNj4Lbg8q3vVUjJwG3CoSMABrnXOiU7BdkpheK9IOS/7YQoJlALvTl1OkJX\n"
                      "HH3UaSjAb72kXQa4t+HTcZuliXRBmp+3IECo4IBG06rykYQV2pu9siZE16xrPV10\n"
                      "jRMapm0Y6gPZyiakFpfhnQeQy1/t5BUxZM5q7WvEdbAmj21QbsfcjlA3jBeGwJpD\n"
                      "DBonplLgHVI/y/UJiWgn/Nd1f99dQp7DR0D5pUDbUWzWH6ATIR0bnttlXUQwN/qp\n"
                      "xu57I9qSWdsCjor8PTa4RV7kFl25W1BT8KvPRFnqAjiq/uPwEofNlZ2HIdvkJ9y5\n"
                      "zimnYZJEKovQICFdB3PJYcyV/McZy3zGhhOecCL0CWoDElB0ifBaoe4NFW0M6VwD\n"
                      "o2JhEKjtB1JinfQytj2coF7noFts/f1yu5h/7LI2pl/oXzNbvPV3igkDkCmabLrY\n"
                      "PYHFIT8hawlbUeRWW/PYxfScB2o1kn492bx51x2HQjGu1fnZiANpLyuMu4wIZJ+D\n"
                      "98Bpwl8inPh++3Kodmv2FY32sRwFxDnMpnoqshsW/csvD1DA+yE/ZWgnEw==\n"
                      "-----END CERTIFICATE-----";
    };
}