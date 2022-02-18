cmake_minimum_required(VERSION 3.20)



add_executable(event_processing_test ../base_impl/base_stage.cpp main.cpp base_source_test.cpp base_stage_test.cpp dynamic_message_test.cpp)
target_link_libraries(event_processing_test gtest pthread)
