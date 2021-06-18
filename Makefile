CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra
CXXFLAGS += -std=gnu++2a

all: format test

test: testexe
	./testexe

testexe: adn.hh test.cc
	$(CXX) $(CXXFLAGS) test.cc -o testexe

clean:
	rm -f testexe

format:
	clang-format -Werror -i --style=file adn.hh

.PHONY: clean test format all
