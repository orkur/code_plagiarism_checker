cmake_minimum_required(VERSION 3.11)
project(code_plagiarism_checker)

set(CMAKE_CXX_STANDARD 17)

find_package(nlohmann_json REQUIRED)

include_directories(include)

add_executable(code_plagiarism_checker
        src/main.cpp
        src/tree_isomorphism.cpp
)

target_link_libraries(code_plagiarism_checker
        PRIVATE nlohmann_json::nlohmann_json)
