
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include "lexer.h"

namespace itmoscript {

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
    ForStmt(std::string v, ExprPtr it, std::vector<StmtPtr> b)
        : var(std::move(v)), iterable(std::move(it)), body(std::move(b)) {}
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

struct Program { 
    std::vector<StmtPtr> stmts;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<Program> parseProgram();

private:
    std::vector<Token> t;
    size_t i = 0;

    bool         at(TK k) const;
    const Token& prev()  const;
    const Token& peek()  const;
    const Token& advance();
    bool         match(TK k);
    void         expect(TK k, const std::string& msg);

    StmtPtr parseStatement();
    StmtPtr parseAssign();
    StmtPtr parseIndexAssign();
    StmtPtr parseFunctionDef();
    StmtPtr parseIf();
    StmtPtr parseFor();
    StmtPtr parseWhile();
    StmtPtr parseReturn();
    StmtPtr parsePrint();
    StmtPtr parseBreak();
    StmtPtr parseContinue();
    StmtPtr parseCompoundAssign();
    std::vector<StmtPtr> parseBlockUntil(TK stopTok);

    ExprPtr parseExpression();
    ExprPtr parseLogicalOr();
    ExprPtr parseLogicalAnd();
    ExprPtr parseEquality();
    ExprPtr parseComparison();
    ExprPtr parseTerm();
    ExprPtr parseFactor();
    ExprPtr parsePower();
    ExprPtr parseUnary();
    ExprPtr parsePostfix();
    ExprPtr parsePrimary();
};

} 

