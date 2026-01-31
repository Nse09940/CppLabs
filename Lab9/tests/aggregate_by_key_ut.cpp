#include <scheduler.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <utility>

TEST(ZeroArgsTest, ReturnsValue) {
    TTaskScheduler sch;
    auto id = sch.add([](){ return 42; });
    EXPECT_EQ(sch.getResult<int>(id), 42);
}
TEST(OneArgTest, InvertsValue) {
    TTaskScheduler sch;
    auto id = sch.add([](int x){ return -x; }, 5);
    EXPECT_EQ(sch.getResult<int>(id), -5);
}
TEST(TwoArgsTest, AddsValues) {
    TTaskScheduler sch;
    auto id = sch.add([](int a, int b){ return a + b; }, 3, 7);
    EXPECT_EQ(sch.getResult<int>(id), 10);
}
TEST(DependencyTest, SquaresValue) {
    TTaskScheduler sch;
    auto id1 = sch.add([](){ return 4; });
    auto id2 = sch.add([](int v){ return v * v; }, sch.getFutureResult<int>(id1));
    EXPECT_EQ(sch.getResult<int>(id2), 16);
}
TEST(MemberFunctionTest, AddsConstant) {
    struct A { int f(int x) const { return x + 2; } } a;
    TTaskScheduler sch;
    auto id = sch.add(&A::f, a, 8);
    EXPECT_EQ(sch.getResult<int>(id), 10);
}
TEST(ExecuteAllTest, ProcessesAll) {
    TTaskScheduler sch;
    auto id1 = sch.add([](){ return 1; });
    auto id2 = sch.add([](int v){ return v + 2; }, sch.getFutureResult<int>(id1));
    auto id3 = sch.add([](int v){ return v * 3; }, sch.getFutureResult<int>(id2));
    sch.executeAll();
    EXPECT_EQ(sch.getResult<int>(id3), 9);
}
TEST(ChainTest, ThreeLevelDependency) {
    TTaskScheduler sch;
    auto id1 = sch.add([](){ return 1; });
    auto id2 = sch.add([](int v){ return v + 2; }, sch.getFutureResult<int>(id1));
    auto id3 = sch.add([](int v){ return v * 3; }, sch.getFutureResult<int>(id2));
    EXPECT_EQ(sch.getResult<int>(id3), 9);
}
TEST(ParallelTest, CombinesFutures) {
    TTaskScheduler sch;
    auto id1 = sch.add([](){ return 5; });
    auto id2 = sch.add([](){ return 10; });
    auto id3 = sch.add([](int a, int b){ return a + b; }, sch.getFutureResult<int>(id1), sch.getFutureResult<int>(id2));
    EXPECT_EQ(sch.getResult<int>(id3), 15);
}
TEST(DoubleReturnTest, SquareRoot) {
    TTaskScheduler sch;
    auto id = sch.add([](double x){ return std::sqrt(x); }, 16.0);
    EXPECT_DOUBLE_EQ(sch.getResult<double>(id), 4.0);
}
TEST(StringTest, Concatenation) {
    TTaskScheduler sch;
    auto id1 = sch.add([](){ return std::string("Hello"); });
    auto id2 = sch.add([](){ return std::string("World"); });
    auto id3 = sch.add([](const std::string& a, const std::string& b){ return a + " " + b; }, sch.getFutureResult<std::string>(id1), sch.getFutureResult<std::string>(id2));
    EXPECT_EQ(sch.getResult<std::string>(id3), "Hello World");
}
TEST(IdempotentTest, MultipleGetCalls) {
    TTaskScheduler sch;
    auto id = sch.add([](){ return 7; });
    EXPECT_EQ(sch.getResult<int>(id), 7);
    EXPECT_EQ(sch.getResult<int>(id), 7);
}
TEST(SumOfSquaresTest, TwoFutures) {
    TTaskScheduler sch;
    auto id1 = sch.add([](){ return 2; });
    auto id2 = sch.add([](){ return 3; });
    auto id3 = sch.add([](int x, int y){ return x*x + y*y; }, sch.getFutureResult<int>(id1), sch.getFutureResult<int>(id2));
    EXPECT_EQ(sch.getResult<int>(id3), 13);
}
TEST(NegativeNumbersTest, Subtraction) {
    TTaskScheduler sch;
    auto id = sch.add([](int a, int b){ return a - b; }, -5, -3);
    EXPECT_EQ(sch.getResult<int>(id), -2);
}
TEST(FloatingTest, Division) {
    TTaskScheduler sch;
    auto id = sch.add([](float a, float b){ return a / b; }, 5.0f, 2.0f);
    EXPECT_FLOAT_EQ(sch.getResult<float>(id), 2.5f);
}
TEST(BigIntTest, Multiplication) {
    TTaskScheduler sch;
    auto id = sch.add([](unsigned long long a, unsigned long long b){ return a * b; }, 100000ULL, 100000ULL);
    EXPECT_EQ(sch.getResult<unsigned long long>(id), 10000000000ULL);
}
TEST(PairTest, ReturnsPair) {
    TTaskScheduler sch;
    auto id = sch.add([](){ return std::make_pair(1, 2); });
    auto res = sch.getResult<std::pair<int,int>>(id);
    EXPECT_EQ(res.first, 1);
    EXPECT_EQ(res.second, 2);
}
TEST(CaptureTest, LambdaCapture) {
    TTaskScheduler sch;
    int c = 5;
    auto id = sch.add([c](){ return c * 2; });
    EXPECT_EQ(sch.getResult<int>(id), 10);
}
TEST(FunctorTest, StdPlus) {
    TTaskScheduler sch;
    std::plus<int> plus;
    auto id = sch.add(plus, 3, 4);
    EXPECT_EQ(sch.getResult<int>(id), 7);
}

