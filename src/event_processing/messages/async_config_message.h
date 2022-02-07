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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-11-05.
 */


#pragma once

#include <string>
#include <future>

#include "../../foundation/extensions.h"
#include "../message.h"

namespace adl::axp::core::event_processing::messages {

/** helper class for implementation, not to be used outside
 *
 */
template<typename config_result_t>
class __async_config_base__message__ : public event_processing::IMessage {
 public:
  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::event_processing::messages::AsyncConfigurationMessage");
  }

  void on_error(const std::exception_ptr ex) noexcept {
    _promise.set_exception(ex);
  }

  std::future<config_result_t> get_future() noexcept {
    return _promise.get_future();
  }

 protected:
  std::promise<config_result_t> _promise;
};

/**
 * All configuration messages should derive from this class.
 * This class provide facilities to send a configuration message
 * to a stage/sink synchrnously or asynchrnously and wait for
 * the completion of processing the same using a std::future
 *
 * @tparam config_result_t result type expected from the configuration message
 */
template<typename config_result_t>
class AsyncConfigurationMessage : public __async_config_base__message__<config_result_t> {
 public:
  void on_complete(config_result_t &result) noexcept {
    this->_promise.set_value(result);
  }
};

/**
 * Specialization of above for void return type case
 */
template<>
class AsyncConfigurationMessage<void> : public __async_config_base__message__<void> {
 public:
  void on_complete() noexcept {
    _promise.set_value();
  }
};

}