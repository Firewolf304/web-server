//
// Created by firewolf304 on 23.03.24.
//

#ifndef SERVER4_0_RESTAPI_HPP
#define SERVER4_0_RESTAPI_HPP
#include "../include.hpp"
namespace firewolf::web_server::restapi {
    typedef boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;
    struct r {
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
        } ;
    public:
        std::string header_body;
        std::string version = "1.1";
        int code_status = 200;
        std::string custom_comment;
        std::string body_text;
    };
    typedef boost::beast::http::verb meth;
    struct req {
    public:
        meth method = boost::beast::http::verb::unknown;
        std::string path = "/";
        unsigned int version;
        struct b {
            nlohmann::json headers;
            std::string body;
        } body;
    };


    class [[nodiscard("is old")]] RestAPI {
        std::shared_ptr<logger> log;
    public:
        req request;
        r response;
        RestAPI() = default;
        explicit RestAPI(std::shared_ptr<logger> log) : log(std::move(log)) {  }
        void get_request(std::string const & buffer) {
            boost::beast::http::request<boost::beast::http::dynamic_body> req;
            boost::beast::error_code error;
            boost::beast::http::request_parser<boost::beast::http::string_body> parser;
            parser.put(boost::asio::buffer(buffer), error);
            if(error) {
                throw std::runtime_error("Put buffer error (" + error.to_string() + ")" );
            }
            auto res = parser.release();
            this->request.method = req.method();
            this->request.path = res.target();
            this->request.version = res.version();
            for (const auto& header : res.base()) {
                this->request.body.headers[header.name_string()] = header.value();
            }
            if(error) {
                throw std::runtime_error("Read buffer error (" + error.to_string() + ")" );
            }
        }
        void get_request(boost::beast::http::request<boost::beast::http::dynamic_body> req) {
            this->request.method = req.method();
            this->request.path = req.target();
            this->request.version = req.version();
            for (const auto& header : req.base()) {
                this->request.body.headers[header.name_string()] = header.value();
            }
        }

        void make_response () {

        }

        std::string make_response_sock(){
            return "HTTP/"+ response.version +" "+std::to_string(response.code_status)+" "+(response.custom_comment.empty() ? response.code_comment[response.code_status] : response.custom_comment) + "\r\n" + response.header_body +
                   "Content-Length: " + std::to_string(response.body_text.length()) + "\r\n\r\n" + response.body_text;
        }
    };
    template <class body_type, int size = 8192>
    class RestAPI_boost {
        std::shared_ptr<logger> log;
    public:
        std::shared_ptr<boost::beast::http::request<body_type>> request;
        boost::beast::http::response<body_type> response;
        boost::beast::flat_buffer buffer_{size};
        RestAPI_boost() {
            init();
        }
        explicit RestAPI_boost(std::shared_ptr<logger> log ) : log(std::move(log)) { init(); }
        explicit RestAPI_boost(std::shared_ptr<logger> log, std::shared_ptr<boost::beast::http::request<body_type>> request ) : log(std::move(log)), request(std::move(request)) { init(); }
        void init() {
            this->request = std::make_shared<boost::beast::http::request<body_type>>();
            //this->response = std::make_shared<boost::beast::http::response<body_type>>();
        }
        void get_request(std::string const & buffer) {
            boost::beast::http::request<boost::beast::http::dynamic_body> req;
            boost::beast::error_code error;
            boost::beast::http::request_parser<boost::beast::http::dynamic_body> parser;
            parser.put(boost::asio::buffer(buffer), error);
            if (error) {
                throw std::runtime_error("Put buffer error (" + error.to_string() + ")");
            }
            this->request = std::make_shared<boost::beast::http::request<body_type>>(parser.release());
            if (error) {
                throw std::runtime_error("Read buffer error (" + error.to_string() + ")");
            }
        }

        void make_response () {


            this->response.content_length(this->response.body().size()); //end
        }
    };
}

#endif //SERVER4_0_RESTAPI_HPP
