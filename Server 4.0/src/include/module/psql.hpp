//
// Created by firewolf304 on 01.04.24.
//

#ifndef SERVER4_0_PSQL_HPP
#define SERVER4_0_PSQL_HPP
#include "../include.hpp"
namespace firewolf::sql {
    using namespace nlohmann;
    /* elementary config like base constructor pqxx:
        sql_dump server(json::parse(R"(
        {
            "user" : "1234",
            "password" : "1234",
            "host" : "127.0.0.1",
            "port" : "1234",
            "dbname" : "1234",
            "target_session_attrs" : "read-write"
        }
        )"));
    */
    class sql_dump {
    private:
        json config;
        std::shared_ptr<pqxx::connection> client; // add dynamic value
    public:
        sql_dump () { }
        sql_dump (json configurate) : config(configurate) {
            this->config = configurate;
            std::string request = "";
            for(auto data = config.begin(); data != config.end(); data++) {
                request += data.key() + "=" + data.value().get<std::string>() + " ";
            }
            //request = "user=firewolf password=1633 host=127.0.0.1 port=8081 dbname=server_bot target_session_attrs=read-write";
            try {

                this->client = make_shared<pqxx::connection>(request);
                pqxx::work work(*this->client);
            } catch (const std::exception &e) {
                throw std::runtime_error("Error connection (" + std::string(e.what()) + ")");
            }
        }
        void configure(json const & configurate) {
            this->config = configurate;
            std::string request = "";
            for(auto data = config.begin(); data != config.end(); data++) {
                request += data.key() + "=" + data.value().get<std::string>() + " ";
            }
            try {
                this->client = make_shared<pqxx::connection>(request);
                pqxx::work work(*this->client);
            } catch (const std::exception &e) {
                throw std::runtime_error("Error connection (" + std::string(e.what()) + ")");
            }
        }
        std::unique_ptr<pqxx::connection> configure() {
            std::string request = "";
            for(auto data = this->config.begin(); data != this->config.end(); data++) {
                request += data.key() + "=" + data.value().get<std::string>() + " ";
            }
            try {
                return make_unique<pqxx::connection>(request);
            } catch (const std::exception &e) {
                throw std::runtime_error("Error connection (" + std::string(e.what()) + ")");
            }
        }
        void disconnect() {
            if(this->client->is_open()) {
                this->client->close();
            }
        }
        ~sql_dump() {
            disconnect();
        }
        bool is_alive() {
            if(this->client != nullptr && client->sock() > 0) {
                if(this->client->is_open()) {
                    try {
                        return client->is_open();
                    } catch (std::runtime_error const &e) {
                        return false;
                    }
                }
            }
            return false;
        }
        // exec without save
        pqxx::result operator < ( const std::string & text )  {
            try {
                pqxx::work work(*this->client);
                auto result = work.exec(text);
                work.abort();
                //this->work->commit();
                return result;
            }
            catch (const std::exception &e) {
                throw std::runtime_error(e.what());
            }
        }
        // exec with save
        pqxx::result operator << ( const std::string & text ) {
            try {
                if(!is_alive()) throw std::runtime_error("SQL not alive");;
                auto active_conn = configure();
                pqxx::work worker (*active_conn);
                auto result = worker.exec(text);
                worker.commit();
                return result;
            }
            catch (const std::exception &e) {
                throw std::runtime_error(e.what());
            }
        }
        /*template<typename... T>
        pqxx::result operator<<(const std::tuple<T...>& values)

        {

            //return os;
        }*/
        class data {
        public:
            data(sql_dump* a) {
                sql_return = a;

            }
            struct types {
                class type {
                public:
                    enum class values : int;
                    std::vector<std::string> string_value;
                    template <typename t>
                    constexpr auto to_int(t e) noexcept
                    {
                        return static_cast<int>(e);
                    }
                    template <typename t>
                    bool is_not_none(t e) {
                        return (bool)to_int(e);
                    }
                    template <typename t>
                    std::string to_string(t value) {
                        return string_value[to_int(value)];
                    }
                };

                class global_type : public type {
                public:
                    global_type() {
                        string_value = {"NONE", "GLOBAL", "LOCAL"};
                    }
                    enum values : int {
                        NONE = 0,
                        GLOBAL = 1,
                        LOCAL = 2
                    };
                    //static vector<string> string_value = {"NONE", "GLOBAL", "LOCAL"};

                };
                class temp_type : public type {
                public:
                    temp_type() {
                        string_value = {"NONE", "TEMP", "TEMPORARY"};
                    }
                    enum values : int {
                        NONE = 0,
                        TEMP = 1,
                        TEMPORARY = 2
                    };
                    //vector<string> string_value = {"NONE", "TEMP", "TEMPORARY"};
                };
                class on_commit : public type {
                public:
                    on_commit() {
                        string_value = {"NONE", "PRESERVE ROWS", "DELETE ROWS", "DROP"};
                    }
                    enum values : int {
                        NONE = 0,
                        PRESERVE_ROWS = 1,
                        DELETE_ROWS = 2,
                        DROP = 3
                    };
                    //vector<string> string_value = {"NONE", "PRESERVE ROWS", "DELETE ROWS", "DROP"};
                };
                global_type globalType;
                temp_type tempType;
                on_commit onCommit;
            };
            void create_table(std::string name,
                              std::vector<std::string> table_data,
                              bool unlogged = false,
                              types::global_type::values global_type = types::global_type::NONE,
                              types::temp_type::values temp_type = types::temp_type::NONE,
                              types::on_commit::values on_commit = types::on_commit::NONE,
                              std::string tablespace = "",
                              std::string with = "")
            {
                //string text = types::global_type().string_value[global_type] + " " + (types::global_type().is_not_none(global_type) ? "true" : "false");
                std::string text = "CREATE TABLE " + name + " " +
                              (types::global_type().is_not_none(global_type) ? types::global_type().to_string(global_type) + " " : "") +
                              (types::temp_type().is_not_none(temp_type) ? types::temp_type().to_string(temp_type) + " " : "") +
                              (unlogged ? "UNLOGGED " : "");
                text += "(\n";
                if(!table_data.empty()) {
                    for(std::string d : table_data) {
                        text += "  " + d + "\n";
                    }
                } text += ")" + (!tablespace.empty() ? " TABLESPACE " + tablespace + " " : "") + (!with.empty() ? "WITH (" + with + ") " : "") +
                          (types::on_commit().is_not_none(on_commit) ? " ON COMMIT " + types::on_commit().to_string(on_commit) + " " : "");

                text += ";";
                //this->sql_return->work->exec(text);
            }
            void ListOFTables(std::string name) {
                std::string command = "create table out(bd_name text, schema_name text, table_name text, column_name text, data_type text);\n"
                                 "truncate out;\n"
                                 "do $$\n"
                                 "declare\n"
                                 "db text;\n"
                                 "dbb text;\n"
                                 "begin\n"
                                 "   create table temparr as select array(select datname from pg_catalog.pg_database where datname != 'postgres' and datname != 'template0' and datname != 'template1') as arr;\n"
                                 "   for i in 1..cardinality(arr) from temparr loop\n"
                                 "   db = arr[i] from temparr;\n"
                                 "   raise notice 'Insert into table: %', db;\n"
                                 "   dbb = 'dbname='||db;\n"
                                 "   insert into out select db, *\n"
                                 "   from dblink(dbb, 'select table_schema, table_name, column_name, data_type from information_schema.columns where table_schema =     ''public'' order by table_name, column_name') as (schema_name text, table_name text, column_name text, data_type text);\n"
                                 "   end loop;\n"
                                 "end $$;";

            }
        private:
            sql_dump* sql_return;
        };

        data methods = data(this );
    };
}
#endif //SERVER4_0_PSQL_HPP
