#include "Request.h"

#include <sstream>
#include <stdexcept>
#include <vector>

static std::vector<std::string> split(const std::string& s, char delim) {
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

uint32_t ipToUint(const std::string& ip) {
    auto parts = split(ip, '.');
    if (parts.size() != 4) throw std::runtime_error("Invalid IP: " + ip);

    uint32_t out = 0;
    for (size_t i = 0; i < 4; ++i) {
        if (parts[i].empty()) throw std::runtime_error("Invalid IP: " + ip);
        int v = std::stoi(parts[i]);
        if (v < 0 || v > 255) throw std::runtime_error("Invalid IP: " + ip);
        out = (out << 8) | static_cast<uint32_t>(v);
    }
    return out;
}

std::string uintToIp(uint32_t ip) {
    std::ostringstream oss;
    oss << ((ip >> 24) & 0xFF) << '.'
        << ((ip >> 16) & 0xFF) << '.'
        << ((ip >> 8) & 0xFF) << '.'
        << (ip & 0xFF);
    return oss.str();
}

std::string Request::toString() const {
    std::ostringstream oss;
    oss << "Request{in=" << uintToIp(ipIn)
        << ", out=" << uintToIp(ipOut)
        << ", time=" << time
        << ", type=" << jobType
        << "}";
    return oss.str();
}