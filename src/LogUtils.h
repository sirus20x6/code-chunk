#pragma once

#include <iostream>
#include <sstream>
#include <string_view>

namespace LogUtils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    template<typename... Args>
    static void log(LogLevel level, Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        std::string message = oss.str();
        std::string prefix;
        std::string color;

        switch (level) {
            case LogLevel::DEBUG:
                prefix = "[DEBUG] ";
                color = "\033[36m"; // Cyan
                break;
            case LogLevel::INFO:
                prefix = "[INFO] ";
                color = "\033[32m"; // Green
                break;
            case LogLevel::WARNING:
                prefix = "[WARNING] ";
                color = "\033[33m"; // Yellow
                break;
            case LogLevel::ERROR:
                prefix = "[ERROR] ";
                color = "\033[31m"; // Red
                break;
        }

        std::cout << color << prefix << message << "\033[0m\n";
    }

    template<typename... Args>
    static void debug(Args&&... args) {
        log(LogLevel::DEBUG, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(Args&&... args) {
        log(LogLevel::INFO, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warning(Args&&... args) {
        log(LogLevel::WARNING, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(Args&&... args) {
        log(LogLevel::ERROR, std::forward<Args>(args)...);
    }
};

} // namespace LogUtils