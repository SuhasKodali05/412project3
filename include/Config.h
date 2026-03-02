#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <string>
#include <vector>

/**
 * @file Config.h
 * @brief Configuration loader for the load balancer simulation.
 */

/**
 * @struct IpRange
 * @brief Closed interval range of IPv4 addresses [start, end].
 */
struct IpRange {
    uint32_t start{0};
    uint32_t end{0};
};

/**
 * @class Config
 * @brief Reads key=value configuration from a text file.
 *
 * Supports IP block rules via:
 * - CIDR:  "192.168.0.0/24"
 * - Range: "10.0.0.1-10.0.0.200"
 * - Single IP: "8.8.8.8"
 */
class Config {
public:
    /**
     * @brief Load configuration from a file.
     * @param path Path to config file
     * @param errMsg Output error message on failure
     * @return True on success, false on failure
     */
    bool load(const std::string& path, std::string& errMsg);

    int initialServers{10};
    int minServers{1};
    int totalCycles{10000};
    int initialQueuePerServer{100};

    int lowQueuePerServer{50};
    int highQueuePerServer{80};
    int scaleCooldownCycles{25};

    int arrivalChancePercent{20};
    int minProcessTime{2};
    int maxProcessTime{30};
    int streamingChancePercent{30};

    std::string logFile{"lb.log"};
    bool echoToConsole{true};
    uint32_t rngSeed{12345};

    std::vector<IpRange> blockedRanges;

    /**
     * @brief Check whether an IP is blocked by any configured range.
     * @param ip IPv4 address as uint32 (host order)
     * @return True if blocked, false otherwise
     */
    bool isBlocked(uint32_t ip) const;

private:
    static std::string trim(const std::string& s);
    static std::vector<std::string> split(const std::string& s, char delim);

    static bool parseBool(const std::string& v, bool& out);
    static bool parseInt(const std::string& v, int& out);
    static bool parseUint32(const std::string& v, uint32_t& out);

    static bool parseIpRangeToken(const std::string& token, IpRange& outRange, std::string& errMsg);
    static bool parseCidr(const std::string& token, IpRange& outRange, std::string& errMsg);
    static bool parseStartEnd(const std::string& token, IpRange& outRange, std::string& errMsg);
};

#endif