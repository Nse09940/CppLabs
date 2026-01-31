
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>

namespace itmoscript {

enum class TK {
    EOF_, UNK,
    FUNC, IF, THEN, ELSE, END, FOR, IN, RETURN, NIL, PRINT, LEN, WHILE,
    END_FUNC, END_IF, END_FOR, END_WHILE,
    TRUE, FALSE,
    AND, OR, NOT, BREAK, CONTINUE,
    IDENT, NUM, STR,
    EQ, NEQ, GTE, LTE, GT, LT, ASSIGN,
    PLUS, MINUS, MUL, DIV, MOD, CARET,
    PLUSEQ, MINUSEQ, MULEQ, DIVEQ, MODEQ, POWEQ,
    LP, RP, LB, RB, COM, COLON
};

struct Token {
    TK          k;
    std::string s;
    double      num = 0.0;
    int         ln  = 1, cl = 1;

    Token(TK k_ = TK::UNK,
          std::string s_ = "",
          double n_ = 0.0,
          int l_ = 1,
          int c_ = 1)
        : k(k_), s(std::move(s_)), num(n_), ln(l_), cl(c_) {}
};

class Lexer {
public:
    explicit Lexer(std::string src);

    Token nextToken();

private:
    std::string src;
    size_t      p  = 0;
    char        ch = 0;
    int         ln = 1, cl = 1;

    void advance();
    char peekChar(size_t k = 1) const;
    void skipWhitespaceAndComments();
    Token makeSimple(TK k, const std::string& lit);
    bool wordAhead(const std::string& w) const;
};

} 

