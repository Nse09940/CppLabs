

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <cmath>

#include "lexer.h"
#include "parser.h"

namespace itmoscript {

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

std::string doubleToString(double x);
std::string toString(const Val& val);
std::string display(const Val& val);
bool isTruthy(const Val& v);

struct Env {
    std::unordered_map<std::string, Val> table;
    std::shared_ptr<Env> parent;
    bool isFunction = false;

    explicit Env(std::shared_ptr<Env> p = nullptr, bool isFunction_ = false);

    Val  get(const std::string& k) const;
    void define(const std::string& k, Val v);
    void assign(const std::string& k, Val v);
};

struct Function {
    std::vector<std::string> params;
    std::vector<const Stmt*> body;
    std::shared_ptr<Env> closure;

    Function(std::vector<std::string> p,
             std::vector<const Stmt*> b,
             std::shared_ptr<Env> c);
};

struct ReturnException : std::exception {
    Val value;
    explicit ReturnException(Val v);
    const char* what() const noexcept override;
};

struct BreakException : std::exception {
    const char* what() const noexcept override;
};

struct ContinueException : std::exception {
    const char* what() const noexcept override;
};

class VM {
    std::shared_ptr<Env> globals;
    std::ostream* out = nullptr;
    std::mt19937 rng{std::random_device{}()};

    Val ev(const Expr& e, std::shared_ptr<Env> env);
    void ex(const Stmt& s, std::shared_ptr<Env> env);
    std::shared_ptr<Env> findClosureEnv(std::shared_ptr<Env> env);

public:
    VM();
    bool run(const std::string& src, std::ostream& output);
};

bool interpret(std::istream& in, std::ostream& out);

} 
