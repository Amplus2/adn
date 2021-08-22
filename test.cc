#include <iostream>
#include <string>

#include "adn.hh"
using namespace Adn;

#define Assert(expr)                                             \
    if(!(expr)) {                                                \
        err++;                                                   \
        std::cout << "Failed assertion: " << #expr << std::endl; \
    }

#define LexAssert(ty, val)                                    \
    {                                                         \
        Lexer::Token t = Lexer::Next(s, send);                \
        Assert(t.type == Lexer::ty);                          \
        Assert(std::u32string(val) == U"" || t.value == val); \
    }

#define ParseAssert(ty, expr)                        \
    {                                                \
        Parser::Element e = Parser::Next(ts, tsend); \
        Assert(e.type == Parser::ty);                \
        Assert(expr);                                \
    }

int main() {
    const std::u32string test1 = U"( [\n{}\t]) \"str\" \\ä \\🍆 3.145 .1 42";
    const std::u32string test2 = U"\\@ (x \\y .5) {… ∧} -42";
    const char32_t *s = test1.c_str();
    const char32_t *send = s + test1.size();
    const auto test2t = Lexer::Lex(test2);
    const Lexer::Token *ts = &test2t[0];
    const Lexer::Token *tsend = ts + test2t.size();
    int err = 0;
    LexAssert(ParenLeft, U"");
    LexAssert(BracketLeft, U"");
    LexAssert(CurlyLeft, U"");
    LexAssert(CurlyRight, U"");
    LexAssert(BracketRight, U"");
    LexAssert(ParenRight, U"");
    LexAssert(String, U"str");
    LexAssert(Char, U"ä");
    LexAssert(Char, U"🍆");
    LexAssert(Float, U"3.145");
    LexAssert(Float, U".1");
    LexAssert(Int, U"42");
    ParseAssert(Char, e.c == U'@');
    ParseAssert(List, e.vec[0].str == U"x");
    ParseAssert(Map,
                e.map[0].first.type == Parser::Id && e.map[0].first.str == U"…" &&
                        e.map[0].second.type == Parser::Id && e.map[0].second.str == U"∧");
    if(!err) std::cout << "All tests passed." << std::endl;
    return err;
}
