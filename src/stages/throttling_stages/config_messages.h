/*
 *
 *   © Copyrights 2021 Axiata Digital Labs Pvt Ltd.
 *   All Rights Reserved.
 *
 *   These material are unpublished, proprietary, confidential source
 *   code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
 *   SECRET of ADL.
 *
 *   ADL retains all title to and intellectual property rights in these
 *   materials.
 *
 *
 *   @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on ${date}.
 *
 */

#pragma once

#include "../../event_processing/messages/async_config_message.h"

namespace adl::axp::core::stages::throttling {

class AddThrottlerConfigMessage : public event_processing::messages::AsyncConfigurationMessage<void> {
 public:
  AddThrottlerConfigMessage(std::string_view id,
                            std::string_view type,
                            bool queue_enable,
                            size_t time_window,
                            size_t allowed_count) :
      _id(id),
      _type(type),
      _queue_enable(queue_enable),
      _time_window(time_window),
      _allowed_count(allowed_count) {
  };

  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::stages::throttling::AddThrottlerConfigMessage");
  }

  std::string_view get_id() const noexcept { return _id; }
  std::string_view get_type() const noexcept { return _type; }
  bool get_queue_enable() const noexcept { return _queue_enable; }
  size_t get_time_window() const noexcept { return _time_window; }
  size_t get_allowed_count() const noexcept { return _allowed_count; }

 private:
  const std::string _id;
  const std::string _type;
  const bool _queue_enable;
  const size_t _time_window;
  const size_t _allowed_count;
};

class UpdateThrottlerConfigMessage : public AddThrottlerConfigMessage {
 public:
  UpdateThrottlerConfigMessage(std::string_view id,
                               std::string_view type,
                               bool queue_enable,
                               size_t time_window,
                               size_t allowed_count) :
      AddThrottlerConfigMessage(id, type, queue_enable, time_window, allowed_count) {}

  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::stages::throttling::UpdateThrottlerConfigMessage");
  }
};

}