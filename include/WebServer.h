#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Request.h"

#include <optional>

/**
 * @file WebServer.h
 * @brief Defines the WebServer class for processing requests.
 */

/**
 * @class WebServer
 * @brief Represents a single server that processes at most one request at a time.
 */
class WebServer {
public:
    /**
     * @brief Construct server with an ID.
     * @param id Server identifier
     */
    explicit WebServer(int id);

    /** @brief Get server ID. */
    int id() const;

    /** @brief True if server is idle (no current request). */
    bool isIdle() const;

    /**
     * @brief Assign a request to this server (must be idle).
     * @param r Request to process
     */
    void assign(const Request& r);

    /**
     * @brief Advance one clock cycle.
     * @return An optional finished request if it completed this tick.
     */
    std::optional<Request> tick();

    /** @brief Remaining cycles on current request (0 if idle). */
    int remaining() const;

    /** @brief Get current request if any. */
    std::optional<Request> current() const;

private:
    int id_{0};
    std::optional<Request> cur_;
    int remaining_{0};
};

#endif