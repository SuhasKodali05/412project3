# 412project3
# Project 3 — Load Balancer (LB)

This is a C++17 simulation of a load-balancing system that:
- Generates random requests (IP in/out, time in cycles, job type P/S)
- Uses a real queue (`std::queue<Request>`)
- Processes work on a configurable number of `WebServer` instances
- Dynamically adds/removes servers to keep the queue size between:
  - **low_queue_per_server * servers** and **high_queue_per_server * servers**
- Blocks configured IP ranges (CIDR or start-end ranges)
- Logs colored events to console and writes a full log + summary to `lb.log`

## Files
- `include/Request.h`, `src/Request.cpp`
- `include/WebServer.h`, `src/WebServer.cpp`
- `include/LoadBalancer.h`, `src/LoadBalancer.cpp`
- `include/Config.h`, `src/Config.cpp`
- `include/Logger.h`, `src/Logger.cpp`
- `src/main.cpp`
- `Makefile`
- `config.txt`

## Build / Run
From the project root:

```bash
make clean
make
./lb