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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-10-15.
 */

#include <iostream>
#include <cassert>

#include "../../event_processing/dynamic_message.h"
#include "throttle_state_builder_stage.h"
#include "messages.h"
#include "config_messages.h"
#include "sliding_log_throttler.h"

namespace adl::axp::core::stages::throttling {

namespace {

event_processing::IMessage *to_message(std::string_view throttle_id, const std::list<size_t> *state) {
  auto *message = new ThrottleStateMessage();
  message->add_throttler_state(throttle_id, state);
  return message;
}

}

ThrottleStateBuilderStage::ThrottleStateBuilderStage(std::string_view name,
                                                     const std::list<std::string> &lookup_filed_names,
                                                     std::string_view time_stamp_field_name,
                                                     int32_t flush_threshold) :
    core::event_processing::details::BaseStage(name),
    _lookup_filed_names(lookup_filed_names),
    _time_stamp_field_name(time_stamp_field_name),
    _flush_threshold(flush_threshold),
    _message_in(std::bind(&ThrottleStateBuilderStage::on_message, this, std::placeholders::_1)),
    _flush_in(std::bind(&ThrottleStateBuilderStage::on_flush_message, this, std::placeholders::_1)),
    _config_in(std::bind(&ThrottleStateBuilderStage::on_config_message, this, std::placeholders::_1)) {

  register_source("state_out", &_state_out);
  register_source("error_out", &_error_out);
  register_source("config_error_out", &_config_error_out);
  register_sink("message_in", &_message_in);
  register_sink("flush_in", &_flush_in);
  register_sink("config_in", &_config_in);

}

void ThrottleStateBuilderStage::on_message(event_processing::IMessage *message) noexcept {

  auto *dynamic_message = static_cast<event_processing::DynamicMessage *>(message);
  auto request_time_stamp =
      dynamic_message->get_field<int64_t>("info", _time_stamp_field_name); // TODO: get the group also injected

  // shows whether the message was throttled or not
  const auto &throttle_state = dynamic_message->get_field<std::string>("throttling", "state");

  for (const auto &field_name: _lookup_filed_names) {
    try {
      const auto &throttler_id =
          dynamic_message->get_field<std::string>("http_info", field_name); //TODO: get the field group injected

      IThrottler *throttler = nullptr;

      auto ite = _throttlers.find(throttler_id);
      if (ite == _throttlers.end()) {
        // TODO: set fields
        _error_out.get_sink()->on_message(message);
      } else {
        if (throttle_state == "allowed") {
          ite->second->add_timestamp(request_time_stamp);
        } else {
          ite->second->re_evaluate(request_time_stamp);
        }
        _updated_throttlers.emplace_back(throttler_id, ite->second->is_allowed());
      }
    } catch (const event_processing::FieldNameDoesNotExistException &ex) {
      //TODO:
      std::cerr << "no field by name " << field_name << std::endl;
      _error_out.get_sink()->on_message(message);
    }
  }
}

void ThrottleStateBuilderStage::flush_updates() noexcept {
  auto *throttle_state_update_message = new ThrottleStateMessage();
  for (const auto &throttler: _updated_throttlers) {
    throttle_state_update_message->add_throttler_state(throttler.first, throttler.second);
  }
  _updated_throttlers.clear();
  _state_out.get_sink()->on_message(throttle_state_update_message); //TODO: delete this
}

void ThrottleStateBuilderStage::on_flush_message(event_processing::IMessage *message) noexcept {
  flush_updates();
}

void ThrottleStateBuilderStage::on_config_message(event_processing::IMessage *message) noexcept {
  auto *throttler_config_message = static_cast<AddThrottlerConfigMessage *>(message);

  try {
    if (extensions::compile_time_hash("adl::axp::core::stages::throttling::AddThrottlerConfigMessage")
        == throttler_config_message->message_type()) {
      on_add_throttler(message);
      throttler_config_message->on_complete();

    } else if (extensions::compile_time_hash("adl::axp::core::stages::throttling::UpdateThrottlerConfigMessage")
        == throttler_config_message->message_type()) {
      on_update_throttler(message);
      throttler_config_message->on_complete();

    } else {
      throttler_config_message->on_error(std::make_exception_ptr(std::runtime_error("Unknown message type")));
    }
  } catch (const std::exception &ex) {
    throttler_config_message->on_error(std::current_exception());
  } catch (...) {
    throttler_config_message->on_error(std::make_exception_ptr(std::runtime_error("Unknown error occurred")));
  }
}

void ThrottleStateBuilderStage::on_add_throttler(event_processing::IMessage *message) {
  auto *add_throttler_message = static_cast<adl::axp::core::stages::throttling::AddThrottlerConfigMessage *>(message);

  if (add_throttler_message->get_type() == "sliding_log") {

    auto *throttler =
        new SlidingLogThrottler(add_throttler_message->get_time_window(), add_throttler_message->get_allowed_count());

    auto[_, success] = _throttlers.try_emplace(std::string(add_throttler_message->get_id()), throttler);
    if (!success) {
      throw ThrottlerIdExistException(add_throttler_message->get_id());
    }
  } else if (add_throttler_message->get_type() == "fixed_window") {
    throw InvalidThrottleTypeException(add_throttler_message->get_id(), add_throttler_message->get_type());
  }
}

void ThrottleStateBuilderStage::on_update_throttler(event_processing::IMessage *message) {
  auto
      *add_throttler_message = static_cast<adl::axp::core::stages::throttling::UpdateThrottlerConfigMessage *>(message);

  if (add_throttler_message->get_type() == "sliding_log") {

    auto ite = _throttlers.find(std::string(add_throttler_message->get_id()));
    if (ite == _throttlers.end()) {
      throw ThrottlerDoesNotExistException(add_throttler_message->get_id());
    } else {
      auto *specific_throttler = dynamic_cast<SlidingLogThrottler *>(ite->second);
      if (nullptr != specific_throttler) {

        specific_throttler->set_allowed_request_count(add_throttler_message->get_allowed_count());
        specific_throttler->set_window_duration(add_throttler_message->get_time_window());

      } else {
        throw InvalidThrottleTypeException(add_throttler_message->get_id(), add_throttler_message->get_type());
      }
    }
  } else if (add_throttler_message->get_type() == "sliding_window") {
    throw InvalidThrottleTypeException(add_throttler_message->get_id(), add_throttler_message->get_type());
  } else if (add_throttler_message->get_type() == "fixed_window") {
    throw InvalidThrottleTypeException(add_throttler_message->get_id(), add_throttler_message->get_type());
  }
}

}
