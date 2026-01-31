#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
TEST(FilterTest, FilterNegativeNumbers) {
    std::vector<int> input = {-10, -5, 0, 3, 7, -2};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return x < 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(-10, -5, -2));
}


TEST(FilterTest, FilterStringsContainingDigit) {
    std::vector<std::string> input = {"abc", "a1b", "123", "hello", "world2"};
    auto result = AsDataFlow(input)
                  | Filter([](const std::string& s) {
                        return std::any_of(s.begin(), s.end(), [](char c) { return std::isdigit(c); });
                    })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("a1b", "123", "world2"));
}


TEST(FilterTest, FilterStringsWithEvenLength) {
    std::vector<std::string> input = {"one", "four", "five", "six", "ten"};
    auto result = AsDataFlow(input)
                  | Filter([](const std::string& s) { return s.size() % 2 == 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("four", "five"));
}

TEST(FilterTest, FilterNumbersGreaterThan25) {
    std::vector<int> input = {10, 20, 25, 30, 35, 40};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return x > 25; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(30, 35, 40));
}

TEST(ReadTest, ByNewLine) {
    std::vector<std::string> files{"1","2","3","4","5","6","7","8","9","10"};
    
    auto result = AsDataFlow(files)  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("1", "2", "3", "4", "5", "6", "7", "8", "9", "10"));
}

TEST(ReadTest, BySpace) {
    std::vector<std::string> files{"1","2","3","4","5","6","7","8","9","10"};
    
    auto result = AsDataFlow(files)  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("1", "2", "3", "4", "5", "6", "7", "8", "9", "10"));
}
