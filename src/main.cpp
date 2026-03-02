#include "Config.h"
#include "LoadBalancer.h"
#include "Logger.h"

#include <iostream>
#include <string>

/**
 * @file main.cpp
 * @brief Driver program for the Load Balancer simulation.
 *
 * Requirements met here:
 * - user input: number of servers and total cycles
 * - generates a full initial queue (servers * 100 by default via config)
 * - runs the simulation and logs summary
 */

static int askInt(const std::string& prompt, int fallback) {
    std::cout << prompt << " (default " << fallback << "): ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return fallback;
    try 
    {
        return std::stoi(line);
    } 
    catch (...) 
    {
        return fallback;
    }
}

int main() 
{
    Config cfg;
    std::string err;
    if (!cfg.load("config.txt", err)) 
    {
        std::cerr << "Failed to load config.txt: " << err << "\n";
        std::cerr << "Fix config.txt and try again.\n";
        return 1;
    }

    cfg.initialServers = askInt("Enter number of servers: ", cfg.initialServers);
    cfg.totalCycles = askInt("Enter number of clock cycles: ", cfg.totalCycles);

    Logger logger(cfg.logFile, cfg.echoToConsole);
    logger.info("Config loaded. log_file=" + cfg.logFile);

    if (!cfg.blockedRanges.empty()) 
    {
        logger.info("IP blocker enabled with " + std::to_string(cfg.blockedRanges.size()) + " range(s).");
    } 
    else 
    {
        logger.warn("IP blocker has no ranges configured.");
    }

    LoadBalancer lb(cfg, logger);
    lb.fillInitialQueue();
    lb.run();

    return 0;
}