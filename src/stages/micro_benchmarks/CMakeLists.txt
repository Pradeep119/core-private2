cmake_minimum_required(VERSION 3.20)

add_executable(stages_micro_benchmark main.cpp router_stage_micro_benchmark.cpp ../../event_processing/base_impl/base_stage.cpp sliding_window_rate_limiter_micro_benchmark.cpp)

target_include_directories(stages_micro_benchmark PUBLIC ~/development/axp/nextgen/3rdparty/include)
target_link_directories(stages_micro_benchmark PUBLIC ~/development/axp/nextgen/3rdparty/lib)
target_link_libraries(stages_micro_benchmark gtest pthread benchmark)

