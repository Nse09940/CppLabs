#include <iostream>

#include "interpreter.h"

int main() {
    return itmoscript::interpret(std::cin, std::cout) ? 0 : 1;
}