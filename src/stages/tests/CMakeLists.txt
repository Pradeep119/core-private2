cmake_minimum_required(VERSION 3.20)

add_executable(stages_test
        message_augmentor_tests.cpp
        ../../event_processing/base_impl/base_stage.cpp
        round_robin_load_balancer_tests.cpp
        main.cpp
        http_request_validator_tests.cpp
        sliding_window_rate_limiter_tests.cpp
        http_server_stage_tests.cpp
        filter_present_absent_stage_tests.cpp
        jwt_validation_stage_tests.cpp
        ../http_validation_stages/request_validation_stage.cpp)

target_include_directories(stages_test PUBLIC ~/development/axp/nextgen/3rdparty/include)
target_link_directories(stages_test PUBLIC ~/development/axp/nextgen/3rdparty/lib)
target_link_libraries(stages_test gtest pthread gcov ssl crypto)

