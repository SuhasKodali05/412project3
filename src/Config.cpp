#include "Config.h"
#include "Request.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>

/**
 * @file Config.cpp
 * @brief Implementation of configuration parsing and IP blocking rules.
 */

std::string Config::trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

std::vector<std::string> Config::split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == delim) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

bool Config::parseBool(const std::string& v, bool& out) {
    std::string t;
    t.reserve(v.size());
    for (char c : v) t.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    t = trim(t);
    if (t == "1" || t == "true" || t == "yes" || t == "on") { out = true; return true; }
    if (t == "0" || t == "false" || t == "no" || t == "off") { out = false; return true; }
    return false;
}

bool Config::parseInt(const std::string& v, int& out) {
    try {
        std::string tv = trim(v);
        size_t idx = 0;
        int x = std::stoi(tv, &idx);
        if (idx != tv.size()) return false;
        out = x;
        return true;
    } catch (...) {
        return false;
    }
}

bool Config::parseUint32(const std::string& v, uint32_t& out) {
    try {
        std::string tv = trim(v);
        size_t idx = 0;
        unsigned long x = std::stoul(tv, &idx);
        if (idx != tv.size()) return false;
        if (x > 0xFFFFFFFFul) return false;
        out = static_cast<uint32_t>(x);
        return true;
    } catch (...) {
        return false;
    }
}

bool Config::parseStartEnd(const std::string& token, IpRange& outRange, std::string& errMsg) {
    auto parts = split(token, '-');
    if (parts.size() != 2) return false;
    try {
        uint32_t a = ipToUint(trim(parts[0]));
        uint32_t b = ipToUint(trim(parts[1]));
        outRange.start = std::min(a, b);
        outRange.end = std::max(a, b);
        return true;
    } catch (const std::exception& e) {
        errMsg = e.what();
        return false;
    }
}

bool Config::parseCidr(const std::string& token, IpRange& outRange, std::string& errMsg) {
    auto parts = split(token, '/');
    if (parts.size() != 2) return false;

    try {
        uint32_t base = ipToUint(trim(parts[0]));
        int bits = 0;
        if (!parseInt(parts[1], bits) || bits < 0 || bits > 32) {
            errMsg = "Invalid CIDR bits in token: " + token;
            return false;
        }

        uint32_t mask = (bits == 0) ? 0u : (0xFFFFFFFFu << (32 - bits));
        uint32_t network = base & mask;
        uint32_t broadcast = network | (~mask);

        outRange.start = network;
        outRange.end = broadcast;
        return true;
    } catch (const std::exception& e) {
        errMsg = e.what();
        return false;
    }
}

bool Config::parseIpRangeToken(const std::string& token, IpRange& outRange, std::string& errMsg) {
    std::string t = trim(token);
    if (t.empty()) return false;

    if (t.find('/') != std::string::npos) {
        return parseCidr(t, outRange, errMsg);
    }
    if (t.find('-') != std::string::npos) {
        return parseStartEnd(t, outRange, errMsg);
    }

    try {
        uint32_t ip = ipToUint(t);
        outRange.start = ip;
        outRange.end = ip;
        return true;
    } catch (const std::exception& e) {
        errMsg = e.what();
        return false;
    }
}

bool Config::load(const std::string& path, std::string& errMsg) {
    std::ifstream in(path);
    if (!in) {
        errMsg = "Could not open config file: " + path;
        return false;
    }

    blockedRanges.clear();

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        std::string t = trim(line);
        if (t.empty() || t[0] == '#') continue;

        auto eqPos = t.find('=');
        if (eqPos == std::string::npos) {
            errMsg = "Config parse error at line " + std::to_string(lineNo) + ": missing '='";
            return false;
        }

        std::string key = trim(t.substr(0, eqPos));
        std::string val = trim(t.substr(eqPos + 1));

        if (key == "initial_servers") { if (!parseInt(val, initialServers)) goto bad; }
        else if (key == "min_servers") { if (!parseInt(val, minServers)) goto bad; }
        else if (key == "total_cycles") { if (!parseInt(val, totalCycles)) goto bad; }
        else if (key == "initial_queue_per_server") { if (!parseInt(val, initialQueuePerServer)) goto bad; }

        else if (key == "low_queue_per_server") { if (!parseInt(val, lowQueuePerServer)) goto bad; }
        else if (key == "high_queue_per_server") { if (!parseInt(val, highQueuePerServer)) goto bad; }
        else if (key == "scale_cooldown_cycles") { if (!parseInt(val, scaleCooldownCycles)) goto bad; }

        else if (key == "arrival_chance_percent") { if (!parseInt(val, arrivalChancePercent)) goto bad; }
        else if (key == "min_process_time") { if (!parseInt(val, minProcessTime)) goto bad; }
        else if (key == "max_process_time") { if (!parseInt(val, maxProcessTime)) goto bad; }
        else if (key == "streaming_chance_percent") { if (!parseInt(val, streamingChancePercent)) goto bad; }

        else if (key == "log_file") { logFile = val; }
        else if (key == "echo_to_console") { if (!parseBool(val, echoToConsole)) goto bad; }
        else if (key == "rng_seed") { if (!parseUint32(val, rngSeed)) goto bad; }

        else if (key == "blocked_ranges") {
            auto tokens = split(val, ',');
            for (const auto& tok : tokens) {
                IpRange r;
                std::string e;
                if (!parseIpRangeToken(tok, r, e)) {
                    errMsg = "Invalid blocked range token '" + trim(tok) + "': " + e;
                    return false;
                }
                blockedRanges.push_back(r);
            }
        }

        continue;

bad:
        errMsg = "Config parse error at line " + std::to_string(lineNo) +
                 " for key '" + key + "' with value '" + val + "'";
        return false;
    }

    if (minServers < 1) minServers = 1;
    if (initialServers < minServers) initialServers = minServers;

    if (totalCycles < 1) totalCycles = 1;
    if (initialQueuePerServer < 0) initialQueuePerServer = 0;

    if (lowQueuePerServer < 1) lowQueuePerServer = 1;
    if (highQueuePerServer < lowQueuePerServer) highQueuePerServer = lowQueuePerServer + 1;
    if (scaleCooldownCycles < 0) scaleCooldownCycles = 0;

    if (arrivalChancePercent < 0) arrivalChancePercent = 0;
    if (arrivalChancePercent > 100) arrivalChancePercent = 100;

    if (streamingChancePercent < 0) streamingChancePercent = 0;
    if (streamingChancePercent > 100) streamingChancePercent = 100;

    if (minProcessTime < 1) minProcessTime = 1;
    if (maxProcessTime < minProcessTime) maxProcessTime = minProcessTime;

    return true;
}

bool Config::isBlocked(uint32_t ip) const {
    for (const auto& r : blockedRanges) {
        if (ip >= r.start && ip <= r.end) return true;
    }
    return false;
}