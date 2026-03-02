#ifndef REQUEST_H
#define REQUEST_H

#include <cstdint>
#include <string>

/**
 * @file Request.h
 * @brief Defines the Request struct used throughout the simulation.
 */

/**
 * @struct Request
 * @brief Represents a simulated web request.
 *
 * A Request contains:
 * - IP in:  requester IP address (uint32)
 * - IP out: destination/result IP address (uint32)
 * - time:   processing time in clock cycles (integer)
 * - jobType: 'P' = Processing, 'S' = Streaming
 */
struct Request {
    uint32_t ipIn{0};
    uint32_t ipOut{0};
    int time{0};
    char jobType{'P'};

    /** @brief Returns human-readable string form. */
    std::string toString() const;
};

/**
 * @brief Convert dotted-quad IPv4 string to uint32.
 * @param ip Dotted quad like "192.168.0.1"
 * @return IPv4 as uint32 (host order)
 * @throws std::runtime_error on parse errors
 */
uint32_t ipToUint(const std::string& ip);

/**
 * @brief Convert uint32 IPv4 to dotted-quad string.
 * @param ip IPv4 as uint32 (host order)
 * @return Dotted quad string
 */
std::string uintToIp(uint32_t ip);

#endif