CXX ?= g++-13
CXXFLAGS ?= -O2 -pipe -std=gnu++17 -s
LDFLAGS ?=

all: code

code: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f code

.PHONY: all clean
