#ifndef ITMOSCRIPT_HPP_
#define ITMOSCRIPT_HPP_
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <algorithm>
#include <random>

namespace itmoscript {

/*────────────────────────── 1. Лексер ───────────────────────────*/
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
    Token(TK k = TK::UNK, std::string s_ = "", double n_ = 0.0, int l = 1, int c = 1)
        : k(k), s(std::move(s_)), num(n_), ln(l), cl(c) {}
};

class Lexer {
    std::string src;
    size_t      p  = 0;
    char        ch = 0;
    int         ln = 1, cl = 1;

    inline void advance() {
        if (ch == '\n') { ++ln; cl = 1; }
        else ++cl;
        ch = (++p < src.size() ? src[p] : '\0');
    }

    inline char peekChar(size_t k = 1) const {
        size_t q = p + k;
        return (q < src.size() ? src[q] : '\0');
    }

    inline void skipWhitespaceAndComments() {
        while (true) {
            /* теперь ';' считается «пробельным», а также обрабатываем '#' */
            while (isspace(ch) || ch == ';')
                advance();

            if (ch == '#') {
                while (ch && ch != '\n') advance();
                continue;
            }
            if (ch == '/' && peekChar() == '/') {
                advance(); advance();
                while (ch && ch != '\n') advance();
                continue;
            }
            break;
        }
    }

    inline Token makeSimple(TK k, const std::string& lit) {
        int colStart = cl;
        for (size_t i = 0; i < lit.size(); ++i) {
            advance();
        }
        return {k, lit, 0.0, ln, colStart};
    }

    inline bool wordAhead(const std::string& w) const {
        size_t q = p;
        while (q < src.size() && isspace(src[q])) ++q;
        for (char c : w) {
            if (q >= src.size() || src[q] != c) return false;
            ++q;
        }
        char after = (q < src.size() ? src[q] : '\0');
        return !isalnum(after) && after != '_';
    }

public:
    explicit Lexer(std::string s) : src(std::move(s)) {
        ch = (src.empty() ? '\0' : src[0]);
    }

    inline Token nextToken() {
        skipWhitespaceAndComments();
        if (!ch) return {TK::EOF_, "", 0.0, ln, cl};

        // Multi-char operators
        if (ch == '=' && peekChar() == '=') { return makeSimple(TK::EQ, "=="); }
        if (ch == '!' && peekChar() == '=') { return makeSimple(TK::NEQ, "!="); }
        if (ch == '>' && peekChar() == '=') { return makeSimple(TK::GTE, ">="); }
        if (ch == '<' && peekChar() == '=') { return makeSimple(TK::LTE, "<="); }
        if (ch == '+' && peekChar() == '=') { return makeSimple(TK::PLUSEQ, "+="); }
        if (ch == '-' && peekChar() == '=') { return makeSimple(TK::MINUSEQ, "-="); }
        if (ch == '*' && peekChar() == '=') { return makeSimple(TK::MULEQ, "*="); }
        if (ch == '/' && peekChar() == '=') { return makeSimple(TK::DIVEQ, "/="); }
        if (ch == '%' && peekChar() == '=') { return makeSimple(TK::MODEQ, "%="); }
        if (ch == '^' && peekChar() == '=') { return makeSimple(TK::POWEQ, "^="); }

        // Identifiers and keywords
        if (isalpha(ch) || ch == '_') {
            int colStart = cl;
            std::string w;
            while (isalnum(ch) || ch == '_') { w.push_back(ch); advance(); }

            if (w == "end") {
                skipWhitespaceAndComments();
                if (wordAhead("if")) {
                    while (isspace(ch)) advance();
                    for (char _: std::string("if")) advance();
                    return {TK::END_IF, "end if", 0.0, ln, colStart};
                }
                if (wordAhead("for")) {
                    while (isspace(ch)) advance();
                    for (char _: std::string("for")) advance();
                    return {TK::END_FOR, "end for", 0.0, ln, colStart};
                }
                if (wordAhead("function")) {
                    while (isspace(ch)) advance();
                    for (char _: std::string("function")) advance();
                    return {TK::END_FUNC, "end function", 0.0, ln, colStart};
                }
                if (wordAhead("while")) {
                    while (isspace(ch)) advance();
                    for (char _: std::string("while")) advance();
                    return {TK::END_WHILE, "end while", 0.0, ln, colStart};
                }
                return {TK::END, "end", 0.0, ln, colStart};
            }

            static const std::unordered_map<std::string, TK> kw = {
                {"function", TK::FUNC}, {"if", TK::IF},       {"then", TK::THEN},
                {"else", TK::ELSE},     {"for", TK::FOR},     {"in", TK::IN},
                {"while", TK::WHILE},   {"return", TK::RETURN},{"nil", TK::NIL},
                {"print", TK::PRINT},   {"len", TK::LEN},     {"true", TK::TRUE},
                {"false", TK::FALSE},   {"and", TK::AND},     {"or", TK::OR},
                {"not", TK::NOT},       {"break", TK::BREAK},{"continue", TK::CONTINUE}
            };
            auto it = kw.find(w);
            if (it != kw.end()) return {it->second, w, 0.0, ln, colStart};
            return {TK::IDENT, w, 0.0, ln, colStart};
        }

        if (isdigit(ch) || (ch == '.' && isdigit(peekChar()))) {
            int colStart = cl;
            std::string digs;
            bool seenDot = false, seenExp = false;
            while (isdigit(ch) || ch == '.' || ch == 'e' || ch == 'E' || 
                   ((ch == '+' || ch == '-') && seenExp && 
                    (digs.back() == 'e' || digs.back() == 'E'))) {
                if (ch == '.') {
                    if (seenDot) break;
                    seenDot = true;
                    digs.push_back(ch);
                    advance();
                } else if ((ch == 'e' || ch == 'E') && !seenExp) {
                    seenExp = true;
                    digs.push_back('e');
                    advance();
                    if (ch == '+' || ch == '-') { digs.push_back(ch); advance(); }
                } else if (isdigit(ch)) {
                    digs.push_back(ch);
                    advance();
                } else break;
            }
            double val = std::stod(digs);
            return {TK::NUM, digs, val, ln, colStart};
        }

        // String literals
        if (ch == '"') {
            int colStart = cl;
            std::string str;
            advance();
            while (ch != '"') {
                if (!ch) throw std::runtime_error("Unterminated string literal at line " +
                                                  std::to_string(ln) + ", col " + std::to_string(cl));
                if (ch == '\\') {
                    advance();
                    if (ch == 'n') { str.push_back('\n'); }
                    else if (ch == '"') { str.push_back('"'); }
                    else { str.push_back(ch); }
                } else {
                    str.push_back(ch);
                }
                advance();
            }
            advance();
            return {TK::STR, str, 0.0, ln, colStart};
        }

        // Single-char tokens
        switch (ch) {
            case '=': return makeSimple(TK::ASSIGN, "=");
            case '>': return makeSimple(TK::GT, ">");
            case '<': return makeSimple(TK::LT, "<");
            case '+': return makeSimple(TK::PLUS, "+");
            case '-': return makeSimple(TK::MINUS, "-");
            case '*': return makeSimple(TK::MUL, "*");
            case '/': return makeSimple(TK::DIV, "/");
            case '%': return makeSimple(TK::MOD, "%");
            case '^': return makeSimple(TK::CARET, "^");
            case '(': return makeSimple(TK::LP, "(");
            case ')': return makeSimple(TK::RP, ")");
            case '[': return makeSimple(TK::LB, "[");
            case ']': return makeSimple(TK::RB, "]");
            case ',': return makeSimple(TK::COM, ",");
            case ':': return makeSimple(TK::COLON, ":");
        }

        char bad = ch;
        advance();
        return {TK::UNK, std::string(1, bad), 0.0, ln, cl - 1};
    }
};
















struct Expr     { virtual ~Expr() = default; };
using ExprPtr   = std::unique_ptr<Expr>;

struct NumExpr    : Expr { double v;    explicit NumExpr(double x): v(x) {} };
struct StrExpr    : Expr { std::string s; explicit StrExpr(std::string t): s(std::move(t)) {} };
struct NilExpr    : Expr {};
struct VarExpr    : Expr { std::string n; explicit VarExpr(std::string x): n(std::move(x)) {} };
struct UnaryExpr  : Expr { TK op; ExprPtr rhs; UnaryExpr(TK o, ExprPtr r): op(o), rhs(std::move(r)) {} };
struct BinaryExpr : Expr { ExprPtr l, r; TK op; BinaryExpr(ExprPtr a, TK o, ExprPtr b): l(std::move(a)), r(std::move(b)), op(o) {} };
struct CallExpr   : Expr { ExprPtr callee; std::vector<ExprPtr> args; CallExpr(ExprPtr c, std::vector<ExprPtr> a): callee(std::move(c)), args(std::move(a)) {} };
struct ArrayExpr  : Expr { std::vector<ExprPtr> el; explicit ArrayExpr(std::vector<ExprPtr> v): el(std::move(v)) {} };
struct LenExpr    : Expr { ExprPtr w; explicit LenExpr(ExprPtr e): w(std::move(e)) {} };
struct IndexExpr  : Expr { ExprPtr arr, idx; IndexExpr(ExprPtr a, ExprPtr i): arr(std::move(a)), idx(std::move(i)) {} };
struct SliceExpr  : Expr {
    ExprPtr arr;
    ExprPtr start; bool hasStart;
    ExprPtr end;   bool hasEnd;
    SliceExpr(ExprPtr a, ExprPtr s, bool hs, ExprPtr e, bool he)
        : arr(std::move(a)), start(std::move(s)), hasStart(hs), end(std::move(e)), hasEnd(he) {}
};
struct FuncLit    : Expr {
    std::vector<std::string> params;
    std::vector<std::unique_ptr<struct Stmt>> body;
    FuncLit(std::vector<std::string> p, std::vector<std::unique_ptr<struct Stmt>> b)
        : params(std::move(p)), body(std::move(b)) {}
};

struct Stmt     { virtual ~Stmt() = default; };
using StmtPtr   = std::unique_ptr<Stmt>;

struct ExprStmt    : Stmt { ExprPtr e; explicit ExprStmt(ExprPtr x): e(std::move(x)) {} };
struct AssignStmt  : Stmt { std::string n; ExprPtr v; AssignStmt(std::string x, ExprPtr y): n(std::move(x)), v(std::move(y)) {} };
struct IndexAssignStmt : Stmt {
    std::string arr;
    ExprPtr idx;
    ExprPtr val;
    IndexAssignStmt(std::string a, ExprPtr i, ExprPtr v)
        : arr(std::move(a)), idx(std::move(i)), val(std::move(v)) {}
};
struct ReturnStmt  : Stmt { ExprPtr v; explicit ReturnStmt(ExprPtr x): v(std::move(x)) {} };
struct PrintStmt   : Stmt { ExprPtr v; explicit PrintStmt(ExprPtr x): v(std::move(x)) {} };
struct BreakStmt   : Stmt {};
struct ContinueStmt: Stmt {};

struct IfStmt       : Stmt {
    ExprPtr cond;
    std::vector<StmtPtr> thenBody;
    std::vector<StmtPtr> elseBody;
    IfStmt(ExprPtr c, std::vector<StmtPtr> t, std::vector<StmtPtr> e)
        : cond(std::move(c)), thenBody(std::move(t)), elseBody(std::move(e)) {}
};

struct ForStmt      : Stmt {
    std::string var;
    ExprPtr iterable;
    std::vector<StmtPtr> body;
    ForStmt(std::string v, ExprPtr it, std::vector<StmtPtr> b): var(std::move(v)), iterable(std::move(it)), body(std::move(b)) {}
};

struct WhileStmt    : Stmt {
    ExprPtr cond;
    std::vector<StmtPtr> body;
    WhileStmt(ExprPtr c, std::vector<StmtPtr> b): cond(std::move(c)), body(std::move(b)) {}
};

struct FuncDef      : Stmt {
    std::string name;
    std::vector<std::string> params;
    std::vector<StmtPtr> body;
    FuncDef(std::string n, std::vector<std::string> p, std::vector<StmtPtr> b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
};

struct Program { std::vector<StmtPtr> stmts; };

/*────────────────────────── 3.  ПАРСЕР ──────────────────────────*/ 
class Parser {
    std::vector<Token> t;
    size_t i = 0;

    inline bool         at(TK k) const { return t[i].k == k; }
    inline const Token& prev()  const { return t[i - 1]; }
    inline const Token& peek()  const { return t[i]; }
    inline const Token& advance()     { if (i < t.size()) ++i; return prev(); }
    inline bool         match(TK k)   { if (at(k)) { advance(); return true; } return false; }
    inline void         expect(TK k, const std::string& msg) {
        if (!match(k)) throw std::runtime_error("Parse error: " + msg + " at line " +
            std::to_string(t[i].ln) + ", col " + std::to_string(t[i].cl));
    }

public:
    explicit Parser(std::vector<Token> v): t(std::move(v)), i(0) {}

    inline std::unique_ptr<Program> parseProgram() {
        auto prog = std::make_unique<Program>();
        while (!at(TK::EOF_)) prog->stmts.push_back(parseStatement());
        return prog;
    }

private:
    inline StmtPtr                 parseStatement();
    inline StmtPtr                 parseAssign();
    inline StmtPtr                 parseIndexAssign();
    inline StmtPtr                 parseFunctionDef();
    inline StmtPtr                 parseIf();
    inline StmtPtr                 parseFor();
    inline StmtPtr                 parseWhile();
    inline StmtPtr                 parseReturn();
    inline StmtPtr                 parsePrint();
    inline StmtPtr                 parseBreak();
    inline StmtPtr                 parseContinue();
    inline StmtPtr                 parseCompoundAssign();
    inline std::vector<StmtPtr>    parseBlockUntil(TK stopTok);

    inline ExprPtr                 parseExpression();
    inline ExprPtr                 parseLogicalOr();
    inline ExprPtr                 parseLogicalAnd();
    inline ExprPtr                 parseEquality();
    inline ExprPtr                 parseComparison();
    inline ExprPtr                 parseTerm();
    inline ExprPtr                 parseFactor();
    inline ExprPtr                 parsePower();    // для '^'
    inline ExprPtr                 parseUnary();
    inline ExprPtr                 parsePostfix();
    inline ExprPtr                 parsePrimary();
};

/*── Реализация Parser ──*/
inline StmtPtr Parser::parseAssign() {
    std::string name = advance().s;
    expect(TK::ASSIGN, "'=' after identifier");
    ExprPtr val = parseExpression();
    return std::make_unique<AssignStmt>(std::move(name), std::move(val));
}

inline StmtPtr Parser::parseCompoundAssign() {
    std::string name = advance().s;
    TK op = advance().k; // PLUSEQ, MINUSEQ, ...
    ExprPtr right = parseExpression();
    TK basicOp;
    switch (op) {
        case TK::PLUSEQ:  basicOp = TK::PLUS;  break;
        case TK::MINUSEQ: basicOp = TK::MINUS; break;
        case TK::MULEQ:   basicOp = TK::MUL;   break;
        case TK::DIVEQ:   basicOp = TK::DIV;   break;
        case TK::MODEQ:   basicOp = TK::MOD;   break;
        case TK::POWEQ:   basicOp = TK::CARET; break;
        default: throw std::runtime_error("Unexpected compound assign");
    }
    ExprPtr leftVar = std::make_unique<VarExpr>(name);
    ExprPtr bin = std::make_unique<BinaryExpr>(std::move(leftVar), basicOp, std::move(right));
    return std::make_unique<AssignStmt>(std::move(name), std::move(bin));
}

inline StmtPtr Parser::parseIndexAssign() {
    std::string arrName = advance().s;          // IDENT
    expect(TK::LB, "[");
    ExprPtr idx = parseExpression();
    expect(TK::RB, "]");
    expect(TK::ASSIGN, "=");
    ExprPtr val = parseExpression();
    return std::make_unique<IndexAssignStmt>(std::move(arrName),
                                             std::move(idx),
                                             std::move(val));
}

inline StmtPtr Parser::parseFunctionDef() {
    std::string name = advance().s;
    expect(TK::ASSIGN, "'='");
    expect(TK::FUNC, "function");
    expect(TK::LP, "(");
    std::vector<std::string> params;
    if (!at(TK::RP)) {
        params.push_back(advance().s);
        while (match(TK::COM)) params.push_back(advance().s);
    }
    expect(TK::RP, ")");
    auto body = parseBlockUntil(TK::END_FUNC);
    return std::make_unique<FuncDef>(std::move(name), std::move(params), std::move(body));
}

inline StmtPtr Parser::parseIf() {
    ExprPtr cond = parseExpression();
    expect(TK::THEN, "then");

    std::vector<StmtPtr> thenBody;
    while (!at(TK::ELSE) && !at(TK::END_IF)) thenBody.push_back(parseStatement());

    if (match(TK::ELSE)) {
        if (match(TK::IF)) {
            StmtPtr nested = parseIf();
            std::vector<StmtPtr> elseBody;
            elseBody.push_back(std::move(nested));
            return std::make_unique<IfStmt>(std::move(cond),
                                            std::move(thenBody),
                                            std::move(elseBody));
        }
        auto elseBody = parseBlockUntil(TK::END_IF);
        return std::make_unique<IfStmt>(std::move(cond),
                                        std::move(thenBody),
                                        std::move(elseBody));
    }

    expect(TK::END_IF, "end if");
    return std::make_unique<IfStmt>(std::move(cond),
                                    std::move(thenBody),
                                    std::vector<StmtPtr>{});
}

inline StmtPtr Parser::parseFor() {
    std::string var = advance().s;
    expect(TK::IN, "in");
    ExprPtr it = parseExpression();
    auto body = parseBlockUntil(TK::END_FOR);
    return std::make_unique<ForStmt>(std::move(var), std::move(it), std::move(body));
}

inline StmtPtr Parser::parseWhile() {
    ExprPtr cond = parseExpression();
    auto body = parseBlockUntil(TK::END_WHILE);
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

inline StmtPtr Parser::parseReturn() {
    ExprPtr v = parseExpression();
    return std::make_unique<ReturnStmt>(std::move(v));
}

inline StmtPtr Parser::parsePrint() {
    expect(TK::LP, "(");
    ExprPtr v = parseExpression();
    expect(TK::RP, ")");
    return std::make_unique<PrintStmt>(std::move(v));
}

inline StmtPtr Parser::parseBreak() {
    return std::make_unique<BreakStmt>();
}

inline StmtPtr Parser::parseContinue() {
    return std::make_unique<ContinueStmt>();
}

inline std::vector<StmtPtr> Parser::parseBlockUntil(TK stopTok) {
    std::vector<StmtPtr> out;
    while (!at(stopTok)) {
        if (at(TK::EOF_)) throw std::runtime_error("Unterminated block at line " +
            std::to_string(peek().ln) + ", col " + std::to_string(peek().cl));
        out.push_back(parseStatement());
    }
    advance();
    return out;
}

inline StmtPtr Parser::parseStatement() {
    if (match(TK::IF))        return parseIf();
    if (match(TK::FOR))       return parseFor();
    if (match(TK::WHILE))     return parseWhile();
    if (match(TK::RETURN))    return parseReturn();
    if (match(TK::PRINT))     return parsePrint();
    if (match(TK::BREAK))     return parseBreak();
    if (match(TK::CONTINUE))  return parseContinue();

    if (at(TK::IDENT) && t[i+1].k == TK::LB && [&]{ // looks like index assign?
            size_t depth = 1, p2 = i + 2;
            while (p2 < t.size() && depth > 0) {
                if (t[p2].k == TK::LB) ++depth;
                else if (t[p2].k == TK::RB) --depth;
                ++p2;
            }
            return (depth == 0 && p2 < t.size() && t[p2].k == TK::ASSIGN);
        }()) return parseIndexAssign();

    if (at(TK::IDENT) && t[i+1].k == TK::ASSIGN && t[i+2].k == TK::FUNC)
        return parseFunctionDef();
    if (at(TK::IDENT) && t[i+1].k == TK::ASSIGN)
        return parseAssign();
    if (at(TK::IDENT) && (t[i+1].k == TK::PLUSEQ || t[i+1].k == TK::MINUSEQ ||
                         t[i+1].k == TK::MULEQ  || t[i+1].k == TK::DIVEQ  ||
                         t[i+1].k == TK::MODEQ  || t[i+1].k == TK::POWEQ))
        return parseCompoundAssign();

    return std::make_unique<ExprStmt>(parseExpression());
}

inline ExprPtr Parser::parseExpression()  { return parseLogicalOr(); }

inline ExprPtr Parser::parseLogicalOr() {
    ExprPtr expr = parseLogicalAnd();
    while (match(TK::OR)) {
        TK op = prev().k;
        ExprPtr rhs = parseLogicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

inline ExprPtr Parser::parseLogicalAnd() {
    ExprPtr expr = parseEquality();
    while (match(TK::AND)) {
        TK op = prev().k;
        ExprPtr rhs = parseEquality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

inline ExprPtr Parser::parseEquality() {
    ExprPtr expr = parseComparison();
    while (match(TK::EQ) || match(TK::NEQ)) {
        TK op = prev().k;
        ExprPtr rhs = parseComparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

inline ExprPtr Parser::parseComparison() {
    ExprPtr expr = parseTerm();
    while (match(TK::GT) || match(TK::LT) || match(TK::GTE) || match(TK::LTE)) {
        TK op = prev().k;
        ExprPtr rhs = parseTerm();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

inline ExprPtr Parser::parseTerm() {
    ExprPtr expr = parseFactor();
    while (match(TK::PLUS) || match(TK::MINUS)) {
        TK op = prev().k;
        ExprPtr rhs = parseFactor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

inline ExprPtr Parser::parseFactor() {
    ExprPtr expr = parsePower();
    while (match(TK::MUL) || match(TK::DIV) || match(TK::MOD)) {
        TK op = prev().k;
        ExprPtr rhs = parsePower();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

// Новый уровень для '^' с право-ассоциативностью
inline ExprPtr Parser::parsePower() {
    ExprPtr left = parseUnary();
    if (match(TK::CARET)) {
        TK op = prev().k;
        ExprPtr right = parsePower();
        return std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

inline ExprPtr Parser::parseUnary() {
    if (match(TK::MINUS) || match(TK::NOT)) {
        TK op = prev().k;
        ExprPtr rhs = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(rhs));
    }
    return parsePostfix();
}

inline ExprPtr Parser::parsePostfix() {
    ExprPtr expr = parsePrimary();
    while (true) {
        if (match(TK::LP)) {
            std::vector<ExprPtr> args;
            if (!at(TK::RP)) {
                args.push_back(parseExpression());
                while (match(TK::COM)) args.push_back(parseExpression());
            }
            expect(TK::RP, ")");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        }
        else if (match(TK::LB)) {
            if (match(TK::COLON)) {
                ExprPtr end = nullptr;
                if (!at(TK::RB)) end = parseExpression();
                expect(TK::RB, "]");
                expr = std::make_unique<SliceExpr>(std::move(expr), nullptr, false, std::move(end), end != nullptr);
            }
            else {
                ExprPtr first = parseExpression();
                if (match(TK::COLON)) {
                    ExprPtr second = nullptr;
                    if (!at(TK::RB)) second = parseExpression();
                    expect(TK::RB, "]");
                    expr = std::make_unique<SliceExpr>(std::move(expr), std::move(first), true, std::move(second), second != nullptr);
                } else {
                    expect(TK::RB, "]");
                    expr = std::make_unique<IndexExpr>(std::move(expr), std::move(first));
                }
            }
        }
        else break;
    }
    return expr;
}

inline ExprPtr Parser::parsePrimary() {
    if (match(TK::NUM))  return std::make_unique<NumExpr>(prev().num);
    if (match(TK::STR))  return std::make_unique<StrExpr>(prev().s);
    if (match(TK::NIL))  return std::make_unique<NilExpr>();
    if (match(TK::TRUE)) return std::make_unique<NumExpr>(1.0);
    if (match(TK::FALSE))return std::make_unique<NumExpr>(0.0);

    if (match(TK::LEN)) {
        expect(TK::LP, "(");
        ExprPtr w = parseExpression();
        expect(TK::RP, ")");
        return std::make_unique<LenExpr>(std::move(w));
    }

    if (match(TK::FUNC)) {
        expect(TK::LP, "(");
        std::vector<std::string> params;
        if (!at(TK::RP)) {
            params.push_back(advance().s);
            while (match(TK::COM)) params.push_back(advance().s);
        }
        expect(TK::RP, ")");
        auto body = parseBlockUntil(TK::END_FUNC);
        return std::make_unique<FuncLit>(std::move(params), std::move(body));
    }

    if (match(TK::IDENT)) return std::make_unique<VarExpr>(prev().s);

    if (match(TK::LB)) {
        std::vector<ExprPtr> elems;
        if (!at(TK::RB)) {
            elems.push_back(parseExpression());
            while (match(TK::COM)) {
                if (at(TK::RB)) break;
                elems.push_back(parseExpression());
            }
        }
        expect(TK::RB, "]");
        return std::make_unique<ArrayExpr>(std::move(elems));
    }

    if (match(TK::LP)) {
        ExprPtr e = parseExpression();
        expect(TK::RP, ")");
        return e;
    }

    throw std::runtime_error("Unexpected token in primary() at line " +
        std::to_string(peek().ln) + ", col " + std::to_string(peek().cl));
}

/*────────────────────────── 4.  ИНТЕРПРЕТАТОР ─────────────────────────*/
struct Function;
using ArrayPtr = std::shared_ptr<std::vector<struct Val>>;
using FuncPtr  = std::shared_ptr<Function>;

struct Val {
    std::variant<std::monostate, double, std::string, ArrayPtr, FuncPtr> v;
    Val() {}
    Val(double x)                    : v(x) {}
    Val(std::string s)               : v(std::move(s)) {}
    Val(ArrayPtr a)                  : v(std::move(a)) {}
    Val(FuncPtr f)                   : v(std::move(f)) {}
};

static inline std::string doubleToString(double x) {
    if (std::isnan(x)) return "nan";
    if (std::isinf(x)) return x > 0 ? "inf" : "-inf";
    double intpart;
    if (std::modf(x, &intpart) == 0.0) {
        return std::to_string(static_cast<long long>(intpart));
    }
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

static inline std::string toString(const Val& val) {
    struct V {
        std::string operator()(std::monostate)       const { return "nil"; }
        std::string operator()(double x)              const { return doubleToString(x); }
        std::string operator()(const std::string& s)  const { return s; }
        std::string operator()(const ArrayPtr& p)     const {
            std::string out = "[";
            for (size_t i = 0; i < p->size(); ++i) {
                out += toString((*p)[i]);
                if (i + 1 < p->size()) out += ", ";
            }
            return out + "]";
        }
        std::string operator()(const FuncPtr&) const { return "<fn>"; }
    };
    return std::visit(V{}, val.v);
}

static inline std::string display(const Val& val) {
    struct V {
        static bool needsQuotes(const std::string& s) {
            if (s.empty()) return false;
            bool inside = false;
            for (size_t i = 0; i < s.size(); ++i) {
                char c = s[i];
                if (c == '\n' || c == '\r') return false;
                if ((c == ' ' || c == '\t') && i + 1 < s.size())
                    inside = true;
            }
            return inside;
        }

        static std::string quote(const std::string& s) {
            std::string res = "\"";
            for (char c : s) {
                if (c == '"') res += "\\\"";
                else          res += c;
            }
            res += "\"";
            return res;
        }

        std::string operator()(std::monostate)       const { return "nil"; }
        std::string operator()(double x)              const { return doubleToString(x); }
        std::string operator()(const std::string& s)  const {
            return needsQuotes(s) ? std::string("\"") + s + "\"" : s;
        }
        std::string operator()(const ArrayPtr& p)     const {
            std::string out = "[";
            for (size_t i = 0; i < p->size(); ++i) {
                const Val& elem = (*p)[i];
                if (std::holds_alternative<std::string>(elem.v))
                    out += quote(std::get<std::string>(elem.v));
                else
                    out += display(elem);
                if (i + 1 < p->size()) out += ", ";
            }
            return out + "]";
        }
        std::string operator()(const FuncPtr&) const { return "<fn>"; }
    };
    return std::visit(V{}, val.v);
}

static inline bool isTruthy(const Val& v) {
    if (std::holds_alternative<std::monostate>(v.v)) return false;
    if (auto p = std::get_if<double>(&v.v))             return (*p) != 0.0;
    if (auto p = std::get_if<std::string>(&v.v))         return !p->empty();
    return true;
}

struct Env {
    std::unordered_map<std::string, Val> table;
    std::shared_ptr<Env> parent;
    bool isFunction = false;

    explicit Env(std::shared_ptr<Env> p = nullptr, bool isFunction_ = false)
        : parent(std::move(p)), isFunction(isFunction_) {}

    Val get(const std::string& k) const {
        auto it = table.find(k);
        if (it != table.end()) return it->second;
        if (parent) return parent->get(k);
        throw std::runtime_error("Undefined variable: " + k);
    }

    void define(const std::string& k, Val v) {
        table[k] = std::move(v);
    }

    void assign(const std::string& k, Val v) {
        auto it = table.find(k);
        if (it != table.end()) { it->second = std::move(v); return; }
        if (isFunction) {
            for (auto p = parent; p; p = p->parent) {
                auto it2 = p->table.find(k);
                if (it2 != p->table.end()) { it2->second = std::move(v); return; }
                if (!p->isFunction) break;
            }
            table[k] = std::move(v);
            return;
        }
        if (parent) { parent->assign(k, std::move(v)); return; }
        table[k] = std::move(v);
    }
};

struct Function {
    std::vector<std::string> params;
    std::vector<const Stmt*> body;
    std::shared_ptr<Env> closure;

    Function(std::vector<std::string> p,
             std::vector<const Stmt*> b,
             std::shared_ptr<Env> c)
        : params(std::move(p)), body(std::move(b)), closure(std::move(c)) {}
};

struct ReturnException : std::exception {
    Val value;
    explicit ReturnException(Val v): value(std::move(v)) {}
    const char* what() const noexcept override { return "return"; }
};

struct BreakException : std::exception {
    const char* what() const noexcept override { return "break"; }
};

struct ContinueException : std::exception {
    const char* what() const noexcept override { return "continue"; }
};

class VM {
    std::shared_ptr<Env> globals;
    std::ostream* out = nullptr;

    Val ev(const Expr& e, std::shared_ptr<Env> env);
    void ex(const Stmt& s, std::shared_ptr<Env> env);

    inline std::shared_ptr<Env> findClosureEnv(std::shared_ptr<Env> env) {
        return env;  // лексическое (static) замыкание
    }

    std::mt19937 rng{std::random_device{}()};

public:
    bool run(const std::string& src, std::ostream& output);
};

inline Val VM::ev(const Expr& e, std::shared_ptr<Env> env) {
    if (auto p = dynamic_cast<const NumExpr*>(&e))  return Val(p->v);
    if (auto p = dynamic_cast<const StrExpr*>(&e))  return Val(p->s);
    if (dynamic_cast<const NilExpr*>(&e))           return Val{};
    if (auto p = dynamic_cast<const VarExpr*>(&e))  return env->get(p->n);

    if (auto p = dynamic_cast<const UnaryExpr*>(&e)) {
        Val r = ev(*p->rhs, env);
        switch (p->op) {
            case TK::MINUS: {
                if (auto pd = std::get_if<double>(&r.v))
                    return Val(-(*pd));
                throw std::runtime_error("Bad unary '-' operand");
            }
            case TK::NOT:
                return Val(isTruthy(r) ? 0.0 : 1.0);
            default:
                throw std::runtime_error("Bad unary op");
        }
    }

    if (auto p = dynamic_cast<const IndexExpr*>(&e)) {
        Val A = ev(*p->arr, env);
        Val I = ev(*p->idx, env);
        if (!std::holds_alternative<ArrayPtr>(A.v) && !std::holds_alternative<std::string>(A.v))
            throw std::runtime_error("Indexing requires array or string");
        if (!std::holds_alternative<double>(I.v))
            throw std::runtime_error("Index must be number");
        int idx = static_cast<int>(std::get<double>(I.v));
        if (std::holds_alternative<std::string>(A.v)) {
            const auto& s = std::get<std::string>(A.v);
            int len = static_cast<int>(s.size());
            if (idx < 0) idx = len + idx;
            if (idx < 0 || idx >= len) throw std::runtime_error("String index out of bounds");
            return Val(std::string(1, s[idx]));
        } else {
            const auto& vec = *std::get<ArrayPtr>(A.v);
            int len = static_cast<int>(vec.size());
            if (idx < 0) idx = len + idx;
            if (idx < 0 || idx >= len) throw std::runtime_error("Array index out of bounds");
            return vec[idx];
        }
    }

    if (auto p = dynamic_cast<const SliceExpr*>(&e)) {
        Val A = ev(*p->arr, env);
        int start = 0, end = 0;
        if (std::holds_alternative<std::string>(A.v)) {
            const auto& s = std::get<std::string>(A.v);
            int len = static_cast<int>(s.size());
            if (p->hasStart) {
                Val S = ev(*p->start, env);
                if (!std::holds_alternative<double>(S.v))
                    throw std::runtime_error("Slice start must be number");
                start = static_cast<int>(std::get<double>(S.v));
                if (start < 0) start = len + start;
            }
            if (p->hasEnd) {
                Val E = ev(*p->end, env);
                if (!std::holds_alternative<double>(E.v))
                    throw std::runtime_error("Slice end must be number");
                end = static_cast<int>(std::get<double>(E.v));
                if (end < 0) end = len + end;
            }
            int sidx = p->hasStart ? std::clamp(start, 0, len) : 0;
            int eidx = p->hasEnd   ? std::clamp(end, 0, len)   : len;
            if (eidx < sidx) eidx = sidx;
            return Val(s.substr(sidx, eidx - sidx));
        } else if (std::holds_alternative<ArrayPtr>(A.v)) {
            const auto& vec = *std::get<ArrayPtr>(A.v);
            int len = static_cast<int>(vec.size());
            if (p->hasStart) {
                Val S = ev(*p->start, env);
                if (!std::holds_alternative<double>(S.v))
                    throw std::runtime_error("Slice start must be number");
                start = static_cast<int>(std::get<double>(S.v));
                if (start < 0) start = len + start;
            }
            if (p->hasEnd) {
                Val E = ev(*p->end, env);
                if (!std::holds_alternative<double>(E.v))
                    throw std::runtime_error("Slice end must be number");
                end = static_cast<int>(std::get<double>(E.v));
                if (end < 0) end = len + end;
            }
            int sidx = p->hasStart ? std::clamp(start, 0, len) : 0;
            int eidx = p->hasEnd   ? std::clamp(end, 0, len)   : len;
            if (eidx < sidx) eidx = sidx;
            auto arr = std::make_shared<std::vector<Val>>();
            for (int k = sidx; k < eidx; ++k) arr->push_back(vec[k]);
            return Val(arr);
        } else {
            throw std::runtime_error("Slice requires string or array");
        }
    }

    if (auto p = dynamic_cast<const BinaryExpr*>(&e)) {
        Val L = ev(*p->l, env);
        Val R = ev(*p->r, env);

        auto bothNum = [&](const Val& x, const Val& y) {
            return std::holds_alternative<double>(x.v) && std::holds_alternative<double>(y.v);
        };
        auto bothStr = [&](const Val& x, const Val& y) {
            return std::holds_alternative<std::string>(x.v) && std::holds_alternative<std::string>(y.v);
        };
        auto bothArr = [&](const Val& x, const Val& y) {
            return std::holds_alternative<ArrayPtr>(x.v) && std::holds_alternative<ArrayPtr>(y.v);
        };
        auto arrNum  = [&](const Val& x, const Val& y) {
            return std::holds_alternative<ArrayPtr>(x.v) && std::holds_alternative<double>(y.v);
        };
        auto numArr  = [&](const Val& x, const Val& y) {
            return std::holds_alternative<double>(x.v) && std::holds_alternative<ArrayPtr>(y.v);
        };

        switch (p->op) {
            case TK::PLUS:
                if (bothNum(L, R)) {
                    return Val(std::get<double>(L.v) + std::get<double>(R.v));
                }
                if (bothStr(L, R)) {
                    return Val(std::get<std::string>(L.v) + std::get<std::string>(R.v));
                }
                if (bothArr(L, R)) {
                    auto a1 = std::get<ArrayPtr>(L.v);
                    auto a2 = std::get<ArrayPtr>(R.v);
                    auto out = std::make_shared<std::vector<Val>>(*a1);
                    out->insert(out->end(), a2->begin(), a2->end());
                    return Val(out);
                }
                throw std::runtime_error("Illegal '+' operands");
            case TK::MINUS:
                if (bothNum(L, R)) {
                    return Val(std::get<double>(L.v) - std::get<double>(R.v));
                }
                if (bothStr(L, R)) {
                    std::string s1 = std::get<std::string>(L.v);
                    std::string s2 = std::get<std::string>(R.v);
                    if (s1.size() >= s2.size() &&
                        s1.compare(s1.size() - s2.size(), s2.size(), s2) == 0) {
                        return Val(s1.substr(0, s1.size() - s2.size()));
                    }
                    return Val(s1);
                }
                throw std::runtime_error("Illegal '-' operands");
            case TK::MUL:
                if (bothNum(L, R)) {
                    return Val(std::get<double>(L.v) * std::get<double>(R.v));
                }
                if ((std::holds_alternative<std::string>(L.v) && std::holds_alternative<double>(R.v)) ||
                    (std::holds_alternative<double>(L.v) && std::holds_alternative<std::string>(R.v))) {
                    std::string base = std::holds_alternative<std::string>(L.v)
                                        ? std::get<std::string>(L.v)
                                        : std::get<std::string>(R.v);
                    int times = static_cast<int>(std::holds_alternative<double>(L.v)
                                        ? std::get<double>(L.v)
                                        : std::get<double>(R.v));
                    if (times < 0) times = 0;
                    std::string out;
                    while (times--) out += base;
                    return Val(out);
                }
                if (arrNum(L, R)) {
                    auto arr = std::get<ArrayPtr>(L.v);
                    int times = static_cast<int>(std::get<double>(R.v));
                    if (times < 0) times = 0;
                    auto out = std::make_shared<std::vector<Val>>();
                    for (int k = 0; k < times; ++k) {
                        out->insert(out->end(), arr->begin(), arr->end());
                    }
                    return Val(out);
                }
                if (numArr(L, R)) {
                    int times = static_cast<int>(std::get<double>(L.v));
                    if (times < 0) times = 0;
                    auto arr = std::get<ArrayPtr>(R.v);
                    auto out = std::make_shared<std::vector<Val>>();
                    for (int k = 0; k < times; ++k) {
                        out->insert(out->end(), arr->begin(), arr->end());
                    }
                    return Val(out);
                }
                throw std::runtime_error("Illegal '*' operands");
            case TK::DIV:
                if (bothNum(L, R)) {
                    double a = std::get<double>(L.v);
                    double b = std::get<double>(R.v);
                    if (b == 0.0) throw std::runtime_error("Division by zero");
                    return Val(a / b);
                }
                throw std::runtime_error("Illegal '/' operands");
            case TK::MOD:
                if (bothNum(L, R)) {
                    double a = std::get<double>(L.v);
                    double b = std::get<double>(R.v);
                    if (b == 0.0) throw std::runtime_error("Modulo by zero");
                    return Val(std::fmod(a, b));  // плавающее взятие остатка
                }
                throw std::runtime_error("Illegal '%' operands");
            case TK::CARET:
                if (bothNum(L, R)) {
                    return Val(std::pow(std::get<double>(L.v), std::get<double>(R.v)));
                }
                throw std::runtime_error("Illegal '^' operands");
            case TK::EQ:
            case TK::NEQ: {
                bool eq = false;
                if (L.v.index() == R.v.index()) {
                    eq = (toString(L) == toString(R));
                }
                return Val(p->op == TK::EQ ? (eq ? 1.0 : 0.0) : (!eq ? 1.0 : 0.0));
            }
            case TK::GT:
            case TK::LT:
            case TK::GTE:
            case TK::LTE:
                if (bothNum(L, R)) {
                    double a = std::get<double>(L.v), b = std::get<double>(R.v);
                    switch (p->op) {
                        case TK::GT:  return Val(a > b ? 1.0 : 0.0);
                        case TK::LT:  return Val(a < b ? 1.0 : 0.0);
                        case TK::GTE: return Val(a >= b ? 1.0 : 0.0);
                        case TK::LTE: return Val(a <= b ? 1.0 : 0.0);
                        default: break;
                    }
                }
                if (bothStr(L, R)) {
                    const auto& s1 = std::get<std::string>(L.v);
                    const auto& s2 = std::get<std::string>(R.v);
                    switch (p->op) {
                        case TK::GT:  return Val(s1 > s2 ? 1.0 : 0.0);
                        case TK::LT:  return Val(s1 < s2 ? 1.0 : 0.0);
                        case TK::GTE: return Val(s1 >= s2 ? 1.0 : 0.0);
                        case TK::LTE: return Val(s1 <= s2 ? 1.0 : 0.0);
                        default: break;
                    }
                }
                throw std::runtime_error("Illegal comparison operands");
            case TK::AND:
                return Val(isTruthy(L) && isTruthy(R) ? 1.0 : 0.0);
            case TK::OR:
                return Val(isTruthy(L) || isTruthy(R) ? 1.0 : 0.0);
            default:
                throw std::runtime_error("Unknown binary op");
        }
    }

    if (auto p = dynamic_cast<const ArrayExpr*>(&e)) {
        auto arr = std::make_shared<std::vector<Val>>();
        for (auto& el : p->el) arr->push_back(ev(*el, env));
        return Val(arr);
    }

    if (auto p = dynamic_cast<const LenExpr*>(&e)) {
        Val v = ev(*p->w, env);
        if (std::holds_alternative<std::string>(v.v))
            return Val(static_cast<double>(std::get<std::string>(v.v).size()));
        if (std::holds_alternative<ArrayPtr>(v.v))
            return Val(static_cast<double>(std::get<ArrayPtr>(v.v)->size()));
        throw std::runtime_error("len() expects string or array");
    }

    if (auto p = dynamic_cast<const FuncLit*>(&e)) {
        auto closureEnv = findClosureEnv(env);
        std::vector<const Stmt*> fn_body;
        for (auto& st : p->body) fn_body.push_back(st.get());
        auto fn = std::make_shared<Function>(p->params, fn_body, closureEnv);
        return Val(fn);
    }

    if (auto p = dynamic_cast<const CallExpr*>(&e)) {
        // If callee is a simple name, check builtins
        if (auto cv = dynamic_cast<const VarExpr*>(p->callee.get())) {
            const std::string& name = cv->n;

            // print
            if (name == "print") {
                if (p->args.size() != 1) throw std::runtime_error("print() takes 1 arg");
                Val v = ev(*p->args[0], env);
                (*out) << display(v);
                return Val{};
            }
            // println
            if (name == "println") {
                if (p->args.size() != 1) throw std::runtime_error("println() takes 1 arg");
                Val v = ev(*p->args[0], env);
                (*out) << display(v) << "\n";
                return Val{};
            }
            // range
            if (name == "range") {
                size_t cnt = p->args.size();
                if (cnt < 1 || cnt > 3) throw std::runtime_error("range() takes 1–3 args");
                int start = 0, end = 0, step = 1;
                if (cnt == 1) {
                    end = static_cast<int>(std::get<double>(ev(*p->args[0], env).v));
                }
                else if (cnt == 2) {
                    start = static_cast<int>(std::get<double>(ev(*p->args[0], env).v));
                    end   = static_cast<int>(std::get<double>(ev(*p->args[1], env).v));
                } else {
                    start = static_cast<int>(std::get<double>(ev(*p->args[0], env).v));
                    end   = static_cast<int>(std::get<double>(ev(*p->args[1], env).v));
                    step  = static_cast<int>(std::get<double>(ev(*p->args[2], env).v));
                    if (step == 0) throw std::runtime_error("range() step cannot be zero");
                }
                auto arr = std::make_shared<std::vector<Val>>();
                if (step > 0)  for (int k = start; k <  end; k += step) arr->push_back(Val(static_cast<double>(k)));
                else           for (int k = start; k >  end; k += step) arr->push_back(Val(static_cast<double>(k)));
                return Val(arr);
            }
            // abs
            if (name == "abs") {
                if (p->args.size() != 1) throw std::runtime_error("abs() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) throw std::runtime_error("abs() expects number");
                return Val(std::fabs(std::get<double>(v.v)));
            }
            // ceil
            if (name == "ceil") {
                if (p->args.size() != 1) throw std::runtime_error("ceil() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) throw std::runtime_error("ceil() expects number");
                return Val(std::ceil(std::get<double>(v.v)));
            }
            // floor
            if (name == "floor") {
                if (p->args.size() != 1) throw std::runtime_error("floor() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) throw std::runtime_error("floor() expects number");
                return Val(std::floor(std::get<double>(v.v)));
            }
            // round
            if (name == "round") {
                if (p->args.size() != 1) throw std::runtime_error("round() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) throw std::runtime_error("round() expects number");
                return Val(std::round(std::get<double>(v.v)));
            }
            // sqrt
            if (name == "sqrt") {
                if (p->args.size() != 1) throw std::runtime_error("sqrt() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) throw std::runtime_error("sqrt() expects number");
                return Val(std::sqrt(std::get<double>(v.v)));
            }
            // rnd
            if (name == "rnd") {
                if (p->args.size() != 1) throw std::runtime_error("rnd() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) throw std::runtime_error("rnd() expects number");
                int n = static_cast<int>(std::get<double>(v.v));
                if (n <= 0) return Val(0.0);
                std::uniform_int_distribution<int> dist(0, n - 1);
                return Val(static_cast<double>(dist(rng)));
            }
            // parse_num
            if (name == "parse_num") {
                if (p->args.size() != 1) throw std::runtime_error("parse_num() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<std::string>(v.v)) throw std::runtime_error("parse_num() expects string");
                const auto& s = std::get<std::string>(v.v);
                try {
                    double x = std::stod(s);
                    return Val(x);
                } catch (...) {
                    return Val{};
                }
            }
            // to_string
            if (name == "to_string") {
                if (p->args.size() != 1) throw std::runtime_error("to_string() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) throw std::runtime_error("to_string() expects number");
                return Val(doubleToString(std::get<double>(v.v)));
            }
            // lower
            if (name == "lower") {
                if (p->args.size() != 1) throw std::runtime_error("lower() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<std::string>(v.v)) throw std::runtime_error("lower() expects string");
                std::string s = std::get<std::string>(v.v);
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                return Val(s);
            }
            // upper
            if (name == "upper") {
                if (p->args.size() != 1) throw std::runtime_error("upper() takes 1 arg");
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<std::string>(v.v)) throw std::runtime_error("upper() expects string");
                std::string s = std::get<std::string>(v.v);
                std::transform(s.begin(), s.end(), s.begin(), ::toupper);
                return Val(s);
            }
            // split
            if (name == "split") {
                if (p->args.size() != 2) throw std::runtime_error("split() takes 2 args");
                Val vs = ev(*p->args[0], env);
                Val vd = ev(*p->args[1], env);
                if (!std::holds_alternative<std::string>(vs.v) || !std::holds_alternative<std::string>(vd.v))
                    throw std::runtime_error("split() expects (string, string)");
                const std::string& s = std::get<std::string>(vs.v);
                const std::string& delim = std::get<std::string>(vd.v);
                auto arr = std::make_shared<std::vector<Val>>();
                if (delim.empty()) {
                    for (char c : s) arr->push_back(Val(std::string(1,c)));
                    return Val(arr);
                }
                size_t pos = 0, start = 0;
                while ((pos = s.find(delim, start)) != std::string::npos) {
                    arr->push_back(Val(s.substr(start, pos - start)));
                    start = pos + delim.size();
                }
                arr->push_back(Val(s.substr(start)));
                return Val(arr);
            }
            // join
            if (name == "join") {
                if (p->args.size() != 2) throw std::runtime_error("join() takes 2 args");
                Val va = ev(*p->args[0], env);
                Val vd = ev(*p->args[1], env);
                if (!std::holds_alternative<ArrayPtr>(va.v) || !std::holds_alternative<std::string>(vd.v))
                    throw std::runtime_error("join() expects (array, string)");
                const auto& arr = *std::get<ArrayPtr>(va.v);
                const std::string& delim = std::get<std::string>(vd.v);
                std::string out;
                for (size_t i = 0; i < arr.size(); ++i) {
                    if (!std::holds_alternative<std::string>(arr[i].v))
                        throw std::runtime_error("join() array elements must be strings");
                    out += std::get<std::string>(arr[i].v);
                    if (i + 1 < arr.size()) out += delim;
                }
                return Val(out);
            }
            // replace
            if (name == "replace") {
                if (p->args.size() != 3) throw std::runtime_error("replace() takes 3 args");
                Val vs = ev(*p->args[0], env);
                Val vo = ev(*p->args[1], env);
                Val vn = ev(*p->args[2], env);
                if (!std::holds_alternative<std::string>(vs.v) ||
                    !std::holds_alternative<std::string>(vo.v) ||
                    !std::holds_alternative<std::string>(vn.v))
                    throw std::runtime_error("replace() expects (string, string, string)");
                std::string s = std::get<std::string>(vs.v);
                std::string oldp = std::get<std::string>(vo.v);
                std::string newp = std::get<std::string>(vn.v);
                if (oldp.empty()) return Val(s);
                size_t pos = 0;
                while ((pos = s.find(oldp, pos)) != std::string::npos) {
                    s.replace(pos, oldp.size(), newp);
                    pos += newp.size();
                }
                return Val(s);
            }
            // push
            if (name == "push") {
                if (p->args.size() != 2) throw std::runtime_error("push() takes 2 args");
                Val va = ev(*p->args[0], env);
                Val vx = ev(*p->args[1], env);
                if (!std::holds_alternative<ArrayPtr>(va.v)) throw std::runtime_error("push() expects array");
                auto arr = std::get<ArrayPtr>(va.v);
                arr->push_back(vx);
                return Val{};
            }
            // pop
            if (name == "pop") {
                if (p->args.size() != 1) throw std::runtime_error("pop() takes 1 arg");
                Val va = ev(*p->args[0], env);
                if (!std::holds_alternative<ArrayPtr>(va.v)) throw std::runtime_error("pop() expects array");
                auto arr = std::get<ArrayPtr>(va.v);
                if (arr->empty()) return Val{};
                Val ret = arr->back();
                arr->pop_back();
                return ret;
            }
            // insert
            if (name == "insert") {
                if (p->args.size() != 3) throw std::runtime_error("insert() takes 3 args");
                Val va = ev(*p->args[0], env);
                Val vi = ev(*p->args[1], env);
                Val vx = ev(*p->args[2], env);
                if (!std::holds_alternative<ArrayPtr>(va.v) || !std::holds_alternative<double>(vi.v))
                    throw std::runtime_error("insert() expects (array, number, value)");
                auto arr = std::get<ArrayPtr>(va.v);
                int idx = static_cast<int>(std::get<double>(vi.v));
                if (idx < 0) idx = 0;
                if (idx > static_cast<int>(arr->size())) idx = static_cast<int>(arr->size());
                arr->insert(arr->begin() + idx, vx);
                return Val{};
            }
            // remove
            if (name == "remove") {
                if (p->args.size() != 2) throw std::runtime_error("remove() takes 2 args");
                Val va = ev(*p->args[0], env);
                Val vi = ev(*p->args[1], env);
                if (!std::holds_alternative<ArrayPtr>(va.v) || !std::holds_alternative<double>(vi.v))
                    throw std::runtime_error("remove() expects (array, number)");
                auto arr = std::get<ArrayPtr>(va.v);
                int idx = static_cast<int>(std::get<double>(vi.v));
                int len = static_cast<int>(arr->size());
                if (idx < 0) idx = len + idx;
                if (idx < 0 || idx >= len) throw std::runtime_error("remove() index out of bounds");
                arr->erase(arr->begin() + idx);
                return Val{};
            }
            // sort
            if (name == "sort") {
                if (p->args.size() != 1) throw std::runtime_error("sort() takes 1 arg");
                Val va = ev(*p->args[0], env);
                if (!std::holds_alternative<ArrayPtr>(va.v)) throw std::runtime_error("sort() expects array");
                auto arr = std::get<ArrayPtr>(va.v);
                if (arr->empty()) return Val{};
                if (std::holds_alternative<double>((*arr)[0].v)) {
                    std::sort(arr->begin(), arr->end(), [](const Val& a, const Val& b) {
                        return std::get<double>(a.v) < std::get<double>(b.v);
                    });
                } else if (std::holds_alternative<std::string>((*arr)[0].v)) {
                    std::sort(arr->begin(), arr->end(), [](const Val& a, const Val& b) {
                        return std::get<std::string>(a.v) < std::get<std::string>(b.v);
                    });
                } else {
                    throw std::runtime_error("sort() array elements must be all numbers or all strings");
                }
                return Val{};
            }
            // read (not implemented -> error)
            if (name == "read") {
                throw std::runtime_error("read() not implemented");
            }
            // stacktrace (not implemented -> error)
            if (name == "stacktrace") {
                throw std::runtime_error("stacktrace() not implemented");
            }
        }

        // Otherwise, call user-defined function
        Val cal = ev(*p->callee, env);
        if (!std::holds_alternative<FuncPtr>(cal.v)) throw std::runtime_error("Call on non-function");
        auto fn = std::get<FuncPtr>(cal.v);
        if (fn->params.size() != p->args.size())
            throw std::runtime_error("Argument count mismatch");

        auto frame = std::make_shared<Env>(fn->closure, true);
        for (size_t i = 0; i < fn->params.size(); ++i) {
            frame->define(fn->params[i], ev(*p->args[i], env));
        }

        try {
            for (const Stmt* st : fn->body) ex(*st, frame);
        } catch (ReturnException& r) {
            return r.value;
        }
        return Val{};
    }

    throw std::runtime_error("Unknown expression type");
}

inline void VM::ex(const Stmt& s, std::shared_ptr<Env> env) {
    if (auto p = dynamic_cast<const ExprStmt*>(&s)) { ev(*p->e, env); return; }

    if (auto p = dynamic_cast<const AssignStmt*>(&s)) {
        env->assign(p->n, ev(*p->v, env));
        return;
    }

    if (auto p = dynamic_cast<const IndexAssignStmt*>(&s)) {
        Val arrVal = env->get(p->arr);
        if (!std::holds_alternative<ArrayPtr>(arrVal.v))
            throw std::runtime_error("Index assignment requires array on the left");
        auto arr = std::get<ArrayPtr>(arrVal.v);

        Val idxVal = ev(*p->idx, env);
        if (!std::holds_alternative<double>(idxVal.v))
            throw std::runtime_error("Index must be number");
        int idx = static_cast<int>(std::get<double>(idxVal.v));
        if (idx < 0) throw std::runtime_error("Negative index not supported");

        if (idx >= static_cast<int>(arr->size())) arr->resize(idx + 1);
        (*arr)[idx] = ev(*p->val, env);
        return;
    }

    if (auto p = dynamic_cast<const ReturnStmt*>(&s)) {
        throw ReturnException(ev(*p->v, env));
    }

    if (auto p = dynamic_cast<const PrintStmt*>(&s)) {
        (*out) << display(ev(*p->v, env));
        return;
    }

    if (auto p = dynamic_cast<const BreakStmt*>(&s)) {
        throw BreakException();
    }

    if (auto p = dynamic_cast<const ContinueStmt*>(&s)) {
        throw ContinueException();
    }

    if (auto p = dynamic_cast<const IfStmt*>(&s)) {
        if (isTruthy(ev(*p->cond, env))) {
            for (auto& st : p->thenBody) ex(*st, env);
        }
        else {
            for (auto& st : p->elseBody) ex(*st, env);
        }
        return;
    }

    if (auto p = dynamic_cast<const ForStmt*>(&s)) {
        Val v = ev(*p->iterable, env);
        if (!std::holds_alternative<ArrayPtr>(v.v)) throw std::runtime_error("for expects array");
        for (const Val& elem : *std::get<ArrayPtr>(v.v)) {
            auto loopEnv = std::make_shared<Env>(env, false);
            loopEnv->define(p->var, elem);
            try {
                for (auto& st : p->body) ex(*st, loopEnv);
            } catch (ContinueException&) {
                continue;
            } catch (BreakException&) {
                break;
            }
        }
        return;
    }

    if (auto p = dynamic_cast<const WhileStmt*>(&s)) {
        while (isTruthy(ev(*p->cond, env))) {
            auto loopEnv = std::make_shared<Env>(env, false);
            try {
                for (auto& st : p->body) ex(*st, loopEnv);
            } catch (ContinueException&) {
                continue;
            } catch (BreakException&) {
                break;
            }
        }
        return;
    }

    if (auto p = dynamic_cast<const FuncDef*>(&s)) {
        auto closureEnv = findClosureEnv(env);
        std::vector<const Stmt*> fn_body;
        for (auto& st : p->body) fn_body.push_back(st.get());
        auto fn = std::make_shared<Function>(p->params, fn_body, closureEnv);
        env->define(p->name, Val(fn));
        return;
    }

    throw std::runtime_error("Unknown statement type");
}

inline bool VM::run(const std::string& src, std::ostream& output) {
    out = &output;
    try {
        Lexer lx(src);
        std::vector<Token> toks;
        for (;;) {
            Token tk = lx.nextToken();
            toks.push_back(tk);
            if (tk.k == TK::EOF_) break;
        }
        Parser ps(std::move(toks));
        auto prog = ps.parseProgram();

        globals = std::make_shared<Env>(nullptr, false);
        for (auto& st : prog->stmts) ex(*st, globals);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

/*────────────────────────── 5. PUBLIC API ─────────────────────────*/
static inline bool interpret(std::istream& in, std::ostream& out) {
    std::string src((std::istreambuf_iterator<char>(in)), {});
    VM vm;
    return vm.run(src, out);
}

} // namespace itmoscript

inline bool interpret(std::istream& in, std::ostream& out) {
    return itmoscript::interpret(in, out);
}

#endif /* ITMOSCRIPT_HPP_ */
