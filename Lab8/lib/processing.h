
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <functional>
#include <format>
#include <expected>

inline std::string to_string_from(const std::stringstream& ss) {
    return ss.str();
}

inline std::string to_string_from(const std::ostringstream& oss) {
    return oss.str();
}
template<typename T>
std::string to_string_from(const T& t) {
    if  (std::is_convertible_v<T, std::string>)
        return t;
    else
        return std::string{};
}

template<typename Container, typename Adapter>
auto operator|(Container&& container, Adapter&& adapter)
    -> decltype(adapter(std::forward<Container>(container)))
{
    return adapter(std::forward<Container>(container));
}

template <typename Cont>
auto AsDataFlow(Cont&& cont) {
    return std::forward<Cont>(cont);
}

struct Dir {
    std::string directory;
    int dummy;

    Dir(const std::string& dir, int d)
        : directory(dir), dummy(d) {}

    class iterator {
    public:
        using RDIt = std::filesystem::recursive_directory_iterator;

        iterator() = default;
        explicit iterator(const RDIt& it_)
            : it(it_)
        {
            if (it != RDIt())
                current = it->path();
        }

        std::filesystem::path& operator*() { return current; }
        const std::filesystem::path& operator*() const { return current; }

        iterator& operator++() {
            ++it;
            if (it != RDIt())
                current = it->path();
            return *this;
        }

        bool operator==(const iterator& other) const { return it == other.it; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    private:
        RDIt it;
        std::filesystem::path current;
    };

    iterator begin() const {
        return iterator(std::filesystem::recursive_directory_iterator(directory));
    }
    iterator end() const {
        return iterator(std::filesystem::recursive_directory_iterator());
    }
};

template<typename Cont, typename Func>
class filter_view {
public:
    filter_view(Cont c, Func f)
        : cont(std::move(c)), func(std::move(f)) {}

    class iterator {
    public:
        using UnderIter = decltype(std::begin(std::declval<const Cont&>()));

        iterator(UnderIter cur, UnderIter end, const Func& fn)
            : current(cur), endIt(end), pred(fn)
        {
            while (current != endIt && !pred(*current))
                ++current;
        }

        decltype(auto) operator*() const { return *current; }
        iterator& operator++() {
            do { ++current; } while (current != endIt && !pred(*current));
            return *this;
        }
        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    private:
        UnderIter current, endIt;
        const Func& pred;
    };

    auto begin() const { return iterator(std::begin(cont), std::end(cont), func); }
    auto end() const { return iterator(std::end(cont), std::end(cont), func); }
private:
    Cont cont;
    Func func;
};

template<typename Func>
struct filter_adapter {
    explicit filter_adapter(Func f) : func(std::move(f)) {}
    template<typename Cont>
    auto operator()(Cont&& c) const {
        return filter_view<std::decay_t<Cont>, Func>(std::forward<Cont>(c), func);
    }
    Func func;
};

template<typename Func>
auto Filter(Func f) {
    return filter_adapter<Func>(std::move(f));
}

template<typename Cont, typename Func>
class transform_view {
public:
    transform_view(const Cont& c, const Func& f)
        : contRef(c), func(f) {}

    class iterator {
    public:
        using UnderIter = decltype(std::begin(std::declval<const Cont&>()));
        iterator(UnderIter it, const Func& fn)
            : current(it), action(fn) {}
        decltype(auto) operator*() const { return action(*current); }
        iterator& operator++() { ++current; return *this; }
        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    private:
        UnderIter current;
        const Func& action;
    };

    auto begin() const { return iterator(std::begin(contRef), func); }
    auto end() const { return iterator(std::end(contRef), func); }
private:
    const Cont& contRef;
    const Func& func;
};

template<typename Func>
struct transform_adapter {
    explicit transform_adapter(const Func& f) : func(f) {}
    template<typename Cont>
    auto operator()(Cont&& c) const {
        return transform_view<std::decay_t<Cont>, Func>(c, func);
    }
    const Func& func;
};

template<typename Func>
auto Transform(Func f) {
    return transform_adapter<Func>(f);
}

struct OpenFiles {
    template<typename Cont>
    auto operator()(Cont&& c) const {
        return Transform([](const std::filesystem::path& p) {
            std::ifstream fin(p);
            std::ostringstream oss;
            oss << fin.rdbuf();
            return oss.str();
        })(std::forward<Cont>(c));
    }
};

template<typename Cont>
class split_view {
public:
    using UnderIter = decltype(std::begin(std::declval<const Cont&>()));

    split_view(const Cont& c, std::string delims)
        : contRef(c), delimiters(std::move(delims))
    {}

    class iterator {
    public:
        using value_type = std::string;
        using UnderIter = decltype(std::begin(std::declval<const Cont&>()));

        iterator(UnderIter cur, UnderIter end, const std::string& d)
            : current(cur), endIt(end), delimiters(d), pos(0)
        {
            if (current != endIt)
                currentStr = to_string_from(*current);
            advancePastEmpty();
        }

        std::string operator*() const {
            auto nextDelim = currentStr.find_first_of(delimiters, pos);
            if (nextDelim == std::string::npos)
                return currentStr.substr(pos);
            else
                return currentStr.substr(pos, nextDelim - pos);
        }

        iterator& operator++() {
            auto nextDelim = currentStr.find_first_of(delimiters, pos);
            if (nextDelim != std::string::npos)
                pos = nextDelim + 1;
            else
                pos = currentStr.size();

            while (current != endIt && pos >= currentStr.size()) {
                ++current;
                if (current != endIt)
                    currentStr = to_string_from(*current);
                pos = 0;
            }
            advancePastEmpty();
            return *this;
        }

        bool operator==(const iterator& other) const {
            return current == other.current && (current == endIt || pos == other.pos);
        }
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    private:
        void advancePastEmpty() {
            while (current != endIt && (*this).operator*().empty()) {
                ++(*this);
            }
        }
        UnderIter current, endIt;
        std::string delimiters;
        std::string currentStr;
        size_t pos;
    };

    auto begin() const { return iterator(std::begin(contRef), std::end(contRef), delimiters); }
    auto end() const { return iterator(std::end(contRef), std::end(contRef), delimiters); }
private:
    const Cont& contRef;
    std::string delimiters;
};

struct Split {
    std::string delimiters;
    explicit Split(std::string d) : delimiters(std::move(d)) {}
    template<typename Cont>
    auto operator()(Cont&& c) const {
        return split_view<Cont>(c, delimiters);
    }
};

template<typename Cont>
class drop_nullopt_view {
public:
    drop_nullopt_view(const Cont& c) : contRef(c) {}
    class iterator {
    public:
        using UnderIter = decltype(std::begin(std::declval<const Cont&>()));
        iterator(UnderIter i, UnderIter e)
            : current(i), endIt(e)
        {
            while (current != endIt && !current->has_value())
                ++current;
        }
        decltype(auto) operator*() const { return *current; }
        iterator& operator++() {
            ++current;
            while (current != endIt && !current->has_value())
                ++current;
            return *this;
        }
        bool operator==(const iterator& o) const { return current == o.current; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
    private:
        UnderIter current, endIt;
    };
    auto begin() const { return iterator(std::begin(contRef), std::end(contRef)); }
    auto end() const { return iterator(std::end(contRef), std::end(contRef)); }
private:
    const Cont& contRef;
};

struct DropNullopt {
    template<typename T>
    DropNullopt(const T&) { }
    template<typename Cont>
    auto operator()(Cont&& c) const {
        return drop_nullopt_view<std::decay_t<Cont>>(c);
    }
};


struct Write {
    std::ostream& os;
    std::string sep;
    Write(std::ostream& s, const std::string& separator)
        : os(s), sep(separator) { }
    Write(std::ostream& s, char c)
        : os(s), sep(1, c) { }
    template<typename Cont>
    void operator()(const Cont& c) const {
        for (auto&& item : c)
            os << item << sep;
    }
};

template<typename T>
concept OptionalType = requires(const T& t) {
    { t.has_value() } -> std::convertible_to<bool>;
};

template<typename T>
concept PairType = requires(const T& t) {
    t.first; t.second;
};

template<typename T>
concept ContainerType = requires(const T& t) {
    { t.begin() } -> std::input_iterator;
    { t.end() } -> std::input_iterator;
};

template<typename T>
concept HasNameMember = requires(const T& t) {
    { t.name } -> std::convertible_to<std::string>;
};

struct out_adapter {
    std::ostream& os;
    out_adapter(std::ostream& s) : os(s) { }
    template<OptionalType T>
    void print_item(const T& val) const {
        if (val.has_value())
            os << *val;
        else
            os << "nullopt";
        os << "\n";
    }
    template<PairType T>
        requires (!OptionalType<T> )
        void print_item(const T& val) const {
            if  (ContainerType<typename T::second_type>) {
                for (auto&& x : val.second) {
                    if  (HasNameMember<decltype(x)>)
                        os << x.name << " ";
                    else
                        os << x << " ";
                }
            } else {
                os << val.second;
            }
            os << "\n";
    }
    template<typename U>
        requires (!OptionalType<U> && !PairType<U>)
    void print_item(const U& val) const {
        os << val << "\n";
    }
    template<typename Cont>
    int operator()(const Cont& c) const {
        for (auto&& x : c)
            print_item(x);
        return 0;
    }
};

inline auto Out(std::ostream& s) {
    return out_adapter(s);
}

struct AsVector {
    template<typename Cont>
    auto operator()(Cont&& cont) const {
        using ValueT = std::decay_t<decltype(*std::begin(cont))>;
        std::vector<ValueT> vec;
        for (auto&& x : cont)
            vec.push_back(x);
        return vec;
    }
};

template<typename Key, typename Value>
struct KV {
    Key key;
    Value value;
    bool operator==(const KV& other) const {
        return key == other.key && value == other.value;
    }
};

template<typename L, typename R>
struct JoinResult {
    L left;
    std::optional<R> right;
    bool operator==(const JoinResult& other) const {
        return left == other.left && right == other.right;
    }
};

template<typename LeftCont, typename RightCont>
class join_view_kv {
public:
    using LKV = typename LeftCont::value_type;
    using RKV = typename RightCont::value_type;
    using LeftValType = decltype(std::declval<LKV>().value);
    using RightValType = decltype(std::declval<RKV>().value);
    using result_type = JoinResult<LeftValType, RightValType>;

    join_view_kv(const LeftCont& l, const RightCont& r)
        : left(l), right(r)
    {
        for (auto&& lk : left) {
            auto myKey = lk.key;
            bool found = false;
            for (auto&& rk : right) {
                if (rk.key == myKey) {
                    results.push_back({ lk.value, rk.value });
                    found = true;
                }
            }
            if (!found) {
                results.push_back({ lk.value, std::nullopt });
            }
        }
    }
    auto begin() const { return results.begin(); }
    auto end() const { return results.end(); }
private:
    const LeftCont& left;
    const RightCont& right;
    std::vector<result_type> results;
};

template<typename RightCont>
struct join_adapter_kv {
    RightCont& r;
    join_adapter_kv(RightCont& rc) : r(rc) { }
    template<typename LeftCont>
    auto operator()(LeftCont&& left) const {
        return join_view_kv<std::decay_t<LeftCont>, RightCont>(left, r);
    }
};

template<typename RightCont>
auto Join(RightCont& right) {
    return join_adapter_kv<RightCont>(right);
}

template<typename LeftCont, typename RightCont, typename LeftKeyFunc, typename RightKeyFunc>
class join_view {
public:
    using LeftItem  = typename LeftCont::value_type;
    using RightItem = typename RightCont::value_type;
    using result_type = JoinResult<LeftItem, RightItem>;
    join_view(const LeftCont& l, const RightCont& r, LeftKeyFunc lf, RightKeyFunc rf)
        : leftRef(l), rightRef(r), leftKey(lf), rightKey(rf)
    {
        for (auto&& li : leftRef) {
            auto lkey = leftKey(li);
            bool found = false;
            for (auto&& ri : rightRef) {
                if (rightKey(ri) == lkey) {
                    results.push_back({ li, ri });
                    found = true;
                }
            }
            if (!found) {
                results.push_back({ li, std::nullopt });
            }
        }
    }
    auto begin() const { return results.begin(); }
    auto end() const { return results.end(); }
private:
    const LeftCont& leftRef;
    const RightCont& rightRef;
    LeftKeyFunc leftKey;
    RightKeyFunc rightKey;
    std::vector<result_type> results;
};

template<typename RightCont, typename LeftKeyFunc, typename RightKeyFunc>
struct join_adapter {
    RightCont& r;
    LeftKeyFunc lf;
    RightKeyFunc rf;
    join_adapter(RightCont& rc, LeftKeyFunc leftF, RightKeyFunc rightF)
        : r(rc), lf(leftF), rf(rightF) {}
    template<typename LeftCont>
    auto operator()(LeftCont&& left) const {
        return join_view<std::decay_t<LeftCont>, RightCont, LeftKeyFunc, RightKeyFunc>(
            left, r, lf, rf
        );
    }
};

template<typename RightCont, typename LeftKeyFunc, typename RightKeyFunc>
auto Join(RightCont& right, LeftKeyFunc lf, RightKeyFunc rf) {
    return join_adapter<RightCont, LeftKeyFunc, RightKeyFunc>(right, lf, rf);
}

template<typename Cont>
class expected_flow {
public:
    explicit expected_flow(const Cont& c) : cont(c) { }
    class iterator {
    public:
        using UnderIter = decltype(std::begin(std::declval<const Cont&>()));
        iterator(UnderIter i) : current(i) { }
        const auto& operator*() const { return *current; }
        iterator& operator++() { ++current; return *this; }
        bool operator==(const iterator& o) const { return current == o.current; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
    private:
        UnderIter current;
    };
    auto begin() const { return iterator(std::begin(cont)); }
    auto end() const { return iterator(std::end(cont)); }
private:
    const Cont& cont;
};

struct SplitExpected {
    template<typename T>
    SplitExpected(T&&) { }
    template<typename Cont>
    auto operator()(Cont&& c) const {
        using Flow = expected_flow<std::decay_t<Cont>>;
        Flow fullFlow(c);
        auto errorFlow = Transform([](const auto& exp) {
            return exp.error();
        })(
            Filter([](const auto& exp) {
                return !exp.has_value();
            })(fullFlow)
        );
        auto goodFlow = Transform([](const auto& exp) {
            return *exp;
        })(
            Filter([](const auto& exp) {
                return exp.has_value();
            })(fullFlow)
        );
        return std::make_pair(errorFlow, goodFlow);
    }
};

template<typename Cont, typename AggType, typename AggFunc, typename KeyFunc>
class aggregate_by_key_view {
public:
    using Element = std::remove_reference_t<decltype(*std::begin(std::declval<Cont&>()))>;
    using KeyType = decltype(std::declval<KeyFunc>()(std::declval<const Element&>()));
    using pair_type = std::pair<KeyType, AggType>;
    aggregate_by_key_view(const Cont& c, AggType init, AggFunc agg, KeyFunc kf)
        : container(c), initial(init), aggregator(agg), key_ex(kf)
    {
        for (auto&& el : container) {
            KeyType k = key_ex(el);
            bool found = false;
            for (auto& p : result) {
                if (p.first == k) {
                    aggregator(el, p.second);
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.push_back({ k, initial });
                aggregator(el, result.back().second);
            }
        }
    }
    class iterator {
    public:
        using VecIt = typename std::vector<pair_type>::const_iterator;
        iterator(VecIt it) : current(it) { }
        const pair_type& operator*() const { return *current; }
        iterator& operator++() { ++current; return *this; }
        bool operator==(const iterator& o) const { return current == o.current; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
    private:
        VecIt current;
    };
    auto begin() const { return iterator(result.begin()); }
    auto end() const { return iterator(result.end()); }
private:
    const Cont& container;
    AggType initial;
    AggFunc aggregator;
    KeyFunc key_ex;
    std::vector<pair_type> result;
};

template<typename AggType, typename AggFunc, typename KeyFunc>
struct aggregate_by_key_adapter {
    AggType initVal;
    AggFunc aggregator;
    KeyFunc keyExtract;
    aggregate_by_key_adapter(AggType i, AggFunc agg, KeyFunc kf)
        : initVal(i), aggregator(agg), keyExtract(kf) {}
    template<typename Cont>
    auto operator()(Cont&& c) const {
        return aggregate_by_key_view<std::decay_t<Cont>, AggType, AggFunc, KeyFunc>(
            c, initVal, aggregator, keyExtract
        );
    }
};

template<typename AggType, typename AggFunc, typename KeyFunc>
auto AggregateByKey(AggType init, AggFunc agg, KeyFunc kf) {
    return aggregate_by_key_adapter<AggType, AggFunc, KeyFunc>(init, agg, kf);
}

