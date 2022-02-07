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

#pragma once

#include <functional>
#include <list>

#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/base_impl/forwarding_sink.h"
#include "throttler.h"
#include "messages.h"

namespace adl::axp::core::stages::throttling {

// exceptions
class ThrottlerIdExistException : public std::runtime_error {
 public:
  ThrottlerIdExistException(std::string_view id) :
      std::runtime_error("Throttler with ID - " + std::string(id) + " already exists") {
  }
};

// exceptions
class ThrottlerDoesNotExistException : public std::runtime_error {
 public:
  ThrottlerDoesNotExistException(std::string_view id) :
      std::runtime_error("Throttler with ID - " + std::string(id) + " does not exist") {
  }
};

class InvalidThrottleTypeException : public std::runtime_error {
 public:
  InvalidThrottleTypeException(std::string_view id, std::string_view type) :
      std::runtime_error("Throttler with ID - " + std::string(id) + " is of an unknown type - "
                             + std::string(type)) { //TODO: WHY cant + take a string view
  }
};

/**
 * Thread safe - no
 */
class ThrottleStateBuilderStage : public core::event_processing::details::BaseStage {
 public:
  //TODO: state duration can vary based on the throttle params.
  ThrottleStateBuilderStage(std::string_view name,
                            const std::list<std::string> &lookup_filed_names,
                            std::string_view time_stamp_field_name,
                            int32_t flush_threshold);

  void on_message(event_processing::IMessage *message) noexcept;
  void on_flush_message(event_processing::IMessage *message) noexcept;
  void on_config_message(event_processing::IMessage *message) noexcept;

 protected:
  void on_add_throttler(event_processing::IMessage *message);
  void on_update_throttler(event_processing::IMessage *message);

 private:
  void flush_updates() noexcept;

  const std::string _time_stamp_field_name;
  const std::list<std::string> _lookup_filed_names;
  const int32_t _flush_threshold;

  std::unordered_map<std::string, IThrottler *> _throttlers;

  event_processing::details::BaseSource _state_out;
  event_processing::details::BaseSource _error_out;
  event_processing::details::BaseSource _config_error_out;

  event_processing::base_impl::ForwardingSink _message_in;
  event_processing::base_impl::ForwardingSink _flush_in;
  event_processing::base_impl::ForwardingSink _config_in;

  std::list<std::pair<std::string, bool>> _updated_throttlers;

};

}