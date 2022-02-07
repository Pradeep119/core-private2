#include "./mpmc_queue.h"

namespace adl::axp::core::stages {

MPMCQueue::MPMCQueue(std::string_view name, const uint32_t capacity) :
    event_processing::details::BaseStage(name),
    _message_queue(capacity) {

  register_source("message_out", &_message_out);
  register_source("buffer_full_out", &_buffer_full_out);
  register_sink("message_in", this);
}

void MPMCQueue::on_message(event_processing::IMessage *message) noexcept {
  const auto ret = _message_queue.write(message);
  if (!ret) {
    _buffer_full_out.get_sink()->on_message(message);
  }
}

int MPMCQueue::run(void *data) noexcept {
  event_processing::IMessage *message = nullptr;
  while (!_stop) {
    if (_message_queue.read(message)) {
      _message_out.get_sink()->on_message(message);
    } else {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(100us); //TODO:
    }
  }
  return 0; //TODO: get return value if any
}
int MPMCQueue::stop() noexcept {
  _stop = true;
  return 0;
}

}