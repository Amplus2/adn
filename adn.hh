#pragma once
#include <string>
#include <vector>
#include <utf8.hh>

namespace {
namespace Adn {
namespace Lexer {
enum Type {
    Error,

    ParenLeft,    // '('
    ParenRight,   // ')'
    BracketLeft,  // '['
    BracketRight, // ']'
    CurlyLeft,    // '{'
    CurlyRight,   // '}'

    Asterisk,     // '*'
    Hash,         // '#'
    SingleQuote,  // '\''

    Identifier,   // [.^[0-9]]+
    Int,          // [0-9]+
    Float,        // [0-9]*'.'[0-9]+
    Char,         // '\\'.
    String,       // '"'.*?'"'

    EndOfFile,    // end of file

    // TODO: comments
};

class Token {
public:
    Type type;
    std::u32string value;
    inline Token(Type t=Error, std::u32string v=std::u32string()) : type(t), value(v) {}
};

constexpr inline uint32_t utf8getc(const uint8_t *&s, int &e) {
    uint32_t c = 0;
    s = utf8_decode(s, c, e);
    return c;
}

constexpr inline bool isWhitespace(char32_t c) {
    for(uint32_t trans : {0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x0020, 0x0085, 0x00A0, 0x1680,
        0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x200B, 0x200C,
        0x2028, 0x2029, 0x202F, 0x205F, 0x3000, 0xFEFF}) if(c == trans) return true;
    if(c == U',') return true;
    return false;
}

    constexpr inline bool isIdentifierChar(char32_t c){
        return !isWhitespace(c) && c != '(' && c != ')' && c != '[' && c != ']' && c != '{' && c != '}' && c != '#' && c != '*';
    }

constexpr inline bool isDigit(char32_t c) {
    return c >= '0' && c <= '9';
}

/**
 * Parses the next `Token` out of the buffer `s` with the length `length`.
 * Errors are stored in `e`.
 */
inline Token next(const uint8_t *&s, const uint8_t *end, int &e) {
    char32_t c = utf8getc(s, e);
    if(e) return Token();

nextBegin:

    // eat whitespace
    while (isWhitespace(c) && s < end && !e) c = utf8getc(s, e);
    if(e) return Token();

    // comments
    if (c == ';') {
        // eat up until end of line
        while (c != '\n' && s < end && !e) c = utf8getc(s, e);
        if(e) return Token();

        goto nextBegin;
    }

    // handle end of file
    if (s >= end) return Token(EndOfFile, U"end of file");

    // handle parentheses and brackets
    switch (c) {
        case '(':  return Token(ParenLeft, U"(");
        case ')':  return Token(ParenRight, U")");
        case '[':  return Token(BracketLeft, U"[");
        case ']':  return Token(BracketRight, U"]");
        case '{':  return Token(CurlyLeft, U"{");
        case '}':  return Token(CurlyRight, U"}");
        case '*':  return Token(Asterisk, U"*");
        case '#':  return Token(Hash, U"#");
        case '\'': return Token(SingleQuote, U"'");
    }

    // helper variable for temporary string storage
    std::u32string tmpStr;

    // handle integers and front part of floats
    if (isDigit(c)) {
        tmpStr += c;
        while (isDigit((c = utf8getc(s, e))) && s < end && !e) tmpStr += c;
        if(e) return Token();

        if (c != '.' || s >= end) return Token(Int, tmpStr);
    }

    // handle floats
    if(c == '.') {
        tmpStr += '.';
        c = utf8getc(s, e);
        if(e) return Token();
        if(isDigit(c) && s <= end) {
            tmpStr += c;
            // append all digits after the '.' to tmpStr
            while (isDigit((c = utf8getc(s, e))) && s < end && !e) tmpStr += c;
            if(e) return Token();

            return Token(Float, tmpStr);
        } else Token(Error, std::u32string() + U"expected digit after '.', got U" + (s > end ? U"EOF" : std::u32string() + U"'" + c + U"'"));
    }

    // TODO: all code below still needs a lot of work
    if (c == '"') {
        if (s >= end) Token(Error, U"expected string after '\"', got EOF");

        while ((c = utf8getc(s, e)) != '"' && s <= end) {
            tmpStr += c == '\\' ? utf8getc(s, e) : c;
        }
        // TODO: this is kinda broken, but i dont know how to properly do this rn (it works most of the time)
        if(s > end && c != '"') return Token(Error, U"expected string after '\"', got EOF");

        return Token(String, tmpStr);
    }

    if (c == '\\') {
        if (s >= end) Token(Error, U"expected char after '\"', got EOF");

        c = utf8getc(s, e);
        if(e) return Token();

        return Token(Char, std::u32string(1, c));
    }

    tmpStr += c;
    // handle identifiers
    while (isIdentifierChar((c = utf8getc(s, e))) && !e && s <= end) tmpStr += c;
    if(e) return Token();

    return Token(Identifier, tmpStr);
}

    inline std::vector<Token> lex(const uint8_t *s, uint_fast32_t length, int &err) {
    const uint8_t *end = s + length;
    std::vector<Token> tokens;
    do {
        tokens.push_back(next(s, end, err));
    } while(!err && tokens.back().type != EndOfFile);
        return tokens;
    }
}
}
}
