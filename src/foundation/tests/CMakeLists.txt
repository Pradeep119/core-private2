cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

add_executable(foundation_test ./static_hash_tests.cpp main.cpp)
target_link_libraries(foundation_test gtest pthread)
