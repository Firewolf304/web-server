#include "../../include/modules/sql.hpp"

namespace firewolf::sql {
    sql_dump::sql_dump(nlohmann::json configurate) : config(configurate) {
        this->config = configurate;
        string request = "";
        for(auto data = config.begin(); data != config.end(); data++) {
            request += data.key() + "=" + data.value().get<string>() + " ";
        }
        //request = "user=firewolf password=1633 host=127.0.0.1 port=8081 dbname=server_bot target_session_attrs=read-write";
        try {
            this->client = make_shared<pqxx::connection>(request);
            this->work = make_shared<pqxx::work>(*(this->client));
        } catch (const exception &e) {
            throw std::runtime_error("Error connection");
        }
    }
    void sql_dump::configure(nlohmann::json configurate) {
        this->config = configurate;
        string request = "";
        for(auto data = config.begin(); data != config.end(); data++) {
            request += data.key() + "=" + data.value().get<string>() + " ";
        }
        try {
            this->client = make_shared<pqxx::connection>(request);
            this->work = make_shared<pqxx::work>(*(this->client));
        } catch (const exception &e) {
            throw std::runtime_error("Error connection");
        }
    }
}