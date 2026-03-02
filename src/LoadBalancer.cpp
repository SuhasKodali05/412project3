#include "LoadBalancer.h"

#include <algorithm>
#include <sstream>

/**
 * @file LoadBalancer.cpp
 * @brief Implementation of LoadBalancer simulation loop and autoscaling.
 */

LoadBalancer::LoadBalancer(const Config& cfg, Logger& logger)
    : cfg_(cfg), log_(logger), rng_(cfg.rngSeed) {
    servers_.reserve(static_cast<size_t>(std::max(1, cfg_.initialServers)));
    for (int i = 0; i < cfg_.initialServers; ++i) addServer();
}

const Stats& LoadBalancer::stats() const { return stats_; }

std::string LoadBalancer::fmtCycle(int cycle) {
    return "[t=" + std::to_string(cycle) + "] ";
}

uint32_t LoadBalancer::randomIp() {
    uint32_t a = static_cast<uint32_t>(1 + (rng_() % 223));
    uint32_t b = static_cast<uint32_t>(rng_() % 256);
    uint32_t c = static_cast<uint32_t>(rng_() % 256);
    uint32_t d = static_cast<uint32_t>(rng_() % 256);
    return (a << 24) | (b << 16) | (c << 8) | d;
}

int LoadBalancer::randomProcessTime() {
    std::uniform_int_distribution<int> dist(cfg_.minProcessTime, cfg_.maxProcessTime);
    return dist(rng_);
}

char LoadBalancer::randomJobType() {
    int x = pct_(rng_);
    return (x < cfg_.streamingChancePercent) ? 'S' : 'P';
}

Request LoadBalancer::generateRequest() {
    Request r;
    r.ipIn = randomIp();
    r.ipOut = randomIp();
    r.time = randomProcessTime();
    r.jobType = randomJobType();
    return r;
}

void LoadBalancer::fillInitialQueue() {
    const long long target = 1LL * static_cast<long long>(servers_.size()) * cfg_.initialQueuePerServer;
    for (long long i = 0; i < target; ++i) {
        Request r = generateRequest();
        stats_.generated++;

        if (cfg_.isBlocked(r.ipIn)) {
            stats_.blocked++;
            continue;
        }

        q_.push(r);
        stats_.queued++;
    }

    stats_.peakQueue = std::max(stats_.peakQueue, static_cast<long long>(q_.size()));

    log_.info(Logger::cyan("Initial queue filled."));
    log_.info("Starting servers: " + std::to_string(servers_.size()));
    log_.info("Starting queue size: " + std::to_string(q_.size()));
    log_.info("Task time range: [" + std::to_string(cfg_.minProcessTime) + ", " +
              std::to_string(cfg_.maxProcessTime) + "] cycles");
}

void LoadBalancer::addServer() {
    servers_.push_back(std::make_unique<WebServer>(nextServerId_++));
    stats_.addedServers++;
}

void LoadBalancer::removeServer() {
    if (servers_.size() <= 1) return;

    auto it = std::find_if(servers_.rbegin(), servers_.rend(),
                           [](const std::unique_ptr<WebServer>& s) { return s->isIdle(); });

    if (it == servers_.rend()) return;

    auto baseIt = std::next(it).base();
    servers_.erase(baseIt);
    stats_.removedServers++;
}

void LoadBalancer::assignRequestsToIdleServers() {
    for (auto& s : servers_) {
        if (q_.empty()) break;
        if (s->isIdle()) {
            Request r = q_.front();
            q_.pop();
            s->assign(r);
        }
    }
}

void LoadBalancer::tickServers(int cycle) {
    for (auto& s : servers_) {
        auto finished = s->tick();
        if (finished.has_value()) {
            stats_.completed++;
            std::string msg = fmtCycle(cycle) + "Server#" + std::to_string(s->id())
                + " completed " + finished->toString();
            log_.raw(Logger::green(msg));
        }
    }
}

void LoadBalancer::maybeGenerateNewRequest(int cycle) {
    int x = pct_(rng_);
    if (x >= cfg_.arrivalChancePercent) return;

    Request r = generateRequest();
    stats_.generated++;

    if (cfg_.isBlocked(r.ipIn)) {
        stats_.blocked++;
        log_.raw(Logger::red(fmtCycle(cycle) + "BLOCKED " + r.toString()));
        return;
    }

    q_.push(r);
    stats_.queued++;
    stats_.peakQueue = std::max(stats_.peakQueue, static_cast<long long>(q_.size()));

    log_.raw(Logger::magenta(fmtCycle(cycle) + "NEW " + r.toString()
        + " (queue=" + std::to_string(q_.size()) + ")"));
}

void LoadBalancer::maybeScale(int cycle) {
    if (scaleCooldown_ > 0) {
        --scaleCooldown_;
        return;
    }

    int srv = static_cast<int>(servers_.size());
    long long qsz = static_cast<long long>(q_.size());

    long long low = 1LL * cfg_.lowQueuePerServer * srv;
    long long high = 1LL * cfg_.highQueuePerServer * srv;

    if (qsz > high) {
        addServer();
        scaleCooldown_ = cfg_.scaleCooldownCycles;
        log_.raw(Logger::yellow(fmtCycle(cycle) + "SCALE UP -> servers=" + std::to_string(servers_.size())
            + " (queue=" + std::to_string(qsz) + ", high=" + std::to_string(high) + ")"));
        return;
    }

    if (qsz < low && static_cast<int>(servers_.size()) > cfg_.minServers) {
        size_t before = servers_.size();
        removeServer();
        if (servers_.size() < before) {
            scaleCooldown_ = cfg_.scaleCooldownCycles;
            log_.raw(Logger::yellow(fmtCycle(cycle) + "SCALE DOWN -> servers=" + std::to_string(servers_.size())
                + " (queue=" + std::to_string(qsz) + ", low=" + std::to_string(low) + ")"));
        }
    }
}

void LoadBalancer::run() {
    const int startingServers = static_cast<int>(servers_.size());
    const long long startingQueue = static_cast<long long>(q_.size());

    log_.info("Starting simulation for " + std::to_string(cfg_.totalCycles) + " cycles");
    log_.info("Servers initially: " + std::to_string(servers_.size()));
    log_.info("Scaling rule: keep queue between "
              + std::to_string(cfg_.lowQueuePerServer) + "*servers and "
              + std::to_string(cfg_.highQueuePerServer) + "*servers");
    log_.info("Min servers: " + std::to_string(cfg_.minServers));

    for (int t = 1; t <= cfg_.totalCycles; ++t) {
        maybeGenerateNewRequest(t);
        assignRequestsToIdleServers();
        tickServers(t);
        maybeScale(t);

        if (t % 1000 == 0) {
            std::ostringstream oss;
            oss << fmtCycle(t)
                << "STATUS servers=" << servers_.size()
                << " queue=" << q_.size()
                << " completed=" << stats_.completed
                << " blocked=" << stats_.blocked
                << " peakQ=" << stats_.peakQueue;
            log_.raw(Logger::dim(oss.str()));
        }
    }

    log_.info("Simulation ended.");

    size_t idleServers = 0;
    for (const auto& s : servers_) {
        if (s->isIdle()) ++idleServers;
    }
    size_t busyServers = servers_.size() - idleServers;

    std::ostringstream summary;
    summary << "\n===== SUMMARY =====\n"
            << "Total cycles: " << cfg_.totalCycles << "\n"
            << "Starting servers: " << startingServers << "\n"
            << "Final servers: " << servers_.size() << "\n"
            << "Active servers: " << busyServers << "\n"
            << "Inactive servers: " << idleServers << "\n"
            << "Starting queue size: " << startingQueue << "\n"
            << "Ending queue size: " << q_.size() << "\n"
            << "Task time range: [" << cfg_.minProcessTime << ", " << cfg_.maxProcessTime << "] cycles\n"
            << "Scaling band: [" << cfg_.lowQueuePerServer << "*servers, " << cfg_.highQueuePerServer << "*servers]\n"
            << "Scale cooldown: " << cfg_.scaleCooldownCycles << " cycles\n"
            << "Arrival chance: " << cfg_.arrivalChancePercent << "% per cycle\n"
            << "Streaming chance: " << cfg_.streamingChancePercent << "%\n"
            << "Blocked ranges count: " << cfg_.blockedRanges.size() << "\n"
            << "Generated: " << stats_.generated << "\n"
            << "Blocked: " << stats_.blocked << "\n"
            << "Queued: " << stats_.queued << "\n"
            << "Completed: " << stats_.completed << "\n"
            << "Peak queue size: " << stats_.peakQueue << "\n"
            << "Servers added: " << stats_.addedServers << "\n"
            << "Servers removed: " << stats_.removedServers << "\n"
            << "===================\n";
    log_.raw(summary.str());
}