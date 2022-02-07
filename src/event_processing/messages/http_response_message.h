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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-03.
 */


#pragma once

#include <string>

#include "../../foundation/extensions.h"
#include "../message.h"

namespace adl::axp::core::event_processing::messages {

class HttpRequestMessage;

/**
 * Represents an http(s) response
 */
class HttpResponseMessage : public event_processing::DynamicMessage {
 public:
  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::event_processing::base_impl::HttpResponseMessage");
  }

  std::string_view get_body() const noexcept { return _body; };
  void set_body(std::string_view body) {
    _body = std::string(body);
  }
  const HttpRequestMessage &get_request_message() const {
    return *_request_message;
  }
  HttpRequestMessage &get_request_message() {
    return *_request_message;
  }
  void set_request_message(HttpRequestMessage &request_message) {
    _request_message = &request_message;
  }
  void set_return_code(int32_t code) {
    _return_code = code;
  }
  int32_t get_return_code() const {
    return _return_code;
  }

 private:
  int32_t _return_code;
  std::string _body; //TODO: handle other content types
  HttpRequestMessage *_request_message;
};

}
