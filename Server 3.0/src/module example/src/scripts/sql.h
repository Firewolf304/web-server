//
// Created by firewolf304 on 02.11.23.
//

#ifndef MODULES_SQL_H
#define MODULES_SQL_H
#include <string>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
namespace firewolf::sql {
    class sql_dump {
    private:
        nlohmann::json config;
        std::shared_ptr<pqxx::connection> client; // add dynamic value
        std::shared_ptr<pqxx::work> work;
        ~sql_dump();
        class data {
        public:
            data(sql_dump* a);
            struct types {
                class type {
                public:
                    enum class values : int;
                    std::vector<std::string> string_value;

                    template<typename t>
                    int to_int(t e);

                    template<typename t>
                    bool is_not_none(t e);

                    template<typename t>
                    std::string to_string(t value);
                };
                class global_type : public type {
                public:
                    global_type();
                    enum values : int {
                        NONE = 0,
                        GLOBAL = 1,
                        LOCAL = 2
                    };

                };
                class temp_type : public type {
                public:
                    temp_type();
                    enum values : int {
                        NONE = 0,
                        TEMP = 1,
                        TEMPORARY = 2
                    };

                };
                class on_commit : public type {
                public:
                    on_commit();
                    enum values : int {
                        NONE = 0,
                        PRESERVE_ROWS = 1,
                        DELETE_ROWS = 2,
                        DROP = 3
                    };
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
                              std::string with = "");
            void insert_table(std::string name);



        private:
            sql_dump* sql_return;
        };

        data methods = data(this );
    };
}
#endif //MODULES_SQL_H
