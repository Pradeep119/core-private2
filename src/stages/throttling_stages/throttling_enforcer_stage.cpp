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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-30.
 */

#include <iostream>
#include <cassert>

#include "throttling_enforcer_stage.h"
#include "../../event_processing/dynamic_message.h"
#include "messages.h"

namespace adl::axp::core::stages::throttling {

ThrottlingEnforcerStage::ThrottlingEnforcerStage(std::string_view name, std::list<std::string> lookup_filed_names) :
    core::event_processing::details::BaseStage(name),
    _lookup_filed_names(lookup_filed_names),
    _throttle_state_in(std::bind(&ThrottlingEnforcerStage::on_throttle_state_message, this, std::placeholders::_1)) {

  register_source("allowed_out", &_allowed_out);
  register_source("throttled_out", &_throttled_out);
  register_source("throttle_error_out", &_throttle_error_out);

  register_sink("throttle_state_in", &_throttle_state_in);
  register_sink("message_in", this);
}

void ThrottlingEnforcerStage::on_message(core::event_processing::IMessage *message) noexcept {
  // lookup throttlers by field values
  // apply throttling sequentially
  // if all throttlers allow the message, forward it
  // otherwise send it through throttled out

  auto *dynamic_message = static_cast<event_processing::DynamicMessage *>(message);
  for (const auto &field_name: _lookup_filed_names) {
    const auto &throttler_id =
        dynamic_message->get_field<std::string>("http_info", field_name); //TODO: get the field group injected
    // TODO: we may have to use a postfix
    auto ite = _throttler_states.find(throttler_id);
    if (ite != _throttler_states.end()) {
      if (false == ite->second) {
        // this throttler does not allow the message.
        // so throttler and break the loop
        dynamic_message->set_field<std::string>("throttling", "state", "suppressed");
        _throttled_out.get_sink()->on_message(message);
        return;
      }
    } else {
      // No throttler state yet.
      // just allow
    }
  }

  // all throttlers have allowed
  dynamic_message->set_field<std::string>("throttling", "state", "allowed");
  _allowed_out.get_sink()->on_message(message);
}

void ThrottlingEnforcerStage::on_throttle_state_message(event_processing::IMessage *message) noexcept {

  const auto *throttler_state_message = static_cast<ThrottleStateMessage *>(message);
  for (const auto &state: throttler_state_message->get_state()) {
    const auto&[throttler_id, throttler_state] = state;
    _throttler_states[throttler_id] = throttler_state;
  }
}

}