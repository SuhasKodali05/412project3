#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "Config.h"
#include "Logger.h"
#include "Request.h"
#include "WebServer.h"

#include <memory>
#include <queue>
#include <random>
#include <vector>

/**
 * @file LoadBalancer.h
 * @brief Defines the LoadBalancer class that manages servers and request queue.
 */

/**
 * @struct Stats
 * @brief Simple statistics collected during the simulation.
 */
struct Stats {
    long long generated{0};
    long long blocked{0};
    long long queued{0};
    long long completed{0};
    long long addedServers{0};
    long long removedServers{0};
    long long peakQueue{0};
};

/**
 * @class LoadBalancer
 * @brief Manages a pool of WebServers and a queue of Requests.
 *
 * Responsibilities:
 * - Holds request queue
 * - Assigns requests to idle servers
 * - Advances time (clock cycles)
 * - Generates new requests at random times
 * - Dynamically scales servers to keep queue within desired range
 * - Blocks specified IP ranges (simple firewall)
 */
class LoadBalancer {
public:
    /**
     * @brief Construct with configuration and a logger.
     * @param cfg Configuration settings
     * @param logger Logger instance
     */
    LoadBalancer(const Config& cfg, Logger& logger);

    /** @brief Fill initial queue of size initialServers * initialQueuePerServer. */
    void fillInitialQueue();

    /**
     * @brief Run the simulation for cfg.totalCycles cycles.
     */
    void run();

    /** @brief Get statistics after running. */
    const Stats& stats() const;

private:
    Config cfg_;
    Logger& log_;
    Stats stats_;

    std::queue<Request> q_;
    std::vector<std::unique_ptr<WebServer>> servers_;

    std::mt19937 rng_;
    std::uniform_int_distribution<int> pct_{0, 99};

    int nextServerId_{1};
    int scaleCooldown_{0};

    Request generateRequest();
    uint32_t randomIp();
    int randomProcessTime();
    char randomJobType();

    void addServer();
    void removeServer();

    void assignRequestsToIdleServers();
    void tickServers(int cycle);
    void maybeGenerateNewRequest(int cycle);
    void maybeScale(int cycle);

    static std::string fmtCycle(int cycle);
};

#endif