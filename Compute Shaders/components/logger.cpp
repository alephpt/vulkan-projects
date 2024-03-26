#include "logger.h"

static const LOGGER LOG_LEVEL = LOGGER::DEBUG;

void report(LOGGER log_level, std::string message) {
    if (log_level < LOG_LEVEL) {
        std::cout << message << std::endl;
    }

    return;
}