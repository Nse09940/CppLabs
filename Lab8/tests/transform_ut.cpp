#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(TransformTest, PowerOfTwo) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | Transform([](int x) { return x * x; }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 4, 9, 16, 25));
}

TEST(TransformTest, FromStringToInt) {
    std::vector<int> files{1,2,3,4,5,6,7,8,9,10};
    auto result = AsDataFlow(files) | Transform([](const int x) { return x*x; }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 4, 9, 16, 25, 36, 49, 64, 81, 100));
}
