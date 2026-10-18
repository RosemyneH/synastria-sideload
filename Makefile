CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -fPIC -Wall -Wextra -Wno-unused-parameter
BUILD := build

.PHONY: all clean

all: $(BUILD)/libsynastria.so $(BUILD)/probe

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/libsynastria.so: src/sideload.cpp src/sym.cpp src/cache.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -shared -o $@ src/sideload.cpp src/sym.cpp src/cache.cpp

$(BUILD)/probe: src/probe.cpp src/cache.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ src/probe.cpp src/cache.cpp

clean:
	rm -rf $(BUILD)
