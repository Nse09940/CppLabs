#include <lib/interpreter.h>
#include <gtest/gtest.h>
inline bool interpret(std::istream& in, std::ostream& out) {
    return itmoscript::interpret(in, out);
}
TEST(FunctionTestSuite, SimpleFunctionTest) {
    std::string code = R"(
        incr = function(value)
            return value + 1
        end function

        x = incr(2)
        print(x)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, FunctionAsArgTest) {
    std::string code = R"(
        incr = function(value)
            return value + 1
        end function

        printresult = function(value, func)
            result = func(value)
            print(result)
        end function

        printresult(2, incr)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, NestedFunctionTest) {
    std::string code = R"(
        // NB: inner and outer `value` are different symbols.
        // You are not required to implement closures (aka lambdas).

        incr_and_print = function(value)
            incr = function(value)
                return value + 1
            end function

            print(incr(value))
        end function

        incr_and_print(2)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, FunnySyntaxTest) {
    std::string code = R"(
        funcs = [
            function() return 1 end function,
            function() return 2 end function,
            function() return 3 end function,
        ]

        print(funcs[0]())
        print(funcs[1]())
        print(funcs[2]())
    )";

    std::string expected = "123";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}
TEST(MissingFeature, LogicalAnd) {
    const std::string src =
        "if true and false then\n"
        "  print(1)\n"
        "else\n"
        "  print(2)\n"
        "end if";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "2");
}

TEST(MissingFeature, LogicalOr) {
    const std::string src =
        "if false or true then\n"
        "  print(42)\n"
        "else\n"
        "  print(0)\n"
        "end if";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "42");
}

TEST(MissingFeature, LogicalNot) {
    const std::string src =
        "if not false then\n"
        "  print(99)\n"
        "else\n"
        "  print(0)\n"
        "end if";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "99");
}

// 2. Вещественные числа и научная нотация
TEST(MissingFeature, FloatLiteral) {
    const std::string src =
        "a = 3.1415\n"
        "print(a)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "3.1415");
}

TEST(MissingFeature, ScientificNotation) {
    const std::string src =
        "x = 1.23e-2\n"
        "print(x)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "0.0123");
}

// 3. Оператор возведения в степень '^'
TEST(MissingFeature, Exponentiation) {
    const std::string src =
        "print(2 ^ 10)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "1024");
}

// 4. Компилированные присваивания: +=, -=, *=, /=, %=, ^=
TEST(MissingFeature, PlusEqual) {
    const std::string src =
        "a = 5\n"
        "a += 3\n"
        "print(a)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "8");
}

TEST(MissingFeature, MinusEqual) {
    const std::string src =
        "a = 10\n"
        "a -= 4\n"
        "print(a)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "6");
}

TEST(MissingFeature, MultiplyEqual) {
    const std::string src =
        "a = 3\n"
        "a *= 7\n"
        "print(a)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "21");
}

TEST(MissingFeature, DivideEqual) {
    const std::string src =
        "a = 20\n"
        "a /= 4\n"
        "print(a)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "5");
}

TEST(MissingFeature, ModEqual) {
    const std::string src =
        "a = 20\n"
        "a %= 6\n"
        "print(a)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "2");
}

TEST(MissingFeature, PowEqual) {
    const std::string src =
        "a = 2\n"
        "a ^= 5\n"
        "print(a)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "32");
}

// 5. Строковое вычитание: удаление суффикса
TEST(MissingFeature, StringSubtraction) {
    const std::string src =
        "s = \"hello world\"\n"
        "print(s - \" world\")";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "hello");
}

// 6. Индексация и срезы строк
TEST(MissingFeature, StringIndexing) {
    const std::string src =
        "s = \"abcdef\"\n"
        "print(s[2])";  // ожидается 'c'
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "c");
}

TEST(MissingFeature, StringSliceFull) {
    const std::string src =
        "s = \"abcdef\"\n"
        "print(s[:])";  // полная копия
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "abcdef");
}

TEST(MissingFeature, StringSliceTo) {
    const std::string src =
        "s = \"abcdef\"\n"
        "print(s[:3])";  // первые 3 символа "abc"
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "abc");
}

TEST(MissingFeature, StringSliceFrom) {
    const std::string src =
        "s = \"abcdef\"\n"
        "print(s[3:])";  // с 3-го до конца: "def"
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "def");
}

TEST(MissingFeature, StringSliceRange) {
    const std::string src =
        "s = \"abcdef\"\n"
        "print(s[1:4])";  // символы с индекса 1 по 3: "bcd"
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "bcd");
}

// 7. Конкатенация и умножение списков
TEST(MissingFeature, ListConcat) {
    const std::string src =
        "a = [1, 2]\n"
        "b = [3, 4]\n"
        "print(a + b)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "[1, 2, 3, 4]");
}

TEST(MissingFeature, ListRepeat) {
    const std::string src =
        "a = [5, 6]\n"
        "print(a * 3)";  // [5,6,5,6,5,6]
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "[5, 6, 5, 6, 5, 6]");
}

// 8. Срезы списков
TEST(MissingFeature, ListSlice) {
    const std::string src =
        "l = [1, 2, 3, 4, 5]\n"
        "print(l[1:4])";  // [2, 3, 4]
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "[2, 3, 4]");
}

// 9. Стандартная библиотека: числовые функции
TEST(MissingFeature, AbsFunction) {
    const std::string src =
        "print(abs(-7))";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "7");
}

TEST(MissingFeature, CeilFunction) {
    const std::string src =
        "print(ceil(3.2))";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "4");
}

TEST(MissingFeature, FloorFunction) {
    const std::string src =
        "print(floor(3.8))";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "3");
}

TEST(MissingFeature, RoundFunction) {
    const std::string src =
        "print(round(2.5))";  // стандартное округление до ближайшего -> 3
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "3");
}

TEST(MissingFeature, SqrtFunction) {
    const std::string src =
        "print(sqrt(16))";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "4");
}

TEST(MissingFeature, RndFunction) {
    // Предполагается, что rnd(5) возвращает int в диапазоне [0, 4].
    // Проверять строгое значение нельзя, но можно проверить, что итоговая строка
    // состоит из цифры от '0' до '4' и интерпретатор не падает.
    const std::string src =
        "x = rnd(5)\n"
        "print(x)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    ASSERT_FALSE(out.str().empty());
    char c = out.str()[0];
    EXPECT_GE(c, '0');
    EXPECT_LE(c, '4');
}

// 10. Стандартная библиотека: parse_num, to_string
TEST(MissingFeature, ParseNumValid) {
    const std::string src =
        "print(parse_num(\"123\"))";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "123");
}

TEST(MissingFeature, ParseNumInvalid) {
    const std::string src =
        "print(parse_num(\"abc\"))";  // должно вернуть nil
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "nil");
}

TEST(MissingFeature, ToStringFunction) {
    const std::string src =
        "print(to_string(456))";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "456");
}

// 11. Стандартная библиотека: строковые функции lower, upper, split, join, replace
TEST(MissingFeature, LowerFunction) {
    const std::string src =
        "print(lower(\"HeLLo\"))";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "hello");
}

TEST(MissingFeature, UpperFunction) {
    const std::string src =
        "print(upper(\"WoRlD\"))";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "WORLD");
}

TEST(MissingFeature, SplitFunction) {
    const std::string src =
        "print(split(\"a,b,c\", \",\"))";  // ["a", "b", "c"]
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "[\"a\", \"b\", \"c\"]");
}

TEST(MissingFeature, JoinFunction) {
    const std::string src =
        "print(join([\"x\", \"y\", \"z\"], \",\"))";  // "x,y,z"
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "x,y,z");
}

TEST(MissingFeature, ReplaceFunction) {
    const std::string src =
        "print(replace(\"abracadabra\", \"a\", \"o\"))";  // "obrocodobro"
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "obrocodobro");
}

// 12. Стандартная библиотека: функции работы со списками push, pop, insert, remove, sort
TEST(MissingFeature, PushFunction) {
    const std::string src =
        "l = [1, 2]\n"
        "push(l, 3)\n"
        "print(l)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "[1, 2, 3]");
}

TEST(MissingFeature, PopFunction) {
    const std::string src =
        "l = [1, 2, 3]\n"
        "x = pop(l)\n"
        "print(x)\n"
        "print(len(l))";  // x == 3, len(l) == 2
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "3" "2");  // без пробела между двумя print
}

TEST(MissingFeature, InsertFunction) {
    const std::string src =
        "l = [1, 3]\n"
        "insert(l, 1, 2)\n"
        "print(l)";  // [1, 2, 3]
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "[1, 2, 3]");
}

TEST(MissingFeature, RemoveFunction) {
    const std::string src =
        "l = [1, 2, 3]\n"
        "remove(l, 1)\n"
        "print(l)";  // [1, 3]
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "[1, 3]");
}

TEST(MissingFeature, SortFunction) {
    const std::string src =
        "l = [3, 1, 2]\n"
        "sort(l)\n"
        "print(l)";  // [1, 2, 3]
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "[1, 2, 3]");
}

// 13. Системные функции: println, read, stacktrace
TEST(MissingFeature, PrintlnFunction) {
    const std::string src =
        "println(\"hi\")";  // должна добавить перевод строки
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "hi\n");
}

TEST(MissingFeature, ReadFunction_NotImplemented) {
    const std::string src =
        "print(read())";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    // Поскольку read() не реализована, интерпретатор должен дать ошибку (false).
    EXPECT_FALSE(ok);
}

TEST(MissingFeature, StacktraceFunction_NotImplemented) {
    const std::string src =
        "print(stacktrace())";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    // stacktrace() не реализована — ожидаем, что интерпретатор вернёт false.
    EXPECT_FALSE(ok);
}

// 14. Break и Continue
TEST(MissingFeature, BreakInWhile) {
    const std::string src =
        "i = 0\n"
        "while true\n"
        "  i = i + 1\n"
        "  if i == 3 then break end if\n"
        "end while\n"
        "print(i)";
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "3");
}

TEST(MissingFeature, ContinueInFor) {
    const std::string src =
        "sum = 0\n"
        "for i in [1, 2, 3]\n"
        "  if i == 2 then continue end if\n"
        "  sum = sum + i\n"
        "end for\n"
        "print(sum)";  // пропускаем i=2, суммируем 1 + 3 = 4
    std::stringstream in(src), out;
    bool ok = interpret(in, out);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.str(), "4");
}

