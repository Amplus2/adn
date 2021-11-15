CXX ?= c++
CXXFLAGS := -std=c++1z -Og -g -Wall -Wextra -pedantic -fsanitize=address -fsanitize=undefined
DB ?= lldb

all: format test

test: testexe
	./testexe

testexe: adn.hh test.cc
	$(CXX) $(CXXFLAGS) test.cc -o testexe

clean:
	rm -f testexe

format:
	clang-format -Werror -i --style=file adn.hh test.cc

debug: testexe
	$(DB) testexe

.PHONY: clean test format all
