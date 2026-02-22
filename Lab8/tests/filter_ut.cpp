#include <processing.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>


TEST(FilterTest, FilterEmpty) {
    std::vector<int> input = {};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return x % 2 == 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre());
}


TEST(FilterTest, FilterGreaterThanThree) {
    std::vector<int> input = {1, 2, 3, 4, 5, 6};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return x > 3; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(4, 5, 6));
}


TEST(FilterTest, FilterPalindromeStrings) {
    std::vector<std::string> input = {"radar", "hello", "level", "world", "noon"};
    auto isPalindrome = [](const std::string& s) {
        return std::equal(s.begin(), s.begin() + s.size()/2, s.rbegin());
    };
    auto result = AsDataFlow(input)
                  | Filter([&](const std::string& s) { return isPalindrome(s); })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("radar", "level", "noon"));
}


TEST(FilterTest, FilterAllLowerCase) {
    std::vector<std::string> input = {"hello", "WORLD", "python", "Java", "cpp"};
    auto result = AsDataFlow(input)
                  | Filter([](const std::string& s) {
                        return std::all_of(s.begin(), s.end(), [](char c) {
                                   return !std::isalpha(c) || std::islower(c);
                               });
                    })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("hello", "python", "cpp"));
}

TEST(FilterTest, FilterMultiplesOfThree) {
    std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return x % 3 == 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(3, 6, 9));
}


TEST(FilterTest, FilterStringsLongerThanFive) {
    std::vector<std::string> input = {"short", "longer", "lengthy", "tiny", "adequate"};
    auto result = AsDataFlow(input)
                  | Filter([](const std::string& s) { return s.size() > 5; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("longer", "lengthy", "adequate"));
}


TEST(FilterTest, FilterNonNegative) {
    std::vector<int> input = {-3, -1, 0, 2, 5};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return x >= 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(0, 2, 5));
}

TEST(FilterTest, FilterStringsWithoutSpace) {
    std::vector<std::string> input = {"hello world", "nospace", "with space", "clean", ""};
    auto result = AsDataFlow(input)
                  | Filter([](const std::string& s) { return s.find(' ') == std::string::npos; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("nospace", "clean", ""));
}


TEST(FilterTest, FilterEvenAbsolute) {
    std::vector<int> input = {-4, -3, -2, 0, 1, 2, 3, 4};
    auto result = AsDataFlow(input)
                  | Filter([](int x) { return std::abs(x) % 2 == 0; })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(-4, -2, 0, 2, 4));
}


TEST(FilterTest, FilterStartsWithVowel) {
    std::vector<std::string> input = {"apple", "banana", "orange", "umbrella", "kiwi", "Egg"};
    auto result = AsDataFlow(input)
                  | Filter([](const std::string& s) {
                        if (s.empty()) return false;
                        char first = std::tolower(s.front());
                        return first == 'a' || first == 'e' || first == 'i' || first == 'o' || first == 'u';
                    })
                  | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("apple", "orange", "umbrella", "Egg"));
}
