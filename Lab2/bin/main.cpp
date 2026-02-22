#include "../lib/number.h"
#include <iostream>

int main() {
   uint239_t a = FromInt(7,1);
    DoShift(a,1);
    std::cout << a;
    return 0;
}
