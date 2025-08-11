cmake_minimum_required(VERSION 3.11)
project(code_plagiarism_checker)

set(CMAKE_CXX_STANDARD 17)

add_executable(code_plagiarism_checker
        src/main.cpp
        src/tree_isomorphism.cpp
)

target_include_directories(code_plagiarism_checker
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)