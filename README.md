# adn
The best data serialization format is a modified version of edn.

We have a [very brief spec](SPEC.md) and an almost complete implementation.

## Usage
To use the adn Reference Implementation, you just have to put it in your include
path and include it, then you can make use of the Lexer and Parser:

```cpp
#include <adn.hh>

int main() {
    auto tokens = Adn::Lexer::Lex(U"(1 2 3)");
    auto elements = Adn::Parser::Parse(tokens);
    for(auto list : elements) {
        assert(list.type == Adn::Parser::List);
        for(auto element : list.vec) {
            assert(element.type == Adn::Parser::Int);
            std::cout << element.i << std::endl;
        }
    }
}
```

```sh
c++ -I/path/to/adn example.cc -o example
./example
```

```
1
2
3
```
