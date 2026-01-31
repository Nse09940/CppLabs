#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
TEST(FilterTest, FilterBooleanTrue) {
    std::vector<bool> input = {true, false, true, false, false};
    auto result = AsDataFlow(input)
                  | Filter([](bool x) { return x; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(true, true));
}

TEST(FilterTest, FilterNonZeroNumbers) {
    std::vector<int> input = {0, 1, 0, 2, 3, 0, 4};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return x != 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3, 4));
}


TEST(FilterTest, FilterNumbersWithEvenDigitSum) {
    std::vector<int> input = {11, 23, 13, 44, 31};
    auto sumDigits = [](int n) {
        int sum = 0;
        n = std::abs(n);
        while(n > 0) {
            sum += n % 10;
            n /= 10;
        }
        return sum;
    };
    auto result = AsDataFlow(input)
                  | Filter([&](int x) { return sumDigits(x) % 2 == 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(11, 13, 44, 31));
}


TEST(FilterTest, FilterEvenNumbersInRange) {
    std::vector<int> input = {5, 10, 12, 25, 50, 52, 60};
    auto result = AsDataFlow(input)
                  | Filter([](int x) {
                        return x % 2 == 0 && x >= 10 && x <= 50;
                    })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(10, 12, 50));
}


TEST(FilterTest, FilterNumbersDivisibleBySix) {
    std::vector<int> input = {2, 3, 4, 6, 12, 15, 18, 20};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return x % 6 == 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(6, 12, 18));
}
TEST(WriteTest, Write) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    std::stringstream file_emulator;
     AsDataFlow(input) | Write(file_emulator, '|');
    ASSERT_EQ(file_emulator.str(), "1|2|3|4|5|");
}
