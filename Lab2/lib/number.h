#pragma once
#include <cinttypes>
#include <iostream>

#include <algorithm>

#include <cstring>
#include <stdexcept>



struct uint239_t {
    uint8_t data[35];
    
    uint239_t() {
        for (int i = 0; i < 35; ++i) {
            data[i] = 0;
        }
    }
};

template<typename T>
class vector {
private:
    static const int MAX_SIZE = 700;
    T data[MAX_SIZE];
    int real_size;

public:
    vector() : real_size(0) {}

    int size() const {
        return real_size;
    }

    void push_back(const T &value) {
        data[real_size++] = value;
    }

    void push_front(const T &value) {
        for (int i = real_size; i > 0; --i) {
            data[i] = data[i - 1];
        }
        data[0] = value;
        real_size++;
    }

    T &operator[](int index) {

        return data[index];
    }

    void clear() {
        real_size = 0;
    }

    void pop_back() {
        if (real_size > 0) {
            --real_size;
        }
    }

    void pop_front() {
        if (real_size > 0) {
            for (int i = 0; i < real_size - 1; ++i) {
                data[i] = data[i + 1];
            }
            --real_size;
        }
    }

    bool operator==(const vector &other) const {
        if (real_size != other.real_size) {
            return false;
        }
        for (size_t i = 0; i < real_size; ++i) {
            if (data[i] != other.data[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator>=(const vector &other) const {
        if (real_size > other.real_size) {
            return true;
        } else if (real_size < other.real_size) {
            return false;
        }

        for (size_t i = 0; i < real_size; ++i) {
            if (data[i] > other.data[i]) {
                return true;
            } else if (data[i] < other.data[i]) {
                return false;
            }
        }
        return true;
    }

    void erase(size_t index) {
        for (size_t i = index; i < real_size - 1; ++i) {
            data[i] = data[i + 1];
        }
        --real_size;
    }

    friend void my_reverse(vector<char> &str, int shift);

    friend uint239_t FromString(const char *str, uint32_t shift);

    friend std::ostream &operator<<(std::ostream &stream, const uint239_t &value);
};

static_assert(sizeof(uint239_t) == 35, "Size of uint239_t must be no higher than 35 bytes");

uint239_t build_new_239(vector<char> &str, long long shift);
vector<char> get_sum_str_for_mul(vector<char> a, vector<char> b);
vector<char> get_multi_str(vector<char> a, vector<char> b);
vector<char> get_sum_str(vector<char> a, vector<char> b);
vector<char> get_minus_str_for_div(vector<char> a, vector<char> b);
vector<char> division(vector<char> &dividend, vector<char> &divisor);
vector<char> get_minus_str(vector<char> a,vector<char> b);
void my_reverse(vector<char> &str, int shift);
vector<char> get_str_from_239(uint239_t a);
uint239_t operator+( const uint239_t &lhs,const   uint239_t &rhs);
uint239_t operator-(const uint239_t &aa, const uint239_t &bb);
uint239_t operator*(const uint239_t &aa, const uint239_t &bb);
uint239_t operator/(const uint239_t &aa, const uint239_t &bb);
bool operator==(const uint239_t &a, const uint239_t &b);
bool operator!=(const uint239_t &a, const uint239_t &b);
std::ostream &operator<<(std::ostream &stream, const uint239_t &value);
uint64_t GetShift(const uint239_t &value);
uint239_t operator+=( uint239_t &lhs, const uint239_t &rhs);
uint239_t operator-=( uint239_t& lhs, const uint239_t& rhs);
void my_reverse2(uint239_t &strr, int shift);
uint239_t SetShift(const uint239_t num, uint64_t shift);
uint239_t FromInt(uint32_t value, uint64_t shift);
uint239_t FromString(const char* str, uint64_t shift);
uint8_t len(const uint239_t& num);
uint239_t ClearShift(const uint239_t num);
bool operator<(const uint239_t& lhs, const uint239_t& rhs);
bool operator>(const uint239_t& lhs, const uint239_t& rhs);
void DoShift(uint239_t& a, int shift);
uint239_t plus( const uint239_t& lhs1,  const uint239_t& rhs1);