#ifndef MODULES_LIBRARY_H
#define MODULES_LIBRARY_H
#include <string>
#include <nlohmann/json.hpp>
#include "scripts/variables.h"
#define __VERSION__ "3"
#define __AUTHOR__ "FIREWOLF"
#define __COMMENT__ "this is a test module"

extern "C" const char* test();
void start();

#endif //MODULES_LIBRARY_H
