CXX = c++
CXXFLAGS = -std=gnu++2a -Og -g -Wall -Wextra -pedantic -fsanitize=address -fsanitize=undefined

all: format test

test: testexe
	./testexe

testexe: adn.hh test.cc
	$(CXX) $(CXXFLAGS) test.cc -o testexe

clean:
	rm -f testexe

format:
	clang-format -Werror -i --style=file adn.hh test.cc

.PHONY: clean test format all
