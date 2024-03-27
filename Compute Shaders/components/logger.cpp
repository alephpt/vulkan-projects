#include "logger.h"

#include <iostream>
#include <cstdarg>


static bool letsGo(LOGGER level) {
    return level <= LOG_LEVEL;
}

void report(LOGGER log_level, const char* format, ...){
        std::va_list args;
        va_start(args, format);

        if (letsGo(log_level)) {
            switch(log_level) {
                case LOGGER::ILINE:
                case LOGGER::DLINE:
                case LOGGER::VLINE:
                    std::cout << " \t ";
                    break;
                case LOGGER::ERROR:
                    std::cout << " [ERROR]: ";
                    break;
                case LOGGER::INFO:
                    std::cout << " [INFO]: ";
                    break;
                case LOGGER::DEBUG:
                    std::cout << " [DEBUG]: ";
                    break;
                case LOGGER::VERBOSE:
                    std::cout << " [VERBOSE]: ";
                    break;
                default:
                    break;
            }

            vprintf(format, args);
            std::cout << std::endl;
        }
        va_end(args);

    return;
}