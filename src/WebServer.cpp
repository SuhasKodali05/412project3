#include "WebServer.h"

#include <stdexcept>

WebServer::WebServer(int id) : id_(id) {}

int WebServer::id() const { return id_; }

bool WebServer::isIdle() const { return !cur_.has_value(); }

int WebServer::remaining() const { return remaining_; }

std::optional<Request> WebServer::current() const { return cur_; }

void WebServer::assign(const Request& r) {
    if (!isIdle()) {
        throw std::runtime_error("Attempted to assign to busy server id=" + std::to_string(id_));
    }
    cur_ = r;
    remaining_ = r.time;
    if (remaining_ < 1) remaining_ = 1;
}

std::optional<Request> WebServer::tick() {
    if (isIdle()) return std::nullopt;

    --remaining_;
    if (remaining_ <= 0) {
        auto finished = cur_;
        cur_.reset();
        remaining_ = 0;
        return finished;
    }
    return std::nullopt;
}