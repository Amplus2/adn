/* The adn Reference Implementation
 * Copyright (C) 2021 Amplus 2.0
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#pragma once
#include <codecvt>
#include <locale>
#include <string>
#include <vector>

namespace {
namespace Adn {
namespace Util {
inline std::string U32ToUtf8(const std::u32string &s) {
    return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().to_bytes(s);
}
inline std::string StringMul(const std::string &s, unsigned n) {
    std::string o;
    while(n--) o += s;
    return o;
}
}
namespace Lexer {
enum Type {
    Error = 0,

    ParenLeft = 1,    // '('
    ParenRight = 2,   // ')'
    BracketLeft = 3,  // '['
    BracketRight = 4, // ']'
    CurlyLeft = 5,    // '{'
    CurlyRight = 6,   // '}'

    Hash = 7,        // '#'
    SingleQuote = 8, // '\''

    Id = 9,      // [.^[0-9]]+
    Int = 10,    // [0-9]+
    Float = 11,  // [0-9]*'.'[0-9]+
    Char = 12,   // '\\'.
    String = 13, // '"'.*?'"'

    Comment = 14, // Currently unused cause all comments are ignored
    EndOfFile = 0xff,
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
    inline Token(enum Type t, std::u32string v = U"", enum Error e = None)
            : type(t), err(e), value(v) {}
    inline std::string to_string() const {
        return std::string() + std::to_string(type) + " (" + std::to_string(err) +
               "): " + Util::U32ToUtf8(value);
    }
};

/**
 * Checks if `c` is whitespace in an **almost** Unicode-compliant fashion.
 */
constexpr inline bool isWhitespace(char32_t c) {
    return c <= ' ' || c == ',' || c == 0x85 || c == 0xA0 || c == 0x1680 ||
           (c >= 0x2000 && c <= 0x200C) || c == 0x2028 || c == 0x2029 || c == 0x202F ||
           c == 0x205F || c == 0x3000 || c == 0xFEFF;
}

constexpr inline bool isIdentifierChar(char32_t c) {
    return !isWhitespace(c) && c != '(' && c != ')' && c != '[' && c != ']' && c != '{' &&
           c != '}' && c != '#';
}

constexpr inline bool isDigit(char32_t c) { return c >= '0' && c <= '9'; }

/**
 * Parses the next `Token` out of the buffer `s` with the end pointer `end`.
 * Increments `s` by the length of the token.
 */
inline Token next(const char32_t *&s, const char32_t *end) {
    // TODO: a lot of fuzzing to find problems in here
    // eat whitespace
    while(isWhitespace(*s) && s < end) s++;

    // eat comments
    // TODO: implement comment pass-through
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
            return Token(ParenLeft);
        case ')':
            return Token(ParenRight);
        case '[':
            return Token(BracketLeft);
        case ']':
            return Token(BracketRight);
        case '{':
            return Token(CurlyLeft);
        case '}':
            return Token(CurlyRight);
        case '#':
            return Token(Hash);
        case '\'':
            return Token(SingleQuote);
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
        case '-':
            tmpStr += '-';
            c = *s++;
            [[fallthrough]];
        case '0' ... '9':
            // handle integers and front half of floats
            do {
                tmpStr += c;
            } while(s < end && isDigit((c = *s++)));
            if(s >= end) return Token(Int, tmpStr);
            if(c != '.') {
                s--;
                return Token(Int, tmpStr);
            }
            [[fallthrough]];
        case '.':
            // handle back half of floats
            tmpStr += '.';
            c = *s++;
            if(isDigit(c) && s <= end) {
                do {
                    tmpStr += c;
                } while(s < end && isDigit(c = *s++));
                if(s < end) s--;
                return Token(Float, tmpStr);
            } else
                return s > end ? Token(Error, U"", FloatEOF)
                               : Token(Error, std::u32string() + c, FloatNotNumber);
        default:
            // handle identifiers
            do {
                tmpStr += c;
            } while(s < end && isIdentifierChar(c = *s++));
            if(s < end) s--;
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
namespace Parser {
enum Type {
    Error = Lexer::Error,
    Id = Lexer::Id,
    Int = Lexer::Int,
    Float = Lexer::Float,
    Char = Lexer::Char,
    String = Lexer::String,
    List,
    Vector,
    Map,
    EndOfFile = Lexer::EndOfFile,
};
enum Error {
    None = 0,
    LexerError = 1,
    UnmatchedParens = 2,
    UnmatchedBrackets = 4,
    UnmatchedCurlies = 8,
    UnmatchedMapKey = 16,
};
class Element {
    public:
    enum Type type;
    enum Error err;
    uint_fast8_t quotes = 0, hashes = 0;
    std::u32string str;
    std::vector<Element> vec;
    // this is HORRIBLE, but we can't use std::map
    std::vector<std::pair<Element, Element>> map;
    int_fast64_t i;
    double d;
    char32_t c;
    inline Element(enum Type t, enum Error e = None) : type(t), err(e) {}
    inline std::string to_string() const {
        std::string s = std::string() + std::to_string(type) + " (" + std::to_string(err) +
                        "): " + Util::StringMul("#", hashes) + Util::StringMul("'", quotes);
        switch(type) {
            case Error:
                break;
            case Id:
                [[fallthrough]];
            case String:
                s += '"' + Util::U32ToUtf8(str) + '"';
                break;
            case Int:
                s += std::to_string(i);
                break;
            case Float:
                s += std::to_string(d);
                break;
            case Char:
                s += Util::U32ToUtf8(std::u32string(1, c));
                break;
            case List:
                [[fallthrough]];
            case Vector:
                for(Element e : vec) {
                    s += "(" + e.to_string() + ") ";
                }
                if(!vec.empty()) s.pop_back();
                break;
            case Map:
                for(auto p : map) {
                    s += "(" + p.first.to_string() + ") : (" + p.second.to_string() + ") ";
                }
                if(!map.empty()) s.pop_back();
                break;
            case EndOfFile:
                break;
        }
        return s;
    }
};
inline Element next(const Lexer::Token *&ts, const Lexer::Token *end) {
    Element e(EndOfFile);
    if(ts >= end) return e;
    Lexer::Token t = *ts++;
    // TODO: this CAN overflow the buffer if it ends in a comment, fix that pls
    while(t.type == Lexer::Comment) {
        t = *ts++;
    }
    switch(t.type) {
        case Lexer::Error:
            return Element(Error, LexerError);
        case Lexer::ParenLeft:
            e = Element(List);
            do {
                if((*ts).type == Lexer::ParenRight) {
                    ts++;
                    return e;
                }
                e.vec.push_back(next(ts, end));
            } while(e.vec.back().type != EndOfFile);
            return e;
        case Lexer::BracketLeft:
            e.type = Vector;
            do {
                if((*ts).type == Lexer::BracketRight) {
                    ts++;
                    return e;
                }
                e.vec.push_back(next(ts, end));
            } while(e.vec.back().type != EndOfFile);
            return e;
        case Lexer::CurlyLeft:
            e.type = Map;
            while(ts <= end) {
                if((*ts).type == Lexer::CurlyRight) {
                    ts++;
                    return e;
                }
                Element key = next(ts, end);
                // for eof the error and handling are not really correct
                // TODO: rethink this a bit
                if(key.type == EndOfFile || (*ts).type == Lexer::CurlyRight) {
                    ts++;
                    e.err = UnmatchedMapKey;
                    return e;
                }
                Element value = next(ts, end);
                // TODO: this is also obviously wrong
                if(value.type == EndOfFile || ts > end) {
                    e.err = UnmatchedMapKey;
                    return e;
                }
                e.map.push_back({key, value});
            }
            return e;
        case Lexer::ParenRight:
            return Element(Error, UnmatchedParens);
        case Lexer::BracketRight:
            return Element(Error, UnmatchedBrackets);
        case Lexer::CurlyRight:
            return Element(Error, UnmatchedCurlies);
        case Lexer::Hash:
            e = next(ts, end);
            e.hashes++;
            return e;
        case Lexer::SingleQuote:
            e = next(ts, end);
            e.quotes++;
            return e;
        case Lexer::Id:
            e.type = Id;
            e.str = t.value;
            return e;
        case Lexer::Int:
            e.type = Int;
            e.i = std::stoll(std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().to_bytes(
                    t.value));
            return e;
        case Lexer::Float:
            e.type = Float;
            e.d = std::stod(std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().to_bytes(
                    t.value));
            return e;
        case Lexer::Char:
            e.type = Char;
            e.c = t.value[0];
            return e;
        case Lexer::String:
            e.type = String;
            e.str = t.value;
            return e;
        case Lexer::EndOfFile:
            return Element(EndOfFile);
            // this can not happen
        case Lexer::Comment:
            break;
    }
    return Element(Error, None);
}

/**
 * A simplified API: calls `next` until it gets an EOF and returns all elements
 */
inline std::vector<Element> parse(const std::vector<Lexer::Token> tokens) {
    const Lexer::Token *s = &tokens[0];
    const Lexer::Token *end = s + tokens.size();
    std::vector<Element> elements;
    do {
        elements.push_back(next(s, end));
    } while(elements.back().type != EndOfFile);
    return elements;
}
}
}
}
