//
// Created by firewolf304 on 13.11.23.
//
#include "../include/variables.hpp"


inline void testist() {
    std::cout << "TESTER: " << app.access["/apis/monitor"]["path"].get<string>() << std::endl;
}