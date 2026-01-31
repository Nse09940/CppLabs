#include "interpreter.h"
#include <sstream>

namespace itmoscript {

Env::Env(std::shared_ptr<Env> p, bool isFunction_)
    : parent(std::move(p)), isFunction(isFunction_) {}

Val Env::get(const std::string& k) const {
    auto it = table.find(k);
    if (it != table.end()) {
        return it->second;
    }
    if (parent) {
        return parent->get(k);
    }
    throw std::runtime_error("Undefined variable: " + k);
}

void Env::define(const std::string& k, Val v) {
    table[k] = std::move(v);
}

void Env::assign(const std::string& k, Val v) {
    auto it = table.find(k);
    if (it != table.end()) {
        it->second = std::move(v);
        return;
    }
    if (isFunction) {
        for (auto p = parent; p; p = p->parent) {
            auto it2 = p->table.find(k);
            if (it2 != p->table.end()) {
                it2->second = std::move(v);
                return;
            }
            if (!p->isFunction) break;
        }
        table[k] = std::move(v);
        return;
    }
    if (parent) {
        parent->assign(k, std::move(v));
        return;
    }
    table[k] = std::move(v);
}

Function::Function(std::vector<std::string> p,
                   std::vector<const Stmt*> b,
                   std::shared_ptr<Env> c)
    : params(std::move(p)), body(std::move(b)), closure(std::move(c)) {}

ReturnException::ReturnException(Val v)
    : value(std::move(v)) {}
const char* ReturnException::what() const noexcept {
    return "return";
}

const char* BreakException::what() const noexcept {
    return "break";
}

const char* ContinueException::what() const noexcept {
    return "continue";
}

std::string doubleToString(double x) {
    if (std::isnan(x)) return "nan";
    if (std::isinf(x)) return (x > 0) ? "inf" : "-inf";
    double intpart;
    if (std::modf(x, &intpart) == 0.0) {
        return std::to_string(static_cast<long long>(intpart));
    }
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

std::string toString(const Val& val) {
    struct V {
        std::string operator()(std::monostate) const {
            return "nil";
        }
        std::string operator()(double x) const {
            return doubleToString(x);
        }
        std::string operator()(const std::string& s) const {
            return s;
        }
        std::string operator()(const ArrayPtr& p) const {
            std::string out = "[";
            for (size_t i = 0; i < p->size(); ++i) {
                out += toString((*p)[i]);
                if (i + 1 < p->size()) out += ", ";
            }
            out += "]";
            return out;
        }
        std::string operator()(const FuncPtr&) const {
            return "<fn>";
        }
    };
    return std::visit(V{}, val.v);
}

std::string display(const Val& val) {
    struct D {
        static bool needsQuotes(const std::string& s) {
            if (s.empty()) return false;
            bool inside = false;
            for (size_t i = 0; i < s.size(); ++i) {
                char c = s[i];
                if (c == '\n' || c == '\r') return false;
                if ((c == ' ' || c == '\t') && i + 1 < s.size()) inside = true;
            }
            return inside;
        }

        static std::string quote(const std::string& s) {
            std::string res = "\"";
            for (char c : s) {
                if (c == '"') res += "\\\"";
                else           res += c;
            }
            res += "\"";
            return res;
        }

        std::string operator()(std::monostate) const {
            return "nil";
        }
        std::string operator()(double x) const {
            return doubleToString(x);
        }
        std::string operator()(const std::string& s) const {
            return needsQuotes(s) ? quote(s) : s;
        }
        std::string operator()(const ArrayPtr& p) const {
            std::string out = "[";
            for (size_t i = 0; i < p->size(); ++i) {
                const Val& elem = (*p)[i];
                if (std::holds_alternative<std::string>(elem.v)) {
                    out += quote(std::get<std::string>(elem.v));
                } else {
                    out += display(elem);
                }
                if (i + 1 < p->size()) out += ", ";
            }
            out += "]";
            return out;
        }
        std::string operator()(const FuncPtr&) const {
            return "<fn>";
        }
    };
    return std::visit(D{}, val.v);
}

bool isTruthy(const Val& v) {
    if (std::holds_alternative<std::monostate>(v.v)) return false;
    if (auto pd = std::get_if<double>(&v.v)) {
        return (*pd) != 0.0;
    }
    if (auto ps = std::get_if<std::string>(&v.v)) {
        return !ps->empty();
    }
    return true;
}

VM::VM() : globals(nullptr), out(nullptr) {}

std::shared_ptr<Env> VM::findClosureEnv(std::shared_ptr<Env> env) {
    return env;
}

Val VM::ev(const Expr& e, std::shared_ptr<Env> env) {
    if (auto p = dynamic_cast<const NumExpr*>(&e)) {
        return Val(p->v);
    }
    if (auto p = dynamic_cast<const StrExpr*>(&e)) {
        return Val(p->s);
    }
    if (dynamic_cast<const NilExpr*>(&e)) {
        return Val{};
    }
    if (auto p = dynamic_cast<const VarExpr*>(&e)) {
        return env->get(p->n);
    }

    if (auto p = dynamic_cast<const UnaryExpr*>(&e)) {
        Val r = ev(*p->rhs, env);
        switch (p->op) {
            case TK::MINUS: {
                if (auto pd = std::get_if<double>(&r.v)) {
                    return Val(-(*pd));
                }
                throw std::runtime_error("Bad unary '-' operand");
            }
            case TK::NOT:
                return Val(isTruthy(r) ? 0.0 : 1.0);
            default:
                throw std::runtime_error("Bad unary operator");
        }
    }

    if (auto p = dynamic_cast<const IndexExpr*>(&e)) {
        Val A = ev(*p->arr, env);
        Val I = ev(*p->idx, env);
        if (!std::holds_alternative<ArrayPtr>(A.v) &&
            !std::holds_alternative<std::string>(A.v)) {
            throw std::runtime_error("Indexing requires array or string");
        }
        if (!std::holds_alternative<double>(I.v)) {
            throw std::runtime_error("Index must be number");
        }
        int idx = static_cast<int>(std::get<double>(I.v));

        if (std::holds_alternative<std::string>(A.v)) {
            const auto& s = std::get<std::string>(A.v);
            int len = static_cast<int>(s.size());
            if (idx < 0) idx = len + idx;
            if (idx < 0 || idx >= len) {
                throw std::runtime_error("String index out of bounds");
            }
            return Val(std::string(1, s[idx]));
        } else {
            const auto& vec = *std::get<ArrayPtr>(A.v);
            int len = static_cast<int>(vec.size());
            if (idx < 0) idx = len + idx;
            if (idx < 0 || idx >= len) {
                throw std::runtime_error("Array index out of bounds");
            }
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
                if (!std::holds_alternative<double>(S.v)) {
                    throw std::runtime_error("Slice start must be number");
                }
                start = static_cast<int>(std::get<double>(S.v));
                if (start < 0) start = len + start;
            }
            if (p->hasEnd) {
                Val E = ev(*p->end, env);
                if (!std::holds_alternative<double>(E.v)) {
                    throw std::runtime_error("Slice end must be number");
                }
                end = static_cast<int>(std::get<double>(E.v));
                if (end < 0) end = len + end;
            }
            int sidx = p->hasStart ? std::clamp(start, 0, len) : 0;
            int eidx = p->hasEnd   ? std::clamp(end,   0, len)   : len;
            if (eidx < sidx) eidx = sidx;
            return Val(s.substr(sidx, eidx - sidx));
        }
        else if (std::holds_alternative<ArrayPtr>(A.v)) {
            const auto& vec = *std::get<ArrayPtr>(A.v);
            int len = static_cast<int>(vec.size());

            if (p->hasStart) {
                Val S = ev(*p->start, env);
                if (!std::holds_alternative<double>(S.v)) {
                    throw std::runtime_error("Slice start must be number");
                }
                start = static_cast<int>(std::get<double>(S.v));
                if (start < 0) start = len + start;
            }
            if (p->hasEnd) {
                Val E = ev(*p->end, env);
                if (!std::holds_alternative<double>(E.v)) {
                    throw std::runtime_error("Slice end must be number");
                }
                end = static_cast<int>(std::get<double>(E.v));
                if (end < 0) end = len + end;
            }
            int sidx = p->hasStart ? std::clamp(start, 0, len) : 0;
            int eidx = p->hasEnd   ? std::clamp(end,   0, len)   : len;
            if (eidx < sidx) eidx = sidx;

            auto arr = std::make_shared<std::vector<Val>>();
            for (int k = sidx; k < eidx; ++k) {
                arr->push_back(vec[k]);
            }
            return Val(arr);
        }
        else {
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
                    (std::holds_alternative<double>(L.v) && std::holds_alternative<std::string>(R.v)))
                {
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
                    auto arr   = std::get<ArrayPtr>(L.v);
                    int  times = static_cast<int>(std::get<double>(R.v));
                    if (times < 0) times = 0;
                    auto out = std::make_shared<std::vector<Val>>();
                    for (int k = 0; k < times; ++k) {
                        out->insert(out->end(), arr->begin(), arr->end());
                    }
                    return Val(out);
                }
                if (numArr(L, R)) {
                    int  times = static_cast<int>(std::get<double>(L.v));
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
                    if (b == 0.0) {
                        throw std::runtime_error("Division by zero");
                    }
                    return Val(a / b);
                }
                throw std::runtime_error("Illegal '/' operands");

            case TK::MOD:
                if (bothNum(L, R)) {
                    double a = std::get<double>(L.v);
                    double b = std::get<double>(R.v);
                    if (b == 0.0) {
                        throw std::runtime_error("Modulo by zero");
                    }
                    return Val(std::fmod(a, b));
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
                return Val(p->op == TK::EQ ? (eq ? 1.0 : 0.0)
                                          : (!eq ? 1.0 : 0.0));
            }

            case TK::GT:
            case TK::LT:
            case TK::GTE:
            case TK::LTE:
                if (bothNum(L, R)) {
                    double a = std::get<double>(L.v);
                    double b = std::get<double>(R.v);
                    switch (p->op) {
                        case TK::GT:  return Val(a > b ? 1.0 : 0.0);
                        case TK::LT:  return Val(a < b ? 1.0 : 0.0);
                        case TK::GTE: return Val(a >= b ? 1.0 : 0.0);
                        case TK::LTE: return Val(a <= b ? 1.0 : 0.0);
                        default:      break;
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
                        default:      break;
                    }
                }
                throw std::runtime_error("Illegal comparison operands");

            case TK::AND:
                return Val(isTruthy(L) && isTruthy(R) ? 1.0 : 0.0);

            case TK::OR:
                return Val(isTruthy(L) || isTruthy(R) ? 1.0 : 0.0);

            default:
                throw std::runtime_error("Unknown binary operator");
        }
    }

    if (auto p = dynamic_cast<const ArrayExpr*>(&e)) {
        auto arr = std::make_shared<std::vector<Val>>();
        for (auto& el : p->el) {
            arr->push_back(ev(*el, env));
        }
        return Val(arr);
    }

    if (auto p = dynamic_cast<const LenExpr*>(&e)) {
        Val v = ev(*p->w, env);
        if (std::holds_alternative<std::string>(v.v)) {
            return Val(static_cast<double>(std::get<std::string>(v.v).size()));
        }
        if (std::holds_alternative<ArrayPtr>(v.v)) {
            return Val(static_cast<double>(std::get<ArrayPtr>(v.v)->size()));
        }
        throw std::runtime_error("len() expects string or array");
    }

    if (auto p = dynamic_cast<const FuncLit*>(&e)) {
        auto closureEnv = findClosureEnv(env);
        std::vector<const Stmt*> fn_body;
        for (auto& st : p->body) {
            fn_body.push_back(st.get());
        }
        auto fn = std::make_shared<Function>(p->params, fn_body, closureEnv);
        return Val(fn);
    }

    if (auto p = dynamic_cast<const CallExpr*>(&e)) {
        if (auto cv = dynamic_cast<const VarExpr*>(p->callee.get())) {
            const std::string& name = cv->n;

            if (name == "print") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("print() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                (*out) << display(v);
                return Val{};
            }
            if (name == "println") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("println() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                (*out) << display(v) << "\n";
                return Val{};
            }
            if (name == "range") {
                size_t cnt = p->args.size();
                if (cnt < 1 || cnt > 3) {
                    throw std::runtime_error("range() takes 1–3 arguments");
                }
                int start = 0, end = 0, step = 1;
                if (cnt == 1) {
                    end = static_cast<int>(std::get<double>(ev(*p->args[0], env).v));
                }
                else if (cnt == 2) {
                    start = static_cast<int>(std::get<double>(ev(*p->args[0], env).v));
                    end   = static_cast<int>(std::get<double>(ev(*p->args[1], env).v));
                }
                else {
                    start = static_cast<int>(std::get<double>(ev(*p->args[0], env).v));
                    end   = static_cast<int>(std::get<double>(ev(*p->args[1], env).v));
                    step  = static_cast<int>(std::get<double>(ev(*p->args[2], env).v));
                    if (step == 0) {
                        throw std::runtime_error("range() step cannot be zero");
                    }
                }
                auto arr = std::make_shared<std::vector<Val>>();
                if (step > 0) {
                    for (int k = start; k < end; k += step) {
                        arr->push_back(Val(static_cast<double>(k)));
                    }
                }
                else {
                    for (int k = start; k > end; k += step) {
                        arr->push_back(Val(static_cast<double>(k)));
                    }
                }
                return Val(arr);
            }
            if (name == "abs") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("abs() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) {
                    throw std::runtime_error("abs() expects number");
                }
                return Val(std::fabs(std::get<double>(v.v)));
            }
            if (name == "ceil") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("ceil() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) {
                    throw std::runtime_error("ceil() expects number");
                }
                return Val(std::ceil(std::get<double>(v.v)));
            }
            if (name == "floor") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("floor() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) {
                    throw std::runtime_error("floor() expects number");
                }
                return Val(std::floor(std::get<double>(v.v)));
            }
            if (name == "round") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("round() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) {
                    throw std::runtime_error("round() expects number");
                }
                return Val(std::round(std::get<double>(v.v)));
            }
            if (name == "sqrt") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("sqrt() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) {
                    throw std::runtime_error("sqrt() expects number");
                }
                return Val(std::sqrt(std::get<double>(v.v)));
            }
            if (name == "rnd") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("rnd() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) {
                    throw std::runtime_error("rnd() expects number");
                }
                int n = static_cast<int>(std::get<double>(v.v));
                if (n <= 0) {
                    return Val(0.0);
                }
                std::uniform_int_distribution<int> dist(0, n - 1);
                return Val(static_cast<double>(dist(rng)));
            }
            if (name == "parse_num") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("parse_num() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<std::string>(v.v)) {
                    throw std::runtime_error("parse_num() expects string");
                }
                const auto& s = std::get<std::string>(v.v);
                try {
                    double x = std::stod(s);
                    return Val(x);
                } catch (...) {
                    return Val{};
                }
            }
            if (name == "to_string") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("to_string() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<double>(v.v)) {
                    throw std::runtime_error("to_string() expects number");
                }
                return Val(doubleToString(std::get<double>(v.v)));
            }
            if (name == "lower") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("lower() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<std::string>(v.v)) {
                    throw std::runtime_error("lower() expects string");
                }
                std::string s = std::get<std::string>(v.v);
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                return Val(s);
            }
            if (name == "upper") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("upper() takes 1 argument");
                }
                Val v = ev(*p->args[0], env);
                if (!std::holds_alternative<std::string>(v.v)) {
                    throw std::runtime_error("upper() expects string");
                }
                std::string s = std::get<std::string>(v.v);
                std::transform(s.begin(), s.end(), s.begin(), ::toupper);
                return Val(s);
            }
            if (name == "split") {
                if (p->args.size() != 2) {
                    throw std::runtime_error("split() takes 2 arguments");
                }
                Val vs = ev(*p->args[0], env);
                Val vd = ev(*p->args[1], env);
                if (!std::holds_alternative<std::string>(vs.v) ||
                    !std::holds_alternative<std::string>(vd.v))
                {
                    throw std::runtime_error("split() expects (string, string)");
                }
                const std::string& s     = std::get<std::string>(vs.v);
                const std::string& delim = std::get<std::string>(vd.v);

                auto arr = std::make_shared<std::vector<Val>>();
                if (delim.empty()) {
                    for (char c : s) {
                        arr->push_back(Val(std::string(1, c)));
                    }
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
            if (name == "join") {
                if (p->args.size() != 2) {
                    throw std::runtime_error("join() takes 2 arguments");
                }
                Val va = ev(*p->args[0], env);
                Val vd = ev(*p->args[1], env);
                if (!std::holds_alternative<ArrayPtr>(va.v) ||
                    !std::holds_alternative<std::string>(vd.v))
                {
                    throw std::runtime_error("join() expects (array, string)");
                }
                const auto& arr   = *std::get<ArrayPtr>(va.v);
                const std::string& delim = std::get<std::string>(vd.v);
                std::string out;
                for (size_t i = 0; i < arr.size(); ++i) {
                    if (!std::holds_alternative<std::string>(arr[i].v)) {
                        throw std::runtime_error("join() array elements must be strings");
                    }
                    out += std::get<std::string>(arr[i].v);
                    if (i + 1 < arr.size()) out += delim;
                }
                return Val(out);
            }
            if (name == "replace") {
                if (p->args.size() != 3) {
                    throw std::runtime_error("replace() takes 3 arguments");
                }
                Val vs = ev(*p->args[0], env);
                Val vo = ev(*p->args[1], env);
                Val vn = ev(*p->args[2], env);
                if (!std::holds_alternative<std::string>(vs.v) ||
                    !std::holds_alternative<std::string>(vo.v) ||
                    !std::holds_alternative<std::string>(vn.v))
                {
                    throw std::runtime_error("replace() expects (string, string, string)");
                }
                std::string s    = std::get<std::string>(vs.v);
                std::string oldp = std::get<std::string>(vo.v);
                std::string newp = std::get<std::string>(vn.v);
                if (oldp.empty()) {
                    return Val(s);
                }
                size_t pos = 0;
                while ((pos = s.find(oldp, pos)) != std::string::npos) {
                    s.replace(pos, oldp.size(), newp);
                    pos += newp.size();
                }
                return Val(s);
            }
            if (name == "push") {
                if (p->args.size() != 2) {
                    throw std::runtime_error("push() takes 2 arguments");
                }
                Val va = ev(*p->args[0], env);
                Val vx = ev(*p->args[1], env);
                if (!std::holds_alternative<ArrayPtr>(va.v)) {
                    throw std::runtime_error("push() expects array");
                }
                auto arr = std::get<ArrayPtr>(va.v);
                arr->push_back(vx);
                return Val{};
            }
            if (name == "pop") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("pop() takes 1 argument");
                }
                Val va = ev(*p->args[0], env);
                if (!std::holds_alternative<ArrayPtr>(va.v)) {
                    throw std::runtime_error("pop() expects array");
                }
                auto arr = std::get<ArrayPtr>(va.v);
                if (arr->empty()) {
                    return Val{};
                }
                Val ret = arr->back();
                arr->pop_back();
                return ret;
            }
            if (name == "insert") {
                if (p->args.size() != 3) {
                    throw std::runtime_error("insert() takes 3 arguments");
                }
                Val va = ev(*p->args[0], env);
                Val vi = ev(*p->args[1], env);
                Val vx = ev(*p->args[2], env);
                if (!std::holds_alternative<ArrayPtr>(va.v) ||
                    !std::holds_alternative<double>(vi.v))
                {
                    throw std::runtime_error("insert() expects (array, number, value)");
                }
                auto arr = std::get<ArrayPtr>(va.v);
                int idx = static_cast<int>(std::get<double>(vi.v));
                if (idx < 0) idx = 0;
                if (idx > static_cast<int>(arr->size())) {
                    idx = static_cast<int>(arr->size());
                }
                arr->insert(arr->begin() + idx, vx);
                return Val{};
            }
            if (name == "remove") {
                if (p->args.size() != 2) {
                    throw std::runtime_error("remove() takes 2 arguments");
                }
                Val va = ev(*p->args[0], env);
                Val vi = ev(*p->args[1], env);
                if (!std::holds_alternative<ArrayPtr>(va.v) ||
                    !std::holds_alternative<double>(vi.v))
                {
                    throw std::runtime_error("remove() expects (array, number)");
                }
                auto arr = std::get<ArrayPtr>(va.v);
                int idx = static_cast<int>(std::get<double>(vi.v));
                int len = static_cast<int>(arr->size());
                if (idx < 0) {
                    idx = len + idx;
                }
                if (idx < 0 || idx >= len) {
                    throw std::runtime_error("remove() index out of bounds");
                }
                arr->erase(arr->begin() + idx);
                return Val{};
            }
            if (name == "sort") {
                if (p->args.size() != 1) {
                    throw std::runtime_error("sort() takes 1 argument");
                }
                Val va = ev(*p->args[0], env);
                if (!std::holds_alternative<ArrayPtr>(va.v)) {
                    throw std::runtime_error("sort() expects array");
                }
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
            if (name == "read") {
                throw std::runtime_error("read() not implemented");
            }
            if (name == "stacktrace") {
                throw std::runtime_error("stacktrace() not implemented");
            }
        }

        Val cal = ev(*p->callee, env);
        if (!std::holds_alternative<FuncPtr>(cal.v)) {
            throw std::runtime_error("Call on non-function");
        }
        auto fn = std::get<FuncPtr>(cal.v);
        if (fn->params.size() != p->args.size()) {
            throw std::runtime_error("Argument count mismatch");
        }

        auto frame = std::make_shared<Env>(fn->closure, true);
        for (size_t i = 0; i < fn->params.size(); ++i) {
            frame->define(fn->params[i], ev(*p->args[i], env));
        }

        try {
            for (const Stmt* st : fn->body) {
                ex(*st, frame);
            }
        } catch (ReturnException& r) {
            return r.value;
        }
        return Val{};
    }

    throw std::runtime_error("Unknown expression type");
}

void VM::ex(const Stmt& s, std::shared_ptr<Env> env) {
    if (auto p = dynamic_cast<const ExprStmt*>(&s)) {
        ev(*p->e, env);
        return;
    }

    if (auto p = dynamic_cast<const AssignStmt*>(&s)) {
        env->assign(p->n, ev(*p->v, env));
        return;
    }

    if (auto p = dynamic_cast<const IndexAssignStmt*>(&s)) {
        Val arrVal = env->get(p->arr);
        if (!std::holds_alternative<ArrayPtr>(arrVal.v)) {
            throw std::runtime_error("Index assignment requires array on the left");
        }
        auto arr = std::get<ArrayPtr>(arrVal.v);
        Val idxVal = ev(*p->idx, env);
        if (!std::holds_alternative<double>(idxVal.v)) {
            throw std::runtime_error("Index must be number");
        }
        int idx = static_cast<int>(std::get<double>(idxVal.v));
        if (idx < 0) {
            throw std::runtime_error("Negative index not supported");
        }
        if (idx >= static_cast<int>(arr->size())) {
            arr->resize(idx + 1);
        }
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
            for (auto& st : p->thenBody) {
                ex(*st, env);
            }
        } else {
            for (auto& st : p->elseBody) {
                ex(*st, env);
            }
        }
        return;
    }

    if (auto p = dynamic_cast<const ForStmt*>(&s)) {
        Val v = ev(*p->iterable, env);
        if (!std::holds_alternative<ArrayPtr>(v.v)) {
            throw std::runtime_error("for expects array");
        }
        for (const Val& elem : *std::get<ArrayPtr>(v.v)) {
            auto loopEnv = std::make_shared<Env>(env, false);
            loopEnv->define(p->var, elem);
            try {
                for (auto& st : p->body) {
                    ex(*st, loopEnv);
                }
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
                for (auto& st : p->body) {
                    ex(*st, loopEnv);
                }
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
        for (auto& st : p->body) {
            fn_body.push_back(st.get());
        }
        auto fn = std::make_shared<Function>(p->params, fn_body, closureEnv);
        env->define(p->name, Val(fn));
        return;
    }

    throw std::runtime_error("Unknown statement type");
}

bool VM::run(const std::string& src, std::ostream& output) {
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
        for (auto& st : prog->stmts) {
            ex(*st, globals);
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool interpret(std::istream& in, std::ostream& out) {
    std::string src((std::istreambuf_iterator<char>(in)), {});
    VM vm;
    return vm.run(src, out);
}

} 
