CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra
CXXFLAGS += -std=c++20

test: testexe
	./testexe

testexe: adn.hh test.cc
	$(CXX) $(CXXFLAGS) test.cc -o testexe

clean:
	rm -f testexe

.PHONY: clean test
