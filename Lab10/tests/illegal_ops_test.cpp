#include <string>
#include <vector>
#include <sstream>

#include <lib/interpreter.h>
#include <gtest/gtest.h>
inline bool interpret(std::istream& in, std::ostream& out) {
    return itmoscript::interpret(in, out);
}

std::string kUnreachable = "239";


TEST(IllegalOperationsSuite, TypeMixing) {
    std::vector<std::string> values = {
        "123",
        "\"string\"",
        "[1, 2, 3]",
        "function() end function",
        "nil",
    };

    for (int a = 0; a < values.size(); ++a) {
        for (int b = a + 1; b < values.size(); ++b) {
            std::ostringstream input_buffer;
            input_buffer << "a = " << values[a] << "\n";
            input_buffer << "b = " << values[b] << "\n";
            input_buffer << "c = a + b" << "\n";
            input_buffer << "print(239)" << "\n";
            std::istringstream input(input_buffer.str());

            std::ostringstream output;

            ASSERT_FALSE(interpret(input, output));
            ASSERT_FALSE(output.str().ends_with(kUnreachable));
        }
    }
}


TEST(IllegalOperationsSuite, ArgumentCountMismatch) {
    std::string code = R"(
        func = function(value) return 1 end function

        func(1, 2)

        print(239)
    )";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_FALSE(interpret(input, output));
    ASSERT_FALSE(output.str().ends_with(kUnreachable));
}
