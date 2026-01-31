// #include <processing.h>
// #include <gmock/gmock.h>
// #include <gtest/gtest.h>
// #include <expected>

// struct Department {
//     std::string name;
//     bool operator==(const Department& rhs) const {
//         return name == rhs.name;
//     }
// };

// std::expected<Department, std::string> ParseDepartment(const std::string& str) {
//     if (str.empty())
//         return std::unexpected("Department name is empty");
//     if (str.find(' ') != std::string::npos)
//         return std::unexpected("Department name contains space");
//     return Department{str};
// }

// TEST(SplitExpectedTest, SplitExpected) {
//     // Используем копируемый контейнер std::vector<std::string>
//     std::vector<std::string> files = {
//         "good-department|bad department||another-good-department"
//     };

//     auto [unexpected_flow, good_flow] =
//         AsDataFlow(files)
//         | Split("|")
//         | Transform(ParseDepartment)
//         | SplitExpected(ParseDepartment);

//     std::stringstream unexpected_file;
//     unexpected_flow | Write(unexpected_file, '.');

//     auto expected_result = good_flow | AsVector();

//     ASSERT_EQ(unexpected_file.str(),
//               "Department name contains space.Department name is empty.");
//     ASSERT_THAT(expected_result, testing::ElementsAre(
//         Department{"good-department"},
//         Department{"another-good-department"}
//     ));
// }
