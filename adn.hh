#pragma once
#include <string>
#include <vector>

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

    Asterisk,    // '*'
    Hash,        // '#'
    SingleQuote, // '\''

    Id,     // [.^[0-9]]+
    Int,    // [0-9]+
    Float,  // [0-9]*'.'[0-9]+
    Char,   // '\\'.
    String, // '"'.*?'"'

    EndOfFile, // end of file

    // TODO: comments (some linters may want to use comments for some functionality)
};

enum Error {
    None = 0,
    CharEOF,
    StringEOF,
    FloatEOF,
    FloatNotNumber,
};

class Token {
    public:
    enum Type type;
    enum Error err;
    std::u32string value;
    inline Token(enum Type t, std::u32string v, enum Error e = None) : type(t), value(v), err(e) {}
};

constexpr inline bool isWhitespace(char32_t c) {
    return c <= ' ' || c == ',' || c == 0x85 || c == 0xA0 || c == 0x1680 ||
           (c >= 0x2000 && c <= 0x200C) || c == 0x2028 || c == 0x2029 || c == 0x202F ||
           c == 0x205F || c == 0x3000 || c == 0xFEFF;
}

constexpr inline bool isIdentifierChar(char32_t c) {
    return !isWhitespace(c) && c != '(' && c != ')' && c != '[' && c != ']' && c != '{' &&
           c != '}' && c != '#' && c != '*';
}

constexpr inline bool isDigit(char32_t c) { return c >= '0' && c <= '9'; }

/**
 * Parses the next `Token` out of the buffer `s` with the end pointer `end`.
 * Increments `s` by the length of the token.
 */
inline Token next(const char32_t *&s, const char32_t *end) {
    // eat whitespace
    while(isWhitespace(*s) && s < end) s++;

    // eat comments
    if(*s == ';') {
        while(*s != '\n' && s < end) s++;
        return next(s, end);
    }

    // handle end of file
    if(s >= end) return Token(EndOfFile, U"");

    char32_t c;
    std::u32string tmpStr;

    switch(c = *s++) {
            // handle parentheses and brackets
        case '(':
            return Token(ParenLeft, U"");
        case ')':
            return Token(ParenRight, U"");
        case '[':
            return Token(BracketLeft, U"");
        case ']':
            return Token(BracketRight, U"");
        case '{':
            return Token(CurlyLeft, U"");
        case '}':
            return Token(CurlyRight, U"");
        case '*':
            return Token(Asterisk, U"");
        case '#':
            return Token(Hash, U"");
        case '\'':
            return Token(SingleQuote, U"");
        case '\\':
            if(s >= end) Token(Error, U"", CharEOF);
            return Token(Char, std::u32string(1, *s++));
        case '"':
            // handle strings
            if(s >= end) Token(Error, U"", StringEOF);
            while(s < end && (c = *s++) != '"') {
                tmpStr += c == '\\' ? *s++ : c;
            }
            if(s >= end && c != '"') return Token(Error, U"", StringEOF);
            return Token(String, tmpStr);
        case '0' ... '9':
            // handle integers and front half of floats
            do {
                tmpStr += c;
            } while(s < end && isDigit((c = *s++)));
            if(c != '.' || s >= end) return Token(Int, tmpStr);
            [[fallthrough]];
        case '.':
            // handle back half of floats
            tmpStr += '.';
            c = *s++;
            if(isDigit(c) && s <= end) {
                do {
                    tmpStr += c;
                } while(isDigit(c = *s++) && s <= end);
                return Token(Float, tmpStr);
            } else
                return s > end ? Token(Error, U"", FloatEOF)
                               : Token(Error, std::u32string() + c, FloatNotNumber);
        default:
            // handle identifiers
            do {
                tmpStr += c;
            } while(s < end && isIdentifierChar(c = *s++));
            return Token(Id, tmpStr);
    }
}

/**
 * A simplified API: calls `next` until it gets an EOF and returns all tokens
 */
inline std::vector<Token> lex(const std::u32string str) {
    const char32_t *s = str.c_str();
    const char32_t *end = s + str.size();
    std::vector<Token> tokens;
    do {
        tokens.push_back(next(s, end));
    } while(tokens.back().type != EndOfFile);
    return tokens;
}
}
}
}
