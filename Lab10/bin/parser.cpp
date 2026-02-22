#include "parser.h"
#include <iostream>

namespace itmoscript {

Parser::Parser(std::vector<Token> tokens)
    : t(std::move(tokens)), i(0)
{
}

bool Parser::at(TK k) const {
    return t[i].k == k;
}

const Token& Parser::prev() const {
    return t[i - 1];
}

const Token& Parser::peek() const {
    return t[i];
}

const Token& Parser::advance() {
    if (i < t.size()) {
        ++i;
    }
    return prev();
}

bool Parser::match(TK k) {
    if (at(k)) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(TK k, const std::string& msg) {
    if (!match(k)) {
        throw std::runtime_error("Parse error: " + msg +
            " at line " + std::to_string(t[i].ln) +
            ", col " + std::to_string(t[i].cl));
    }
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto prog = std::make_unique<Program>();
    while (!at(TK::EOF_)) {
        prog->stmts.push_back(parseStatement());
    }
    return prog;
}

StmtPtr Parser::parseStatement() {
    if (match(TK::IF)) {
        return parseIf();
    }
    if (match(TK::FOR)) {
        return parseFor();
    }
    if (match(TK::WHILE)) {
        return parseWhile();
    }
    if (match(TK::RETURN)) {
        return parseReturn();
    }
    if (match(TK::PRINT)) {
        return parsePrint();
    }
    if (match(TK::BREAK)) {
        return parseBreak();
    }
    if (match(TK::CONTINUE)) {
        return parseContinue();
    }

    if (at(TK::IDENT) && t[i + 1].k == TK::LB &&
        [&]{
            size_t depth = 1;
            size_t p2 = i + 2;
            while (p2 < t.size() && depth > 0) {
                if (t[p2].k == TK::LB)  ++depth;
                else if (t[p2].k == TK::RB) --depth;
                ++p2;
            }
            return (depth == 0 && p2 < t.size() && t[p2].k == TK::ASSIGN);
        }())
    {
        return parseIndexAssign();
    }

    if (at(TK::IDENT) && t[i + 1].k == TK::ASSIGN && t[i + 2].k == TK::FUNC) {
        return parseFunctionDef();
    }

    if (at(TK::IDENT) && t[i + 1].k == TK::ASSIGN) {
        return parseAssign();
    }

    if (at(TK::IDENT) &&
        (t[i + 1].k == TK::PLUSEQ || t[i + 1].k == TK::MINUSEQ ||
         t[i + 1].k == TK::MULEQ  || t[i + 1].k == TK::DIVEQ  ||
         t[i + 1].k == TK::MODEQ  || t[i + 1].k == TK::POWEQ))
    {
        return parseCompoundAssign();
    }

    return std::make_unique<ExprStmt>(parseExpression());
}

StmtPtr Parser::parseAssign() {
    std::string name = advance().s;
    expect(TK::ASSIGN, "'=' after identifier");
    ExprPtr val = parseExpression();
    return std::make_unique<AssignStmt>(std::move(name), std::move(val));
}

StmtPtr Parser::parseCompoundAssign() {
    std::string name = advance().s;
    TK op = advance().k;
    ExprPtr right = parseExpression();

    TK basicOp;
    switch (op) {
        case TK::PLUSEQ:  basicOp = TK::PLUS;  break;
        case TK::MINUSEQ: basicOp = TK::MINUS; break;
        case TK::MULEQ:   basicOp = TK::MUL;   break;
        case TK::DIVEQ:   basicOp = TK::DIV;   break;
        case TK::MODEQ:   basicOp = TK::MOD;   break;
        case TK::POWEQ:   basicOp = TK::CARET; break;
        default:
            throw std::runtime_error("Unexpected compound assign operator");
    }

    ExprPtr leftVar = std::make_unique<VarExpr>(name);
    ExprPtr bin     = std::make_unique<BinaryExpr>(std::move(leftVar), basicOp, std::move(right));
    return std::make_unique<AssignStmt>(std::move(name), std::move(bin));
}

StmtPtr Parser::parseIndexAssign() {
    std::string arrName = advance().s;
    expect(TK::LB, "[");
    ExprPtr idx = parseExpression();
    expect(TK::RB, "]");
    expect(TK::ASSIGN, "=");
    ExprPtr val = parseExpression();
    return std::make_unique<IndexAssignStmt>(std::move(arrName),
                                             std::move(idx),
                                             std::move(val));
}

StmtPtr Parser::parseFunctionDef() {
    std::string name = advance().s;
    expect(TK::ASSIGN, "'='");
    expect(TK::FUNC,    "function");
    expect(TK::LP,      "(");

    std::vector<std::string> params;
    if (!at(TK::RP)) {
        params.push_back(advance().s);
        while (match(TK::COM)) {
            params.push_back(advance().s);
        }
    }
    expect(TK::RP, ")");

    auto body = parseBlockUntil(TK::END_FUNC);
    return std::make_unique<FuncDef>(std::move(name),
                                     std::move(params),
                                     std::move(body));
}

StmtPtr Parser::parseIf() {
    ExprPtr cond = parseExpression();
    expect(TK::THEN, "then");

    std::vector<StmtPtr> thenBody;
    while (!at(TK::ELSE) && !at(TK::END_IF)) {
        thenBody.push_back(parseStatement());
    }

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

StmtPtr Parser::parseFor() {
    std::string var = advance().s;
    expect(TK::IN, "in");
    ExprPtr it = parseExpression();
    auto body = parseBlockUntil(TK::END_FOR);
    return std::make_unique<ForStmt>(std::move(var),
                                     std::move(it),
                                     std::move(body));
}

StmtPtr Parser::parseWhile() {
    ExprPtr cond = parseExpression();
    auto body = parseBlockUntil(TK::END_WHILE);
    return std::make_unique<WhileStmt>(std::move(cond),
                                       std::move(body));
}

StmtPtr Parser::parseReturn() {
    ExprPtr v = parseExpression();
    return std::make_unique<ReturnStmt>(std::move(v));
}

StmtPtr Parser::parsePrint() {
    expect(TK::LP, "(");
    ExprPtr v = parseExpression();
    expect(TK::RP, ")");
    return std::make_unique<PrintStmt>(std::move(v));
}

StmtPtr Parser::parseBreak() {
    return std::make_unique<BreakStmt>();
}

StmtPtr Parser::parseContinue() {
    return std::make_unique<ContinueStmt>();
}

std::vector<StmtPtr> Parser::parseBlockUntil(TK stopTok) {
    std::vector<StmtPtr> out;
    while (!at(stopTok)) {
        if (at(TK::EOF_)) {
            throw std::runtime_error("Unterminated block at line " +
                std::to_string(peek().ln) + ", col " + std::to_string(peek().cl));
        }
        out.push_back(parseStatement());
    }
    advance();
    return out;
}

ExprPtr Parser::parseExpression() {
    return parseLogicalOr();
}

ExprPtr Parser::parseLogicalOr() {
    ExprPtr expr = parseLogicalAnd();
    while (match(TK::OR)) {
        TK op = prev().k;
        ExprPtr rhs = parseLogicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::parseLogicalAnd() {
    ExprPtr expr = parseEquality();
    while (match(TK::AND)) {
        TK op = prev().k;
        ExprPtr rhs = parseEquality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::parseEquality() {
    ExprPtr expr = parseComparison();
    while (match(TK::EQ) || match(TK::NEQ)) {
        TK op = prev().k;
        ExprPtr rhs = parseComparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::parseComparison() {
    ExprPtr expr = parseTerm();
    while (match(TK::GT) || match(TK::LT) || match(TK::GTE) || match(TK::LTE)) {
        TK op = prev().k;
        ExprPtr rhs = parseTerm();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::parseTerm() {
    ExprPtr expr = parseFactor();
    while (match(TK::PLUS) || match(TK::MINUS)) {
        TK op = prev().k;
        ExprPtr rhs = parseFactor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::parseFactor() {
    ExprPtr expr = parsePower();
    while (match(TK::MUL) || match(TK::DIV) || match(TK::MOD)) {
        TK op = prev().k;
        ExprPtr rhs = parsePower();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::parsePower() {
    ExprPtr left = parseUnary();
    if (match(TK::CARET)) {
        TK op = prev().k;
        ExprPtr right = parsePower();
        return std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parseUnary() {
    if (match(TK::MINUS) || match(TK::NOT)) {
        TK op = prev().k;
        ExprPtr rhs = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(rhs));
    }
    return parsePostfix();
}

ExprPtr Parser::parsePostfix() {
    ExprPtr expr = parsePrimary();
    while (true) {
        if (match(TK::LP)) {
            std::vector<ExprPtr> args;
            if (!at(TK::RP)) {
                args.push_back(parseExpression());
                while (match(TK::COM)) {
                    args.push_back(parseExpression());
                }
            }
            expect(TK::RP, ")");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        }
        else if (match(TK::LB)) {
            if (match(TK::COLON)) {
                ExprPtr end = nullptr;
                if (!at(TK::RB)) {
                    end = parseExpression();
                }
                expect(TK::RB, "]");
                expr = std::make_unique<SliceExpr>(std::move(expr), nullptr, false, std::move(end), end != nullptr);
            }
            else {
                ExprPtr first = parseExpression();
                if (match(TK::COLON)) {
                    ExprPtr second = nullptr;
                    if (!at(TK::RB)) {
                        second = parseExpression();
                    }
                    expect(TK::RB, "]");
                    expr = std::make_unique<SliceExpr>(std::move(expr), std::move(first), true, std::move(second), second != nullptr);
                } else {
                    expect(TK::RB, "]");
                    expr = std::make_unique<IndexExpr>(std::move(expr), std::move(first));
                }
            }
        }
        else {
            break;
        }
    }
    return expr;
}

ExprPtr Parser::parsePrimary() {
    if (match(TK::NUM)) {
        return std::make_unique<NumExpr>(prev().num);
    }
    if (match(TK::STR)) {
        return std::make_unique<StrExpr>(prev().s);
    }
    if (match(TK::NIL)) {
        return std::make_unique<NilExpr>();
    }
    if (match(TK::TRUE)) {
        return std::make_unique<NumExpr>(1.0);
    }
    if (match(TK::FALSE)) {
        return std::make_unique<NumExpr>(0.0);
    }

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
            while (match(TK::COM)) {
                params.push_back(advance().s);
            }
        }
        expect(TK::RP, ")");
        auto body = parseBlockUntil(TK::END_FUNC);
        return std::make_unique<FuncLit>(std::move(params), std::move(body));
    }

    if (match(TK::IDENT)) {
        return std::make_unique<VarExpr>(prev().s);
    }

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

} 
