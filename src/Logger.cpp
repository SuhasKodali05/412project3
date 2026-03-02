#include "Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

static std::string nowTimestamp() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

Logger::Logger(const std::string& filepath, bool echoToConsole)
    : echo_(echoToConsole) {
    file_.open(filepath, std::ios::out);
    if (file_) {
        writeLine("=== Log started: " + nowTimestamp() + " ===");
    }
}

Logger::~Logger() {
    if (file_) {
        writeLine("=== Log ended: " + nowTimestamp() + " ===");
        file_.close();
    }
}

void Logger::setEcho(bool enabled) { echo_ = enabled; }

void Logger::writeLine(const std::string& line) {
    if (file_) file_ << line << "\n";
    if (echo_) std::cout << line << "\n";
}

void Logger::raw(const std::string& msg) { writeLine(msg); }

void Logger::info(const std::string& msg) {
    writeLine("[INFO " + nowTimestamp() + "] " + msg);
}

void Logger::warn(const std::string& msg) {
    writeLine("[WARN " + nowTimestamp() + "] " + msg);
}

void Logger::error(const std::string& msg) {
    writeLine("[ERR  " + nowTimestamp() + "] " + msg);
}

static std::string wrap(const std::string& code, const std::string& s) {
    return code + s + "\033[0m";
}
std::string Logger::green(const std::string& s) { return wrap("\033[32m", s); }
std::string Logger::yellow(const std::string& s) { return wrap("\033[33m", s); }
std::string Logger::red(const std::string& s) { return wrap("\033[31m", s); }
std::string Logger::cyan(const std::string& s) { return wrap("\033[36m", s); }
std::string Logger::magenta(const std::string& s) { return wrap("\033[35m", s); }
std::string Logger::dim(const std::string& s) { return wrap("\033[2m", s); }