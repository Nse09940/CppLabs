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
#include <memory>
class Any {
    struct Base { 
        virtual ~Base() = default; 
        virtual Base* clone() const = 0; 
    };
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
    friend T any_cast(const Any&);
    
    
};
template<typename T>
    T any_cast(const Any& any) {
    using D = Any::Derived<T>;
    if (auto d = dynamic_cast<D*>(any.ptr_))
        return d->value;
}

class TTaskScheduler {
public:
    using TaskId = std::size_t;

    template<class T>
    struct FutureResult { TaskId id; };

    template<class T>
    FutureResult<T> getFutureResult(TaskId id) const { return {id}; }

    template<class Callable, class... Args>
    TaskId add(Callable&& f, Args&&... args) {
        TaskId id = tasks_.size();
        tasks_.push_back(
            std::shared_ptr<ITask>(
                new Task<Callable, Args...>(
                    id,
                    std::forward<Callable>(f),
                    std::forward<Args>(args)...
                )
            )
        );
        results_.push_back(Any());
        done_.push_back(false);
        return id;
    }

    template<class T>
    T getResult(TaskId id) {
        run(id);
        return any_cast<T>(results_[id]);
    }

    void executeAll() {
        for (TaskId i = 0; i < tasks_.size(); ++i)
            run(i);
    }

private:
    struct ITask {
        virtual ~ITask() = default;
        virtual void exec(TTaskScheduler&) = 0;
    };

    template<class U = char, class... Us>
    struct First { using type = U; };

    template<class U = char, class V = char, class... Us>
    struct Second { using type = V; };

    template<class A>
    static auto resolve(A&& a, TTaskScheduler&) {
        return std::forward<A>(a);
    }

    template<class T>
    static auto resolve(FutureResult<T> f, TTaskScheduler& sch) {
        return sch.getResult<T>(f.id);
    }

    template<class F> static auto call(F& f) { return f(); }
    template<class F, class A1>
    static auto call(F& f, A1&& a1) {
        if constexpr (std::is_member_function_pointer_v<F>)
            return (std::forward<A1>(a1).*f)();
        else
            return f(std::forward<A1>(a1));
    }
    template<class F, class A1, class A2>
    static auto call(F& f, A1&& a1, A2&& a2) {
        if constexpr (std::is_member_function_pointer_v<F>)
            return (std::forward<A1>(a1).*f)(std::forward<A2>(a2));
        else
            return f(std::forward<A1>(a1), std::forward<A2>(a2));
    }

    template<class Callable, class... Args>
    class Task : public ITask {
        using A0 = std::conditional_t<sizeof...(Args)>=1, typename First<Args...>::type, char>;
        using A1 = std::conditional_t<sizeof...(Args)>=2, typename Second<Args...>::type, char>;

        TaskId id_;
        Callable func_;
        A0 a0_;
        A1 a1_;

    public:
        Task(TaskId id, Callable&& f, Args&&... args)
            : id_(id)
            , func_(std::forward<Callable>(f))
            , a0_(pick<0>(std::forward<Args>(args)...))
            , a1_(pick<1>(std::forward<Args>(args)...))
        {}

        void exec(TTaskScheduler& sch) override {
            if constexpr (sizeof...(Args) == 0) {
                auto r = call(func_);
                sch.store(id_, std::move(r));
            } else if constexpr (sizeof...(Args) == 1) {
                auto r = call(func_, resolve(a0_, sch));
                sch.store(id_, std::move(r));
            } else {
                auto r = call(func_,
                              resolve(a0_, sch),
                              resolve(a1_, sch));
                sch.store(id_, std::move(r));
            }
        }

    private:
        template<std::size_t N, class F, class... R>
        static decltype(auto) pick(F&& f, R&&... r) {
            if constexpr (N == 0) return std::forward<F>(f);
            else return pick<N-1>(std::forward<R>(r)...);
        }
        template<std::size_t> static char pick() { return {}; }
    };

    template<class V>
    void store(TaskId id, V&& v) {
        results_[id] = Any(std::forward<V>(v));
        done_[id] = true;
    }

    void run(TaskId id) {
        if (!done_[id]) {
            auto taskPtr = any_cast<std::shared_ptr<ITask>>(tasks_[id]);
            taskPtr->exec(*this);
        }
    }

    std::vector<Any> tasks_;
    std::vector<Any> results_;
    std::vector<bool> done_;
};



