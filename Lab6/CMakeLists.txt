cmake_minimum_required(VERSION 3.10)
project(your_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/admin/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
#cmake .. -DCMAKE_TOOLCHAIN_FILE="C:/User/admin/vcpkg/scripts/buildsystems/vcpkg.cmake"
#cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="C:/Users/admin/vcpkg/scripts/buildsystems/vcpkg.cmake"

find_package(CURL REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(main main.cpp)


target_link_libraries(main PRIVATE 
    cpr::cpr
    nlohmann_json::nlohmann_json
)