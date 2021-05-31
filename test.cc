#include "adn.hh"
#include <iostream>
#include <string>

#define err_assert(expr) if(!(expr)) { if(!err) err = 1; std::cout << "Failed assertion: " << #expr << std::endl; }

int main() {
        const std::u32string test1 = U"( [\n{\t}]) \"str\" \\ä \\🍆 3.145 .1 42";
        auto tokens = Adn::Lexer::lex(test1.c_str(), test1.size());
        int err = 0;
        err_assert(tokens[0].type == Adn::Lexer::ParenLeft);
        err_assert(tokens[1].type == Adn::Lexer::BracketLeft);
        err_assert(tokens[2].type == Adn::Lexer::CurlyLeft);
        err_assert(tokens[3].type == Adn::Lexer::CurlyRight);
        err_assert(tokens[4].type == Adn::Lexer::BracketRight);
        err_assert(tokens[5].type == Adn::Lexer::ParenRight);
        err_assert(tokens[6].type == Adn::Lexer::String && tokens[6].value == U"str");
        err_assert(tokens[7].type == Adn::Lexer::Char && tokens[7].value == U"ä");
        err_assert(tokens[8].type == Adn::Lexer::Char && tokens[8].value == U"🍆");
        err_assert(tokens[9].type == Adn::Lexer::Float && tokens[9].value == U"3.145");
        err_assert(tokens[10].type == Adn::Lexer::Float && tokens[10].value == U".1");
        err_assert(tokens[11].type == Adn::Lexer::Int && tokens[11].value == U"42");
        if(!err) std::cout << "All tests passed." << std::endl;
        return err;
}
