cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

add_executable(services_test ./key_value_store_tests.cpp main.cpp)
target_link_libraries(services_test gtest pthread)
