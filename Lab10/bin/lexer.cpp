#include "lexer.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace itmoscript {

Lexer::Lexer(std::string src_)
    : src(std::move(src_)), p(0), ch(0), ln(1), cl(1)
{
    ch = (src.empty() ? '\0' : src[0]);
}

void Lexer::advance() {
    if (ch == '\n') {
        ++ln;
        cl = 1;
    } else {
        ++cl;
    }
    if (++p < src.size()) {
        ch = src[p];
    } else {
        ch = '\0';
    }
}

char Lexer::peekChar(size_t k) const {
    size_t q = p + k;
    return (q < src.size() ? src[q] : '\0');
}

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        while (ch && (isspace(static_cast<unsigned char>(ch)) || ch == ';')) {
            advance();
        }
        if (ch == '#') {
            while (ch && ch != '\n') {
                advance();
            }
            continue;
        }
        if (ch == '/' && peekChar() == '/') {
            advance();
            advance();
            while (ch && ch != '\n') {
                advance();
            }
            continue;
        }
        break;
    }
}

Token Lexer::makeSimple(TK k, const std::string& lit) {
    int colStart = cl;
    for (size_t i = 0; i < lit.size(); ++i) {
        advance();
    }
    return {k, lit, 0.0, ln, colStart};
}

bool Lexer::wordAhead(const std::string& w) const {
    size_t q = p;
    while (q < src.size() && isspace(static_cast<unsigned char>(src[q]))) {
        ++q;
    }
    for (char c : w) {
        if (q >= src.size() || src[q] != c) {
            return false;
        }
        ++q;
    }
    char after = (q < src.size() ? src[q] : '\0');
    return !isalnum(static_cast<unsigned char>(after)) && after != '_';
}

Token Lexer::nextToken() {
    skipWhitespaceAndComments();
    if (!ch) {
        return {TK::EOF_, "", 0.0, ln, cl};
    }
    if (ch == '=' && peekChar() == '=') {
        return makeSimple(TK::EQ, "==");
    }
    if (ch == '!' && peekChar() == '=') {
        return makeSimple(TK::NEQ, "!=");
    }
    if (ch == '>' && peekChar() == '=') {
        return makeSimple(TK::GTE, ">=");
    }
    if (ch == '<' && peekChar() == '=') {
        return makeSimple(TK::LTE, "<=");
    }
    if (ch == '+' && peekChar() == '=') {
        return makeSimple(TK::PLUSEQ, "+=");
    }
    if (ch == '-' && peekChar() == '=') {
        return makeSimple(TK::MINUSEQ, "-=");
    }
    if (ch == '*' && peekChar() == '=') {
        return makeSimple(TK::MULEQ, "*=");
    }
    if (ch == '/' && peekChar() == '=') {
        return makeSimple(TK::DIVEQ, "/=");
    }
    if (ch == '%' && peekChar() == '=') {
        return makeSimple(TK::MODEQ, "%=");
    }
    if (ch == '^' && peekChar() == '=') {
        return makeSimple(TK::POWEQ, "^=");
    }
    if (isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
        int colStart = cl;
        std::string w;
        while (isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
            w.push_back(ch);
            advance();
        }
        if (w == "end") {
            skipWhitespaceAndComments();
            if (wordAhead("if")) {
                while (isspace(static_cast<unsigned char>(ch))) advance();
                for (char _ : std::string("if")) advance();
                return {TK::END_IF, "end if", 0.0, ln, colStart};
            }
            if (wordAhead("for")) {
                while (isspace(static_cast<unsigned char>(ch))) advance();
                for (char _ : std::string("for")) advance();
                return {TK::END_FOR, "end for", 0.0, ln, colStart};
            }
            if (wordAhead("function")) {
                while (isspace(static_cast<unsigned char>(ch))) advance();
                for (char _ : std::string("function")) advance();
                return {TK::END_FUNC, "end function", 0.0, ln, colStart};
            }
            if (wordAhead("while")) {
                while (isspace(static_cast<unsigned char>(ch))) advance();
                for (char _ : std::string("while")) advance();
                return {TK::END_WHILE, "end while", 0.0, ln, colStart};
            }
            return {TK::END, "end", 0.0, ln, colStart};
        }
        static const std::unordered_map<std::string, TK> kw = {
            {"function", TK::FUNC},
            {"if",       TK::IF},
            {"then",     TK::THEN},
            {"else",     TK::ELSE},
            {"for",      TK::FOR},
            {"in",       TK::IN},
            {"while",    TK::WHILE},
            {"return",   TK::RETURN},
            {"nil",      TK::NIL},
            {"print",    TK::PRINT},
            {"len",      TK::LEN},
            {"true",     TK::TRUE},
            {"false",    TK::FALSE},
            {"and",      TK::AND},
            {"or",       TK::OR},
            {"not",      TK::NOT},
            {"break",    TK::BREAK},
            {"continue", TK::CONTINUE}
        };
        auto it = kw.find(w);
        if (it != kw.end()) {
            return {it->second, w, 0.0, ln, colStart};
        }
        return {TK::IDENT, w, 0.0, ln, colStart};
    }
    if (isdigit(static_cast<unsigned char>(ch)) || (ch == '.' && isdigit(static_cast<unsigned char>(peekChar())))) {
        int colStart = cl;
        std::string digs;
        bool seenDot = false, seenExp = false;
        while (true) {
            if (isdigit(static_cast<unsigned char>(ch))) {
                digs.push_back(ch);
                advance();
            }
            else if (ch == '.' && !seenDot) {
                seenDot = true;
                digs.push_back(ch);
                advance();
            }
            else if ((ch == 'e' || ch == 'E') && !seenExp) {
                seenExp = true;
                digs.push_back('e');
                advance();
                if (ch == '+' || ch == '-') {
                    digs.push_back(ch);
                    advance();
                }
            }
            else {
                break;
            }
        }
        double val = std::stod(digs);
        return {TK::NUM, digs, val, ln, colStart};
    }
    if (ch == '"') {
        int colStart = cl;
        std::string str;
        advance();
        while (ch && ch != '"') {
            if (ch == '\\') {
                advance();
                if (!ch) {
                    throw std::runtime_error("Unterminated string literal at line " +
                                              std::to_string(ln) + ", col " + std::to_string(cl));
                }
                if (ch == 'n') {
                    str.push_back('\n');
                } else if (ch == '"') {
                    str.push_back('"');
                } else {
                    str.push_back(ch);
                }
                advance();
            } else {
                str.push_back(ch);
                advance();
            }
        }
        if (!ch) {
            throw std::runtime_error("Unterminated string literal at line " +
                                      std::to_string(ln) + ", col " + std::to_string(cl));
        }
        advance();
        return {TK::STR, str, 0.0, ln, colStart};
    }
    switch (ch) {
        case '=': return makeSimple(TK::ASSIGN, "=");
        case '>': return makeSimple(TK::GT,       ">");
        case '<': return makeSimple(TK::LT,       "<");
        case '+': return makeSimple(TK::PLUS,     "+");
        case '-': return makeSimple(TK::MINUS,    "-");
        case '*': return makeSimple(TK::MUL,      "*");
        case '/': return makeSimple(TK::DIV,      "/");
        case '%': return makeSimple(TK::MOD,      "%");
        case '^': return makeSimple(TK::CARET,    "^");
        case '(': return makeSimple(TK::LP,       "(");
        case ')': return makeSimple(TK::RP,       ")");
        case '[': return makeSimple(TK::LB,       "[");
        case ']': return makeSimple(TK::RB,       "]");
        case ',': return makeSimple(TK::COM,      ",");
        case ':': return makeSimple(TK::COLON,    ":");
        default:
            break;
    }
    char bad = ch;
    advance();
    return {TK::UNK, std::string(1, bad), 0.0, ln, cl - 1};
}

} 
