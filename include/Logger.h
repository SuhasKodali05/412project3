#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

/**
 * @file Logger.h
 * @brief Logging utility for console + file output.
 */

/**
 * @class Logger
 * @brief Simple logger that writes to a file and optionally to console.
 *
 * Also supports ANSI color codes for terminal output.
 */
class Logger {
public:
    /**
     * @brief Construct logger.
     * @param filepath Log file path
     * @param echoToConsole Print to console too
     */
    Logger(const std::string& filepath, bool echoToConsole);

    /** @brief Close file if open. */
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /** @brief Log an info message. */
    void info(const std::string& msg);

    /** @brief Log a warning message. */
    void warn(const std::string& msg);

    /** @brief Log an error message. */
    void error(const std::string& msg);

    /** @brief Log a raw line (no prefix). */
    void raw(const std::string& msg);

    /** @brief Enable/disable console echo. */
    void setEcho(bool enabled);

    /** @brief ANSI color helpers. */
    static std::string green(const std::string& s);
    static std::string yellow(const std::string& s);
    static std::string red(const std::string& s);
    static std::string cyan(const std::string& s);
    static std::string magenta(const std::string& s);
    static std::string dim(const std::string& s);

private:
    std::ofstream file_;
    bool echo_{true};

    void writeLine(const std::string& line);
};

#endif