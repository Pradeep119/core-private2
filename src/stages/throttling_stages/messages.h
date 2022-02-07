/*
 * © Copyrights 2021 Axiata Digital Labs Pvt Ltd.
 * All Rights Reserved.
 *
 * These material are unpublished, proprietary, confidential source
 * code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
 * SECRET of ADL.
 *
 * ADL retains all title to and intellectual property rights in these
 * materials.
 *
 *
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-15.
 */

#pragma once

#include <list>
#include <utility>

namespace adl::axp::core::stages::throttling {

class ThrottleStateMessage : public event_processing::IMessage {
  using throttler_state_t = std::list<std::pair<std::string, bool>>;

 public:
  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::stages::throttling::ThrottleStateMessage");
  }

  const throttler_state_t &get_state() const noexcept {
    return _throttler_states;
  }

  void add_throttler_state(std::string_view id, bool state) {
    _throttler_states.emplace_back(std::string(id), state);
  }

 private:
  throttler_state_t _throttler_states;
};
}