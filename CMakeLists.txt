cmake_minimum_required(VERSION 3.20)
project(core)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(ENABLE_METRICS_PREVIEW)

add_subdirectory(src)

include_directories(~/development/axp/nextgen/3rdparty/include ~/development/axp/nextgen/3rdparty/include/uSockets/)
link_directories(~/development/axp/nextgen/3rdparty/lib)

add_library(core STATIC src/stages/message_augmentor_stage/dummy.cpp src/event_processing/base_impl/base_stage.cpp src/stages/queue_stages/mpmc_queue.cpp src/stages/http_server_stage/http_server_stage.h src/stages/tcp_server_stage/tcp_server_stage.cpp src/services/store_services/key_value_store.h src/stages/http_server_stage/config_messages.h src/event_processing/dynamic_message.h src/stages/load_balancer_stages/round_robin_load_balancer_stage.h src/stages/http_validation_stages/request_validation_stage.h src/stages/http_validation_stages/request_validation_stage.cpp src/stages/http_validation_stages/request_validation.h src/services/store_services/key_complex_value_store.h src/services/store_services/key_complex_value_store.h src/event_processing/messages/task_execution_message.h src/stages/misc_stages/delay_stage.h src/stages/misc_stages/task_execution_sink.h src/stages/misc_stages/no_op_sink.h src/stages/load_balancer_stages/in_process_load_balancer_stage.h src/stages/jwt_decoder_stage/jwt_decoder_stage.h src/stages/throttling_stages/throttling_enforcer_stage.h src/stages/throttling_stages/throttle_state_builder_stage.cpp src/stages/throttling_stages/messages.h src/stages/throttling_stages/throttling_enforcer_stage.cpp src/stages/throttling_stages/config_messages.h src/stages/throttling_stages/sliding_log_throttler.h src/stages/throttling_stages/throttle_state_builder_stage.h src/stages/throttling_stages/throttler.h src/stages/throttling_stages/time_stamp_pack.h src/stages/misc_stages/printing_sink.h src/stages/filter_stage/filter_present_absent_stage.h src/stages/jwt_validation_stage/jwt_validation_stage.h)
target_link_libraries(core folly)

install(TARGETS core
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
        PATTERN "cmake*" EXCLUDE)




