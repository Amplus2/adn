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
 * Parses the next `Token` out of the buffer `s` with the end pointer `end`.
 * Errors are stored in `err`.
 */
inline Token next(const char32_t *&s, const char32_t *end) {
    // eat whitespace
    while (isWhitespace(*s) && s < end) s++;

    // eat comments
    if (*s == ';') {
        while (*s != '\n' && s < end) s++;
        return next(s, end);
    }

    // handle end of file
    if (s >= end) return Token(EndOfFile, U"end of file");

    char32_t c;
    std::u32string tmpStr;

    // handle parentheses and brackets
    switch (c = *s++) {
        case '(':  return Token(ParenLeft, U"(");
        case ')':  return Token(ParenRight, U")");
        case '[':  return Token(BracketLeft, U"[");
        case ']':  return Token(BracketRight, U"]");
        case '{':  return Token(CurlyLeft, U"{");
        case '}':  return Token(CurlyRight, U"}");
        case '*':  return Token(Asterisk, U"*");
        case '#':  return Token(Hash, U"#");
        case '\'': return Token(SingleQuote, U"'");
        case '\\':
            if (s >= end) Token(Error, U"expected char after '\"', got EOF");
            return Token(Char, std::u32string(1, *s++));
        case '"':
            // handle strings
            if (s >= end) Token(Error, U"expected string after '\"', got EOF");
            while (s < end && (c = *s++) != '"') {
                tmpStr += c == '\\' ? *s++ : c;
            }
            if(s >= end && c != '"') return Token(Error, U"expected string after '\"', got EOF");
            return Token(String, tmpStr);
        case '0'...'9':
            // handle integers and front half of floats
            do { tmpStr += c; } while (s < end && isDigit((c = *s++)));
            if (c != '.' || s >= end) return Token(Int, tmpStr);
        case '.':
            // handle back half of floats
            tmpStr += '.';
            c = *s++;
            if(isDigit(c) && s <= end) {
                do { tmpStr += c; } while (isDigit(c = *s++) && s <= end);
                return Token(Float, tmpStr);
            } else return Token(Error, std::u32string() + U"expected digit after '.', got " + (s > end ? U"EOF" : std::u32string() + U"'" + c + U"'"));
        default:
            // handle identifiers
            do {
                tmpStr += c;
            } while (s < end && isIdentifierChar(c = *s++));
            return Token(Identifier, tmpStr);
    }
}

inline std::vector<Token> lex(const char32_t *s, uint_fast32_t length) {
    const char32_t *end = s + length;
    std::vector<Token> tokens;
    do {
        tokens.push_back(next(s, end));
    } while(tokens.back().type != EndOfFile);
    return tokens;
}
}
}
}
