#pragma once

#include <cstddef>
#include <vector>
#include <tuple>
#include <utility>
#include <type_traits>
#include <cmath>
#include <iostream>
#include <functional>
#include <stdexcept>

class Any {
    struct Base { virtual ~Base() = default; virtual Base* clone() const = 0; };
    template<typename T>
    struct Derived : Base {
        T value;
        Derived(T&& v): value(std::move(v)) {}
        Derived(const T& v): value(v) {}
        Base* clone() const override { return new Derived<T>(value); }
    };

    Base* ptr_;
public:
    Any(): ptr_(nullptr) {}
    Any(const Any& other): ptr_(other.ptr_ ? other.ptr_->clone() : nullptr) {}
    Any(Any&& other) noexcept: ptr_(other.ptr_) { other.ptr_ = nullptr; }

    template<
      typename T,
      typename U = std::decay_t<T>,
      std::enable_if_t<!std::is_same_v<U, Any>, int> = 0
    >
    Any(T&& v): ptr_(new Derived<U>(std::forward<T>(v))) {}

    ~Any() { delete ptr_; }

    Any& operator=(const Any& other) {
        if (this != &other) {
            delete ptr_;
            ptr_ = other.ptr_ ? other.ptr_->clone() : nullptr;
        }
        return *this;
    }
    Any& operator=(Any&& other) noexcept {
        if (this != &other) {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    bool has_value() const { return ptr_ != nullptr; }

    template<typename T>
    friend T any_cast(const Any& any) {
        using D = Derived<T>;
        if (auto d = dynamic_cast<D*>(any.ptr_))
            return d->value;
    }
};


template<typename> class Function;
template<typename R, typename... Args>
class Function<R(Args...)> {
    struct Base {
        virtual ~Base() = default;
        virtual R invoke(Args&&...) = 0;
        virtual Base* clone() const = 0;
    };

    template<typename F>
    struct Holder : Base {
        F f;
        Holder(F&& fn): f(std::forward<F>(fn)) {}
        Holder(const F& fn): f(fn) {}
        R invoke(Args&&... args) override {
            return std::invoke(f, std::forward<Args>(args)...);
        }
        Base* clone() const override {
            return new Holder<F>(f);
        }
    };

    Base* ptr_;
public:
    Function(): ptr_(nullptr) {}
    Function(std::nullptr_t): ptr_(nullptr) {}

    template<
      typename F,
      std::enable_if_t<!std::is_same_v<std::decay_t<F>, Function>, int> = 0
    >
    Function(F&& f): ptr_(new Holder<std::decay_t<F>>(std::forward<F>(f))) {}

    Function(const Function& other)
      : ptr_(other.ptr_ ? other.ptr_->clone() : nullptr) {}

    Function(Function&& other) noexcept
      : ptr_(other.ptr_) { other.ptr_ = nullptr; }

    ~Function() { delete ptr_; }

    Function& operator=(const Function& other) {
        if (this != &other) {
            delete ptr_;
            ptr_ = other.ptr_ ? other.ptr_->clone() : nullptr;
        }
        return *this;
    }
    Function& operator=(Function&& other) noexcept {
        if (this != &other) {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    R operator()(Args... args) const {
        if (!ptr_) throw std::bad_function_call();
        return ptr_->invoke(std::forward<Args>(args)...);
    }

    explicit operator bool() const { return ptr_ != nullptr; }
};


class TTaskScheduler {
public:
    using TaskId = std::size_t;
    template<class T>
    struct FutureResult {
        TaskId id;
        using value_type = T;
    };

    template<class T>
    FutureResult<T> getFutureResult(TaskId id) const {
        return {id};
    }

    
    template<typename U> struct is_future_result : std::false_type {};
    template<typename T> struct is_future_result<FutureResult<T>> : std::true_type {};

    template<class F, class... Args>
    TaskId add(F&& f, Args&&... args) {
        TaskId id = tasks_.size();
        results_.emplace_back();
        done_.push_back(false);

        auto tup = std::make_tuple(std::forward<Args>(args)...);

        tasks_.emplace_back(
            [this, id, f = std::forward<F>(f), tup = std::move(tup)]() mutable {
                auto resolve = [this](auto&& x) -> decltype(auto) {
                    using A = std::decay_t<decltype(x)>;
                    if constexpr (is_future_result<A>::value) {
                        return this->getResult<typename A::value_type>(x.id);
                    } else {
                        return std::forward<decltype(x)>(x);
                    }
                };

                auto caller = [&](auto&&... vs) {
                    using R = std::invoke_result_t<F, decltype(resolve(vs))...>;
                    R r = std::invoke(f, resolve(vs)...);
                    results_[id] = std::move(r);
                    done_[id] = true;
                };
                std::apply(caller, tup);
            }
        );

        return id;
    }

    template<class T>
    T getResult(TaskId id) {
        if (!done_[id]) {
            tasks_[id]();
        }
        return any_cast<T>(results_[id]);
    }

    void executeAll() {
        for (TaskId i = 0; i < tasks_.size(); ++i) {
            if (!done_[i]) tasks_[i]();
        }
    }

private:
    std::vector< Function<void()> > tasks_;
    std::vector< Any >             results_;
    std::vector< bool >            done_;
};


struct AddNumber {
    float add(float x) const { return x + k; }
    float k{3};
};

int main() {
    float a = 1, b = -2, c = 0;
    AddNumber add;

    TTaskScheduler sch;

    auto id1 = sch.add([](float aa, float cc){ return -4*aa*cc; }, a, c);
    auto id2 = sch.add([](float bb, float v ){ return bb*bb + v;  }, b,
                       sch.getFutureResult<float>(id1));
    auto id3 = sch.add([](float bb, float d ){ return -bb + std::sqrt(d); },
                       b, sch.getFutureResult<float>(id2));
    auto id4 = sch.add([](float bb, float d ){ return -bb - std::sqrt(d); },
                       b, sch.getFutureResult<float>(id2));
    auto id5 = sch.add([](float aa, float v ){ return v/(2*aa); }, a,
                       sch.getFutureResult<float>(id3));
    auto id6 = sch.add([](float aa, float v ){ return v/(2*aa); }, a,
                       sch.getFutureResult<float>(id4));
    auto id7 = sch.add(&AddNumber::add, add, sch.getFutureResult<float>(id6));

    sch.executeAll();

    std::cout << "x1 = " << sch.getResult<float>(id5) << '\n'
              << "x2 = " << sch.getResult<float>(id6) << '\n'
              << "x3 = " << sch.getResult<float>(id7) << '\n';
    return 0;
}
