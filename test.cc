#include <iostream>
#include <string>

#include "adn.hh"

#define err_assert(expr)                                         \
    if(!(expr)) {                                                \
        err++;                                                   \
        std::cout << "Failed assertion: " << #expr << std::endl; \
    }

#define lex_assert(expr)                                \
    {                                                   \
        Adn::Lexer::Token t = Adn::Lexer::Next(s, end); \
        err_assert(expr);                               \
    }

int main() {
    const std::u32string test1 = U"( [\n{}\t]) \"str\" \\ä \\🍆 3.145 .1 42";
    const std::u32string test2 = U"\\@ (x \\y .5) {… ∧} -42";
    const char32_t *s = test1.c_str();
    const char32_t *end = s + test1.size();
    int err = 0;
    lex_assert(t.type == Adn::Lexer::ParenLeft);
    lex_assert(t.type == Adn::Lexer::BracketLeft);
    lex_assert(t.type == Adn::Lexer::CurlyLeft);
    lex_assert(t.type == Adn::Lexer::CurlyRight);
    lex_assert(t.type == Adn::Lexer::BracketRight);
    lex_assert(t.type == Adn::Lexer::ParenRight);
    lex_assert(t.type == Adn::Lexer::String && t.value == U"str");
    lex_assert(t.type == Adn::Lexer::Char && t.value == U"ä");
    lex_assert(t.type == Adn::Lexer::Char && t.value == U"🍆");
    lex_assert(t.type == Adn::Lexer::Float && t.value == U"3.145");
    lex_assert(t.type == Adn::Lexer::Float && t.value == U".1");
    lex_assert(t.type == Adn::Lexer::Int && t.value == U"42");
    const std::vector<Adn::Parser::Element> elements = Adn::Parser::Parse(Adn::Lexer::Lex(test2));
    err_assert(elements[0].type == Adn::Parser::Char && elements[0].c == U'@');
    err_assert(elements[1].type == Adn::Parser::List && elements[1].vec[0].str == U"x");
    err_assert(elements[2].type == Adn::Parser::Map);
    err_assert(elements[2].map[0].first.type == Adn::Parser::Id);
    err_assert(elements[2].map[0].first.str == U"…");
    err_assert(elements[2].map[0].second.type == Adn::Parser::Id);
    err_assert(elements[2].map[0].second.str == U"∧");
    if(!err) std::cout << "All tests passed." << std::endl;
    return err;
}
