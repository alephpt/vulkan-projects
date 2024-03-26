#pragma once
#include <iostream>
#include <string>

enum LOGGER {
    OFF,
    ERROR,
    INFO,
    DEBUG,
    VERBOSE
};

void report(LOGGER log_level, std::string message);