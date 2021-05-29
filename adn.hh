#pragma once
#include <string>
#include <utf8.hh>

namespace {
namespace Adn {
namespace Token {
constexpr inline uint32_t utf8getc(const uint8_t *&s, int &e) {
    uint32_t c = 0;
    s = utf8_decode(s, c, e);
    return c;
}

enum Type {
    Error,

    ParenLeft,    // '('
    ParenRight,   // ')'
    BracketLeft,  // '['
    BracketRight, // ']'

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
    std::string value;
    Token() : type(Error), value(std::string()) {}
    Token(Type t, std::string v) : type(t), value(v) {}
};

//enum RawType {
//    TYPE_ERR,
//
//    TYPE_VOID,
//    TYPE_I8,
//    TYPE_I16,
//    TYPE_I32,
//    TYPE_I64,
//    TYPE_FLOAT,
//    TYPE_DOUBLE,
//};

/**
 * Parses the next `Token` out of the buffer `s` with the length `length`.
 * Errors are stored in `e`.
 */
constexpr inline Token next(const uint8_t *&s, uint_fast32_t length, int &e) {
    const uint8_t *end = s + length;

    uint32_t c = utf8getc(s, e);
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
    if (s >= end) return Token(EndOfFile, "end of file");

    // handle parentheses and brackets
    switch (c) {
        case '(':  return Token(LeftParen, "(");
        case ')':  return Token(RightParen, ")");
        case '[':  return Token(LeftBracket, "[");
        case ']':  return Token(RightBracket, "]");
        case '*':  return Token(Asterisk, "*");
        case '#':  return Token(Hash, "#");
        case '\'': return Token(SingleQuote, "'");
    }

    // helper variable for temporary string storage
    std::string tmpStr;

    // handle integers and front part of floats
    if (isDigit(c)) {
        while (isDigit((c = utf8getc(s, e))) && !e) tmpStr += c;
        if(e) return Token();

        if (c != '.') return Token(Int, tmpStr);
    }

    // handle floats
    // TODO: all code below still needs a lot of work
    if(c == '.') {
        tmpStr += '.';
        c = utf8getc(s, e);
        if(e) return Token();
        if(isDigit(c)) {
            if (eofReached()) lexerEOFError();

            // append all digits after the '.' to tmpStr
            while (isDigit(getc(idx))) tmpStr += getc(idx++);

            return Token(Float, tmpStr);
        } else
        error(ERROR_LEXER, "expected digit after '.', got '"
            + std::string(1, c) + "'", pos());
    }

    if (getc(idx) == '"') {
        // eat up '"'
        idx += 1;

        if (eofReached()) lexerEOFError();

        bool lastBS = false;
        while ((c = getc(idx)) != '"' || lastBS) {
            tmpStr += c;
            lastBS = c == '\\';
            idx += 1;

            if (eofReached()) lexerEOFError();
        }

        // eat up '"'
        idx += 1;

        return Token(String, tmpStr);
    } else if ((c = getc(idx)) == '\\') {
        // eat up '\\'
        idx += 1;

        if (eofReached()) lexerEOFError();

        c = getc(idx);

        // eat up char
        idx += 1;

        return Token(TT_CHAR, std::string(1, c));
    }

    // handle identifiers
    while (!isWhitespace((c = getc(idx))) && !isSpecialChar(c)
            && idx < text.size())
        tmpStr += getc(idx++);

    return Token(TT_ID, tmpStr);
}
}
}
}
