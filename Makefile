CXX ?= c++
CXXFLAGS ?= -O2
CXXFLAGS += -std=c++20 -I. -Iutf8

test: testexe
	./testexe

testexe: adn.hh test/main.cc
	$(CXX) $(CXXFLAGS) test/main.cc -o testexe

clean:
	rm -f testexe

.PHONY: clean test
