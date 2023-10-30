#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <nlohmann/json.hpp>
#include <future>
using namespace std;

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
        shared_ptr<pqxx::work> work;
        shared_ptr<pqxx::connection> client; // add dynamic value
    public:
        sql_dump () { }
        sql_dump (json configurate) {
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
        void configure(json configurate) {
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
        void connect(json data)  { // request to server: return non negative value
            string request = "";
            for(auto data = config.begin(); data != config.end(); data++) {
                request += data.key() + "=" + data.value().get<string>() + " ";
            }
            //request = "user=firewolf password=1633 host=127.0.0.1 port=8081 dbname=server_bot target_session_attrs=read-write";
            try {
                //this->client = pqxx::connection(request);
            } catch (const exception &e) {
                cout << "Error connection: " << e.what() << endl;
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
        pqxx::result operator<< ( const string & text ) {
            try {
                return this->work->exec(text);
            }
            catch (const exception &e) {
                throw std::runtime_error(e.what());
            }
        }
        template<typename... T>
        pqxx::result operator<<(const std::tuple<T...>& values)

        {

            //return os;
        }
        class data {
        public:
            data(sql_dump* a) {
                sql_return = a;

            }
            struct types {
                class type {
                public:
                    enum class values : int;
                    vector<string> string_value;
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
                    string to_string(t value) {
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
            void create_table(string name,
                              vector<string> table_data,
                              bool unlogged = false,
                              types::global_type::values global_type = types::global_type::NONE,
                              types::temp_type::values temp_type = types::temp_type::NONE,
                              types::on_commit::values on_commit = types::on_commit::NONE,
                              string tablespace = "",
                              string with = "")
            {
                //string text = types::global_type().string_value[global_type] + " " + (types::global_type().is_not_none(global_type) ? "true" : "false");
                string text = "CREATE TABLE " + name + " " +
                              (types::global_type().is_not_none(global_type) ? types::global_type().to_string(global_type) + " " : "") +
                              (types::temp_type().is_not_none(temp_type) ? types::temp_type().to_string(temp_type) + " " : "") +
                              (unlogged ? "UNLOGGED " : "");
                text += "(\n";
                if(!table_data.empty()) {
                    for(string d : table_data) {
                        text += "  " + d + "\n";
                    }
                } text += ")" + (!tablespace.empty() ? " TABLESPACE " + tablespace + " " : "") + (!with.empty() ? "WITH (" + with + ") " : "") +
                          (types::on_commit().is_not_none(on_commit) ? " ON COMMIT " + types::on_commit().to_string(on_commit) + " " : "");

                text += ";";
                this->sql_return->work->exec(text);
            }
            void ListOFTables(string name) {
                string command = "create table out(bd_name text, schema_name text, table_name text, column_name text, data_type text);\n"
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

/*using namespace firewolf::sql;
void dd (vector<string> mass) {
    cout<< mass.empty() << endl;
}
int main() {
    std::cout << "Hello, World!" << std::endl;
    try {
        //shared_ptr<pqxx::connection> client;
        //client = make_shared<pqxx::connection>("user=firewolf password=1633 host=127.0.0.1 port=8081 dbname=server_bot target_session_attrs=read-write");
        //sql_dump server("user=firewolf password=1633 host=127.0.0.1 port=8081 dbname=server_bot target_session_attrs=read-write");
        sql_dump server(json::parse(R"(
        {
            "user" : "firewolf",
            "password" : "1633",
            "host" : "127.0.0.1",
            "port" : "8081",
            "dbname" : "server_bot",
            "target_session_attrs" : "read-write"
        }
        )"));
        auto dda = static_cast<void*>(0);
        using types = sql_dump::data::types;
        server.methods.create_table("test", {"name\ttext", "id\tinteger"},
                                    false,
                                    types::global_type::GLOBAL,
                                    types::temp_type::TEMP,
                                    types::on_commit::NONE,
                                    "data");
        //server.methods.create_table();
    }
    catch (const exception &e) {
        cout << "Error: " << e.what() << endl;
    }

    enum class data : int {
        LOG = 123
    };
    cout << static_cast<int>(data::LOG) << endl;
    return 0;
}*/
