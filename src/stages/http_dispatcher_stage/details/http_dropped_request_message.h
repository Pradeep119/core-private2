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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-04.
 */

#pragma once

#include <string>

#include "../../../event_processing/message.h"

namespace adl::axp::core::stages::details::http_dispatcher {

class HttpDroppedRequestMessage : public event_processing::IMessage {
 public:
  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::stages::base_impl::http_dispatcher::HttpDroppedRequestMessage");
  }

  event_processing::IMessage &get_dropped_message() noexcept {
    return *_dropped_message;
  }

  const event_processing::IMessage &get_dropped_message() const noexcept {
    return *_dropped_message;
  }

  void set_dropped_message(event_processing::IMessage &dropped_message) {
    _dropped_message = &dropped_message;
  }

 private:
  event_processing::IMessage *_dropped_message;
};

}
