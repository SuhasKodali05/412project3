CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Iinclude
LDFLAGS :=

SRC := src/main.cpp src/Request.cpp src/WebServer.cpp src/LoadBalancer.cpp src/Config.cpp src/Logger.cpp
OBJ := $(SRC:.cpp=.o)
BIN := lb

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN) *.log

.PHONY: all clean