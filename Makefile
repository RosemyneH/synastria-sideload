CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -fPIC -Wall -Wextra -Wno-unused-parameter
BUILD := build

.PHONY: all clean

all: $(BUILD)/libsynastria.so $(BUILD)/probe

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/libsynastria.so: src/sideload.cpp src/sym.cpp src/cache.cpp src/features.cpp $(wildcard src/natives/*.inc) | $(BUILD)
	$(CXX) $(CXXFLAGS) -rdynamic -shared -o $@ src/sideload.cpp src/sym.cpp src/cache.cpp src/features.cpp -llz4

$(BUILD)/probe: src/probe.cpp src/cache.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ src/probe.cpp src/cache.cpp

clean:
	rm -rf $(BUILD)
