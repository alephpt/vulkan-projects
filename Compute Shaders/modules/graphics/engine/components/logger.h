#pragma once

enum LOGGER {
    OFF,
    ERROR,
    ILINE,
    INFO,
    DLINE,
    DEBUG,
    VLINE,
    VERBOSE
};

static LOGGER LOG_LEVEL = LOGGER::VERBOSE;

void report(LOGGER log_level, const char* message, ...);