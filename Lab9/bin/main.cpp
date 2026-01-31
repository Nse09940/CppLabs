#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <stdexcept>
#include "scheduler.h"

struct AddNumber {
    float add(float x) const { return x + k; }
    AddNumber(float n){
        k = n;
    }
    AddNumber(const AddNumber& other){
        k = other.k;
        std::cout << "copy";
    }
    float k;
};

int main() {
    float a = 1, b = -2, c = 0;
    AddNumber add(3);

    TTaskScheduler sch;

    auto id1 = sch.add([](float aa, float cc){ return -4*aa*cc; }, a, c);
    auto id2 = sch.add([](float bb, float v){ return bb*bb + v; }, b,
                       sch.getFutureResult<float>(id1));
    auto id3 = sch.add([](float bb, float d){ return -bb + std::sqrt(d); },
                       b, sch.getFutureResult<float>(id2));
    auto id4 = sch.add([](float bb, float d){ return -bb - std::sqrt(d); },
                       b, sch.getFutureResult<float>(id2));
    auto id5 = sch.add([](float aa, float v){ return v/(2*aa); },
                       a, sch.getFutureResult<float>(id3));
    auto id6 = sch.add([](float aa, float v){ return v/(2*aa); },
                       a, sch.getFutureResult<float>(id4));
    auto id7 = sch.add(&AddNumber::add, add, sch.getFutureResult<float>(id6));
    auto id8 = sch.add(&AddNumber::add, AddNumber(3), sch.getFutureResult<float>(id6));

    sch.executeAll();

    std::cout << "x1 = " << sch.getResult<float>(id5) << '\n'
              << "x2 = " << sch.getResult<float>(id6) << '\n'
              << "x3 = " << sch.getResult<float>(id7) << '\n';

    return 0;
}
