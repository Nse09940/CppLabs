#include <iostream>
#include <vector>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <iterator>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <format>
#include <algorithm>
#include <processing.h>
#include <cstdlib> 

using namespace std;



int main(int argc, char* argv[]) {
    if(argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <directory>" << std::endl;
      return 1;
    }

    auto pipeline = Dir(argv[1], 0)
        | Filter([](std::filesystem::path& p){ return p.extension() == ".txt"; })
        | OpenFiles()
        | Split("\n ,.;")
        | Transform(
            [](std::string token) { 
                std::transform(token.begin(), token.end(), token.begin(), [](unsigned char c){ return std::tolower(c); });
                return token;
            })
        | AggregateByKey(
            0ULL, 
            [](const std::string&, size_t& count) { ++count; },
            [](const std::string& token) { return token; }
          )
        | Transform([](const std::pair<std::string, size_t>& stat) {
              return std::format("{} - {}", stat.first, stat.second);
          })
        | Out(std::cout);

    return 0;
}

