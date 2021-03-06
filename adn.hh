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
#include <numeric>
#include <string>
#include <vector>

namespace {
namespace Adn {
namespace Version {
inline static const int Major = 0;
inline static const int Minor = 0;
inline static const std::string Pretty = std::to_string(Major) + "." + std::to_string(Minor);
}
namespace Util {
inline static auto Utf32 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>();
inline std::string StringMul(const std::string &s, unsigned n) {
    std::string o;
    while(n--) o += s;
    return o;
}
constexpr inline bool IsWhitespace(char32_t c) {
    return c <= ' ' || c == ',' || c == 0x85 || c == 0xA0 || c == 0x1680 ||
           (c >= 0x2000 && c <= 0x200C) || c == 0x2028 || c == 0x2029 || c == 0x202F ||
           c == 0x205F || c == 0x3000 || c == 0xFEFF;
}
constexpr inline bool IsTerminator(char32_t c) {
    return IsWhitespace(c) || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' ||
           c == '}' || c == '#' || c == '\'' || c == '`';
}
constexpr inline char32_t Unescape(char32_t c) {
    return c == 'n' ? '\n' : c == 'r' ? '\r' : c == 't' ? '\t' : c;
}
inline void ReadToSeparator(std::u32string &tmpStr,
                            char32_t &c,
                            const char32_t *&s,
                            const char32_t *&end) {
    do {
        tmpStr += c;
    } while(s <= end && !IsTerminator(c = *s++));
    if(s <= end) s--;
}
}
using namespace Util;
namespace Lexer {
enum Type {
    Error = 0,

    ParenLeft = 1,
    ParenRight = 2,
    BracketLeft = 3,
    BracketRight = 4,
    CurlyLeft = 5,
    CurlyRight = 6,

    Hash = 7,
    SingleQuote = 8,
    Backtick = 9,

    Id = 10,
    Int = 11,    // [0-9]+
    Float = 12,  // [0-9]*'.'[0-9]+
    Char = 13,   // '\\'.
    String = 14, // '"'.*?'"'

    Comment = 15,
    EndOfFile = 0xff,
};

enum Error {
    None = 0,
    CharEOF = 1,
    StringEOF = 2,
    FloatEOF = 4,
};

class Token {
    public:
    enum Type type;
    enum Error err;
    std::u32string value;
    inline Token(enum Type t, const std::u32string &v = U"", enum Error e = None)
            : type(t), err(e), value(v) {}
    inline std::string to_string() const {
        return std::string() + std::to_string(type) + " (" + std::to_string(err) +
               "): " + Utf32.to_bytes(value);
    }
};

/**
 * Parses the next `Token` out of the buffer `s` with the end pointer `end`.
 * Increments `s` by the length of the token.
 */
inline Token Next(const char32_t *&s, const char32_t *end) {
    // TODO: a lot of fuzzing to find problems in here

    while(s < end && IsWhitespace(*s)) s++;
    if(s >= end) return Token(EndOfFile);

    char32_t c;
    std::u32string tmpStr;

    switch(c = *s++) {
        case '(': return Token(ParenLeft);
        case ')': return Token(ParenRight);
        case '[': return Token(BracketLeft);
        case ']': return Token(BracketRight);
        case '{': return Token(CurlyLeft);
        case '}': return Token(CurlyRight);
        case '#': return Token(Hash);
        case '\'': return Token(SingleQuote);
        case '`': return Token(Backtick);
        case '\\':
            if(s >= end) Token(Error, U"", CharEOF);
            return Token(Char, std::u32string(1, *s++));
        case '"':
            c = '\0'; // this is a hack for the error check later to work
            while(s < end && (c = *s++) != '"') {
                tmpStr += c == '\\' ? Unescape(*s++) : c;
            }
            if(s >= end && c != '"') return Token(Error, U"", StringEOF);
            return Token(String, tmpStr);
        case ';':
            while(s < end && *s != '\n') tmpStr += *s++;
            return Token(Comment, tmpStr);
        case '-':
            tmpStr += '-';
            if(s >= end) return Token(Error, U"", FloatEOF);
            c = *s++;
            [[fallthrough]];
        case '+': [[fallthrough]];
        case '0': [[fallthrough]];
        case '1': [[fallthrough]];
        case '2': [[fallthrough]];
        case '3': [[fallthrough]];
        case '4': [[fallthrough]];
        case '5': [[fallthrough]];
        case '6': [[fallthrough]];
        case '7': [[fallthrough]];
        case '8': [[fallthrough]];
        case '9':
            ReadToSeparator(tmpStr, c, s, end);
            return tmpStr.find('.') == std::u32string::npos ? Token(Int, tmpStr)
                   : tmpStr.back() == '.'                   ? Token(Error, tmpStr, FloatEOF)
                                                            : Token(Float, tmpStr);
        case '.':
            ReadToSeparator(tmpStr, c, s, end);
            return tmpStr.size() == 1 ? Token(Error, U"", FloatEOF) : Token(Float, tmpStr);
        default: ReadToSeparator(tmpStr, c, s, end); return Token(Id, tmpStr);
    }
}

/**
 * A simplified API: calls `Next` until it gets an EOF and returns all tokens
 */
inline std::vector<Token> Lex(const std::u32string &str) {
    const char32_t *s = str.c_str();
    const char32_t *end = s + str.size();
    std::vector<Token> tokens;
    do {
        tokens.push_back(Next(s, end));
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
    List = Lexer::ParenLeft,
    Vector = Lexer::BracketLeft,
    Map = Lexer::CurlyLeft,
    Comment = Lexer::Comment,
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
    uint_fast8_t hashes = 0, quotes = 0, backquotes = 0;
    std::u32string str;
    std::vector<Element> vec;
    // this is HORRIBLE, but we can't use std::map
    std::vector<std::pair<Element, Element>> map;
    union {
        int_fast64_t i;
        double d;
        char32_t c;
    };
    inline Element(enum Type t, enum Error e = None) : type(t), err(e) {}
    inline std::string to_string() const {
        std::string s = std::to_string(type) + " (" + std::to_string(err) +
                        "): " + StringMul("#", hashes) + StringMul("'", quotes);
        switch(type) {
            case Error: break;
            case EndOfFile: break;
            case Comment: [[fallthrough]];
            case Id: [[fallthrough]];
            case String: s += '"' + Utf32.to_bytes(str) + '"'; break;
            case Int: s += std::to_string(i); break;
            case Float: s += std::to_string(d); break;
            case Char: s += Utf32.to_bytes(std::u32string(1, c)); break;
            case List: [[fallthrough]];
            case Vector:
                s = std::accumulate(vec.begin(), vec.end(), s, [](auto s, auto e) {
                    return s + "(" + e.to_string() + ") ";
                });
                if(!vec.empty()) s.pop_back();
                break;
            case Map:
                s = std::accumulate(map.begin(), map.end(), s, [](auto s, auto p) {
                    return s + "(" + p.first.to_string() + ") : (" + p.second.to_string() + ") ";
                });
                if(!map.empty()) s.pop_back();
                break;
        }
        return s;
    }
};
/**
 * Parses the next `Element` out of the buffer `ts` with the end pointer `end`.
 * Increments `ts` by the length of the token.
 */
// TODO: check if this somehow can return nothing (according to gh actions it can)
inline Element Next(const Lexer::Token *&ts, const Lexer::Token *end) {
    if(ts >= end) return Element(EndOfFile);
    Lexer::Token t = *ts++;
    switch(t.type) {
        case Lexer::Error: return Element(Error, LexerError);
        case Lexer::ParenLeft: {
            Element e(List);
            do {
                if((*ts).type == Lexer::ParenRight) {
                    ts++;
                    return e;
                }
                e.vec.push_back(Next(ts, end));
            } while(e.vec.back().type != EndOfFile);
            e.err = UnmatchedParens;
            return e;
        }
        case Lexer::BracketLeft: {
            Element e(Vector);
            do {
                if((*ts).type == Lexer::BracketRight) {
                    ts++;
                    return e;
                }
                e.vec.push_back(Next(ts, end));
            } while(e.vec.back().type != EndOfFile);
            e.err = UnmatchedBrackets;
            return e;
        }
        case Lexer::CurlyLeft: {
            Element e(Map);
            while(ts <= end) {
                if((*ts).type == Lexer::CurlyRight) {
                    ts++;
                    return e;
                }
                Element key = Next(ts, end);
                if(key.type == EndOfFile || (*ts).type == Lexer::CurlyRight) {
                    ts++;
                    e.err = key.type == EndOfFile ? UnmatchedCurlies : UnmatchedMapKey;
                    return e;
                }
                Element value = Next(ts, end);
                if(value.type == EndOfFile) {
                    e.err = (enum Error)(UnmatchedCurlies | UnmatchedMapKey);
                    return e;
                }
                e.map.push_back({key, value});
            }
            return e;
        }
        case Lexer::ParenRight: return Element(Error, UnmatchedParens);
        case Lexer::BracketRight: return Element(Error, UnmatchedBrackets);
        case Lexer::CurlyRight: return Element(Error, UnmatchedCurlies);
        case Lexer::Hash: {
            Element e = Next(ts, end);
            e.hashes++;
            return e;
        }
        case Lexer::SingleQuote: {
            Element e = Next(ts, end);
            e.quotes++;
            return e;
        }
        case Lexer::Backtick: {
            Element e = Next(ts, end);
            e.backquotes++;
            return e;
        }
        case Lexer::Id: {
            Element e(Id);
            e.str = t.value;
            return e;
        }
        case Lexer::Int: {
            // TODO: support for other number systems? read std::stoll docs?
            Element e(Int);
            e.i = std::stoll(std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().to_bytes(
                    t.value));
            return e;
        }
        case Lexer::Float: {
            Element e(Float);
            e.d = std::stod(std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().to_bytes(
                    t.value));
            return e;
        }
        case Lexer::Char: {
            Element e(Char);
            e.c = t.value[0];
            return e;
        }
        case Lexer::String: {
            Element e(String);
            e.str = t.value;
            return e;
        }
        case Lexer::Comment: {
            Element e(Comment);
            e.str = t.value;
            return e;
        }
        case Lexer::EndOfFile: return Element(EndOfFile);
    }
}

/**
 * A simplified API: calls `Next` until it gets an EOF and returns all elements
 */
inline std::vector<Element> Parse(const std::vector<Lexer::Token> &tokens) {
    const Lexer::Token *ts = &tokens[0];
    const Lexer::Token *end = ts + tokens.size();
    std::vector<Element> elements;
    do {
        elements.push_back(Next(ts, end));
    } while(elements.back().type != EndOfFile);
    elements.pop_back();
    return elements;
}
}
}
}
