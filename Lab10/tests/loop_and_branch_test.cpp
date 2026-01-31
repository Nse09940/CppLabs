#include <lib/interpreter.h>
#include <gtest/gtest.h>
inline bool interpret(std::istream& in, std::ostream& out) {
    return itmoscript::interpret(in, out);
}
TEST(BranchTestSuite, SimpleIfTest) {
    std::string code = R"(
        cond = true
        if cond then
            print("true")
        end if
    )";

    std::string expected = "true";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}

TEST(BranchTestSuite, SimpleElseIfTest) {
    std::string code = R"(
        cond = false
        if cond then
            print("true")
        else
            print("false")
        end if
    )";

    std::string expected = "false";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(BranchTestSuite, ComplexIfTest) {
    std::string code = R"(
        v = 100 * 2 + 10 * 3 + 9
        if v == 30 then
            print(30)
        else if v == 366 then
            print(366)
        else if v == 239 then
            print(239)
        else
            print(0)
        end if
    )";

    std::string expected = "239";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(BranchTestSuite, OneLineIfTest) {
    std::string code = "if 2 * 2 == 4 then print(\"2 * 2 == 4\") else print(\"omg\") end if";
    std::string expected = "\"2 * 2 == 4\"";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(LoopTestSuit, ForLoop) {
    std::string code = R"(
        for i in range(0,5,1)
            print(i)
        end for
    )";

    std::string expected = "01234";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(LoopTestSuit, WhileLoop) {
    std::string code = R"(
        s = "ITMO"
        while  len(s) < 12
            s = s * 2
        end while
        print(s)
    )";

    std::string expected = "ITMOITMOITMOITMO";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}






































#include <sstream>


// Helper to run a piece of ItmoScript code and capture its output
static bool runScript(const std::string& code, std::string& outStr) {
    std::istringstream input(code);
    std::ostringstream output;
    bool success = interpret(input, output);
    outStr = output.str();
    return success;
}

// 1. Arithmetic and Print Tests
TEST(ArithmeticTestSuite, SimpleAdditionAndMultiplication) {
    std::string code = R"(
        print(1 + 2 * 3)
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "7");
}

TEST(ArithmeticTestSuite, ParenthesesChangePrecedence) {
    std::string code = R"(
        print((1 + 2) * 3)
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "9");
}



// 2. Variable Assignment and Usage
TEST(VariableTestSuite, AssignAndPrintVariable) {
    std::string code = R"(
        x = 42
        print(x)
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "42");
}

TEST(VariableTestSuite, ReassignVariable) {
    std::string code = R"(
        x = 5
        x = x + 10
        print(x)
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "15");
}

// 3. If / ElseIf / Else Tests
TEST(IfElseTestSuite, SimpleIfTrue) {
    std::string code = R"(
        if true then
            print(1)
        else
            print(0)
        end if
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "1");
}

TEST(IfElseTestSuite, SimpleIfFalse) {
    std::string code = R"(
        if false then
            print(1)
        else
            print(0)
        end if
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "0");
}

TEST(IfElseTestSuite, ElseIfChain) {
    std::string code = R"(
        v = 100 * 2 + 10 * 3 + 9
        if v == 30 then
            print(30)
        else if v == 366 then
            print(366)
        else if v == 239 then
            print(239)
        else
            print(0)
        end if
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "239");
}

TEST(IfElseTestSuite, NestedElseIfAllFalse) {
    std::string code = R"(
        v = 10
        if v == 5 then
            print(5)
        else if v == 7 then
            print(7)
        else
            print(-1)
        end if
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "-1");
}

// 4. For Loop Tests
TEST(ForLoopTestSuite, LoopOverArray) {
    std::string code = R"(
        arr = [1, 2, 3]
        for i in arr
            print(i)
        end for
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    // Expect "123"
    ASSERT_EQ(output, "123");
}

TEST(ForLoopTestSuite, LoopUsingRangeFunction) {
    std::string code = R"(
        r = range(1, 4)
        for x in r
            print(x)
        end for
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    // range(1,4) yields [1,2,3]
    ASSERT_EQ(output, "123");
}

TEST(ForLoopTestSuite, RangeWithStep) {
    std::string code = R"(
        r = range(0, 10, 3)
        for x in r
            print(x)
        end for
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    // range(0,10,3) yields [0,3,6,9]
    ASSERT_EQ(output, "0369");
}

// 5. While Loop Tests
TEST(WhileLoopTestSuite, SimpleWhileLoop) {
    std::string code = R"(
        i = 1
        while i <= 3
            print(i)
            i = i + 1
        end while
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "123");
}

TEST(WhileLoopTestSuite, ZeroIterations) {
    std::string code = R"(
        i = 5
        while i < 3
            print(i)
            i = i + 1
        end while
        print(0)
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    // While body never executes; prints only "0"
    ASSERT_EQ(output, "0");
}

// 6. Function Definition and Call Tests
TEST(FunctionTestSuite, SimpleFunctionPrint) {
    std::string code = R"(
        f = function(x)
            print(x * 2)
        end function
        f(5)
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "10");
}

TEST(FunctionTestSuite, FunctionWithReturnValue) {
    std::string code = R"(
        sum = function(a, b)
            return a + b
        end function
        result = sum(3, 4)
        print(result)
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "7");
}

TEST(FunctionTestSuite, RecursiveFactorial) {
    std::string code = R"(
        fact = function(n)
            if n == 0 then
                return 1
            else
                return n * fact(n - 1)
            end if
        end function
        print(fact(5))
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    // 5! = 120
    ASSERT_EQ(output, "120");
}

// 7. Array and Len Tests
TEST(ArrayTestSuite, CreateAndPrintArrayLiteral) {
    std::string code = R"(
        a = [ "foo", "bar", "baz" ]
        print(a[1])      # 0-based indexing: index 1 -> "bar"
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    ASSERT_EQ(output, "bar");
}

TEST(ArrayTestSuite, LengthOfStringAndArray) {
    std::string code = R"(
        s = "hello"
        print(len(s))
        a = [1, 2, 3, 4]
        print(len(a))
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    // len("hello") = 5, len([1,2,3,4]) = 4
    ASSERT_EQ(output, "54");
}

TEST(ArrayTestSuite, IndexOutOfBoundsThrows) {
    std::string code = R"(
        a = [1, 2, 3]
        print(a[5])
    )";
    std::string output;
    // Expect interpret() to return false due to out-of-bounds
    ASSERT_FALSE(runScript(code, output));
}

// 8. Combined Complex Script
TEST(ComplexScriptTestSuite, CombinedFeatures) {
    std::string code = R"(
        # Compute the sum of even numbers from 1 to 10
        sum = 0
        for i in range(1, 11)
            if i % 2 == 0 then
                sum = sum + i
            end if
        end for
        print(sum)    # 2 + 4 + 6 + 8 + 10 = 30

        # Test a nested function
        mul = function(a, b)
            return a * b
        end function

        print(mul(sum, 2))  # 30 * 2 = 60
    )";
    std::string output;
    ASSERT_TRUE(runScript(code, output));
    // First print: "30", second print: "60"
    ASSERT_EQ(output, "3060");
}

// 9. Error Handling Tests
TEST(ErrorHandlingTestSuite, UndefinedVariableThrows) {
    std::string code = R"(
        print(x)
    )";
    std::string output;
    ASSERT_FALSE(runScript(code, output));
}



TEST(ErrorHandlingTestSuite, RangeStepZeroThrows) {
    std::string code = R"(
        r = range(1, 10, 0)
    )";
    std::string output;
    ASSERT_FALSE(runScript(code, output));
}














































// itmoscript_big_complex_test.cpp





TEST(ComplexStressTestSuite, BigCombinedFeatures) {
    std::string code = R"(
        # Определение факториала (рекурсия)
        fact = function(n)
            if n == 0 then
                return 1
            else
                return n * fact(n - 1)
            end if
        end function

        # Определение чисел Фибоначчи (рекурсия)
        fib = function(m)
            if m == 0 then
                return 0
            else if m == 1 then
                return 1
            else
                return fib(m - 1) + fib(m - 2)
            end if
        end function

        # Вывод первых 5 факториалов
        i = 1
        while i <= 5
            print(fact(i))
            i = i + 1
        end while

        # Вывод первых 6 чисел Фибоначчи (0..5)
        for j in range(0, 6)
            print(fib(j))
        end for

        # Тест работы с массивами
        arr = [ fact(3), fib(4), 42, "hello" ]
        print(len(arr))
        print(arr[0])
        print(arr[1])
        print(arr[2])
        print(arr[3])

        # Сложное вложенное условие
        x = 10
        if x < 5 then
            print("small")
        else if x < 15 then
            if x % 2 == 0 then
                print("even")
            else
                print("odd")
            end if
        else
            print("large")
        end if

        # Операции со строками: умножение и конкатенация
        s = "a"
        print(s * 3)        # "aaa"
        s2 = "b"
        print(s2 + s)       # "ba"
    )";
    
    std::string output;
    ASSERT_TRUE(runScript(code, output));

    // Ожидаемый результат:
    // Факториалы 1..5:          1 2 6 24 120  → "12624120"
    // Фибоначчи 0..5:           0 1 1 2 3 5    → "011235"
    // len(arr)=4, arr[0]=6, arr[1]=3, arr[2]=42, arr[3]="hello"
    //                          → "46342hello"
    // Сложный if: x=10 (<15 и чётное) → "even"
    // s*3 = "aaa", s2 + s = "ba"
    //
    // Вся строка без разделителей:
    // "12624120" + "011235" + "46342hello" + "even" + "aaa" + "ba"
    // = "1262412001123546342helloevenaaaba"
    ASSERT_EQ(output, "1262412001123546342helloevenaaaba");
}

//  ❗ gtest: ловим баг с замыканиями (лексическое ↔ динамическое окружение)
TEST(ClosureTestSuite, StaticScopingClosureTest) {
    // make_adder возвращает лямбду, запоминающую параметр a
    const std::string code = R"(
        make_adder = function(a)
            return function(b)
                return a + b           # a ДОЛЖЕН быть доступен здесь
            end function
        end function

        add5 = make_adder(5)
        print(add5(3))                 # ждём 8
    )";

    const std::string expected = "8";

    std::istringstream input(code);
    std::ostringstream output;

    // При статическом (правильном) связывании должно вернуться true и напечататься 8
    ASSERT_TRUE( interpret(input, output) )
        << "Интерпретатор упал: вероятно, переменная «a» не найдена (динамическое окружение).";

    ASSERT_EQ( output.str(), expected )
        << "Лямбда не захватила «a» из make_adder – проверьте реализацию окружений.";
}






TEST(EnvScopingTestSuite, LoopVariableDoesNotLeakAndLocalAssignDoesNotOverwrite) {
    const std::string code = R"(
        x = 100
        sum = 0

        
        for i in range(1, 5)
            sum = sum + i
            
            helper = function()
                x = x + 1
                return x
            end function
            
            helper()
        end for

        print(x)      # ожидаем 100
        print(",")
        print(sum)    # ожидаем 10  (1+2+3+4=10)
    )";

    const std::string expected = "100,10";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE( interpret(input, output) )
        << "Интерпретатор упал — вероятно, неверно устроено окружение (Env).";

    ASSERT_EQ( output.str(), expected )
        << "После цикла x должно остаться 100, а sum == 10.";
}




















//
// 2) Тест для рекурсивной функции Fibonacci
TEST(RecursiveTest, Fibonacci) {
    const std::string code = R"(
        fib = function(n)
            if n == 0 then
                return 0
            else if n == 1 then
                return 1
            else
                return fib(n - 1) + fib(n - 2)
            end if
        end function

        print(fib(10)); print("\n")
    )";

    const std::string expected = "55\n";

    std::istringstream input(code);
    std::ostringstream output;
    ASSERT_TRUE( interpret(input, output) )
        << "Интерпретатор упал при тесте Fibonacci.";

    ASSERT_EQ(output.str(), expected)
        << "Неверный вывод для Fibonacci.";
}

// 3) Тест для функции-counter: проверяем замыкание внешней переменной c
TEST(ClosureTest, Counter) {
    const std::string code = R"(
        counter = function()
            c = 0
            return function() 
                c = c + 1
                return c
            end function
        end function

        ctr1 = counter()
        print(ctr1()); print("\n")  # 1
        print(ctr1()); print("\n")  # 2
        ctr2 = counter()
        print(ctr2()); print("\n")  # 1
    )";

    const std::string expected =
        "1\n"
        "2\n"
        "1\n";

    std::istringstream input(code);
    std::ostringstream output;
    ASSERT_TRUE( interpret(input, output) )
        << "Интерпретатор упал при тесте counter.";

    ASSERT_EQ(output.str(), expected)
        << "Неверный вывод для counter.";
}

// 4) Тест для массива, len и суммирования
TEST(ArrayTest, LenAndSum) {
    const std::string code = R"(
        arr = [1, 2, 3, 4, 5]
        print(len(arr)); print("\n")    # 5

        s = 0
        for i in range(0, len(arr))
            s = s + arr[i]
        end for
        print(s); print("\n")          # 15
    )";

    const std::string expected =
        "5\n"
        "15\n";

    std::istringstream input(code);
    std::ostringstream output;
    ASSERT_TRUE( interpret(input, output) )
        << "Интерпретатор упал при тесте LenAndSum.";

    ASSERT_EQ(output.str(), expected)
        << "Неверный вывод для LenAndSum.";
}

// 5) Тест для вывода чётных элементов массива
TEST(ControlFlowTest, PrintEven) {
    const std::string code = R"(
        arr = [1, 2, 3, 4, 5]
        print("Even: "); print("\n")
        for i in range(0, len(arr))
            if arr[i] % 2 == 0 then
                print(arr[i]); print("\n")
            end if
        end for
    )";

    const std::string expected =
        "Even: \n"
        "2\n"
        "4\n";

    std::istringstream input(code);
    std::ostringstream output;
    ASSERT_TRUE( interpret(input, output) )
        << "Интерпретатор упал при тесте PrintEven.";

    ASSERT_EQ(output.str(), expected)
        << "Неверный вывод для PrintEven.";
}




TEST(MissingFeaturesTest, ReadNotImplemented) {
    // Программа на ITMOScript, вызывающая read()
    std::string code = R"(
        // Попытка прочитать строку
        x = read()
        println(x)
    )";

    std::stringstream input(code);
    std::stringstream output;
    // interpret() должен вернуть false, так как read() не реализована
    EXPECT_FALSE(itmoscript::interpret(input, output));
}

// Тест для непретворённой функции stacktrace()
TEST(MissingFeaturesTest, StacktraceNotImplemented) {
    // Программа на ITMOScript, вызывающая stacktrace()
    std::string code = R"(
        // Попытка получить стек вызова
        s = stacktrace()
        println(s)
    )";

    std::stringstream input(code);
    std::stringstream output;
    // interpret() должен вернуть false, так как stacktrace() не реализована
    EXPECT_FALSE(itmoscript::interpret(input, output));
}

// Дополнительные тесты, проверяющие ожидаемое поведение операторов / и ^
// согласно техническому заданию (они не будут проходить до исправления).
TEST(ArithmeticTests, DivisionFloatingPoint) {
    // Ожидаем: 5 / 2 = 2.5
    std::string code = R"(
        println(5/2)
    )";

    std::stringstream input(code);
    std::stringstream output;
    bool ok = itmoscript::interpret(input, output);
    // Если реализация не изменена, interpret вернёт true,
    // но вывод будет "2" вместо "2.5", поэтому проверка на точное совпадение даст FAIL.
    EXPECT_TRUE(ok);
    EXPECT_EQ(output.str(), "2.5\n");
}

TEST(ArithmeticTests, ExponentPrecedence) {
    // По ТЗ: 2 * 3^2 должно читаться как 2 * (3^2) = 18
    std::string code = R"(
        println(2 * 3 ^ 2)
    )";

    std::stringstream input(code);
    std::stringstream output;
    bool ok = itmoscript::interpret(input, output);
    // Если реализация не изменена, interpret вернёт true,
    // но вывод будет "36" вместо "18", поэтому проверка на точное совпадение даст FAIL.
    EXPECT_TRUE(ok);
    EXPECT_EQ(output.str(), "18\n");
}
















TEST(MissingFeaturesTest2, ReadStillNotImplemented) {
    const std::string code = R"(
        // Попытка использовать read()
        x = read()
        println(x)
    )";

    std::stringstream input(code);
    std::stringstream output;
    // interpret должен вернуть false, потому что read() пока не реализован
    EXPECT_FALSE(itmoscript::interpret(input, output));
}

// --- 2. Проверяем, что stacktrace() не реализован ---

TEST(MissingFeaturesTest2, StacktraceStillNotImplemented) {
    const std::string code = R"(
        // Попытка получить стек
        s = stacktrace()
        println(s)
    )";

    std::stringstream input(code);
    std::stringstream output;
    // interpret должен вернуть false, потому что stacktrace() пока не реализован
    EXPECT_FALSE(itmoscript::interpret(input, output));
}

// --- 3. Дробное деление: ожидание 2.5 для 5/2 ---

TEST(ArithmeticTests2, DivisionFloatingPointExpected) {
    const std::string code = R"(
        println(5 / 2)
    )";

    std::stringstream input(code);
    std::stringstream output;
    bool ok = itmoscript::interpret(input, output);
    // Интерпретатор должен успешно выполниться
    EXPECT_TRUE(ok);
    // Ожидаем именно "2.5\n" согласно ТЗ
    EXPECT_EQ(output.str(), "2.5\n");
}

// --- 4. Взятие остатка у дробных: ожидание 1.5 для 5.5 % 2 ---

TEST(ArithmeticTests2, ModuloFloatingPointExpected) {
    const std::string code = R"(
        println(5.5 % 2)
    )";

    std::stringstream input(code);
    std::stringstream output;
    bool ok = itmoscript::interpret(input, output);
    EXPECT_TRUE(ok);
    // Ожидаем "1.5\n" согласно ТЗ
    EXPECT_EQ(output.str(), "1.5\n");
}

// --- 5. Приоритет оператора ^: 2 * 3^2 == 18 ---

TEST(ArithmeticTests2, ExponentPrecedenceExpected) {
    const std::string code = R"(
        println(2 * 3 ^ 2)
    )";

    std::stringstream input(code);
    std::stringstream output;
    bool ok = itmoscript::interpret(input, output);
    EXPECT_TRUE(ok);
    // Ожидаем "18\n"
    EXPECT_EQ(output.str(), "18\n");
}

// --- 6. Составное деление /=: x = 5; x /= 2; println(x) == 2.5 ---

TEST(CompoundAssignmentTests2, DivideEqualFloatingPointExpected) {
    const std::string code = R"(
        x = 5
        x /= 2
        println(x)
    )";

    std::stringstream input(code);
    std::stringstream output;
    bool ok = itmoscript::interpret(input, output);
    EXPECT_TRUE(ok);
    // Ожидаем "2.5\n"
    EXPECT_EQ(output.str(), "2.5\n");
}

// --- 7. Составный остаток %=: x = 5.5; x %= 2; println(x) == 1.5 ---

TEST(CompoundAssignmentTests2, ModuloEqualFloatingPointExpected) {
    const std::string code = R"(
        x = 5.5
        x %= 2
        println(x)
    )";

    std::stringstream input(code);
    std::stringstream output;
    bool ok = itmoscript::interpret(input, output);
    EXPECT_TRUE(ok);
    // Ожидаем "1.5\n"
    EXPECT_EQ(output.str(), "1.5\n");
}