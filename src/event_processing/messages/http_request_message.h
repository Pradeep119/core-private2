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

#include "../dynamic_message.h"

namespace adl::axp::core::event_processing::messages {

/**
 * Represents an http(s) request
 * It uses uWS httpRequest class to hold all the data, but exposes
 * contents in neutral way to avoid coupling with uWS by the code
 * that use this class.
 */
class HttpRequestMessage : public event_processing::DynamicMessage {
 public:
  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::event_processing::base_impl::HttpRequestMessage");
  }

  std::string_view get_body() const { return _body; }
  void set_body(std::string_view body) { _body = body; }

 private:
  std::string _body;
};

}


