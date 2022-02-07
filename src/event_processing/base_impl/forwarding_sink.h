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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-01.
 */

#pragma once

#include <functional>

namespace adl::axp::core::event_processing::base_impl {

/**
 * This sink can be used to implement multiple sinks in a single stage
 */
class ForwardingSink : public event_processing::ISink {
 public:
  explicit ForwardingSink(const std::function<void(event_processing::IMessage *)> &forward_fun) :
      _forward_fun(forward_fun) {}

  void on_message(event_processing::IMessage *message) noexcept override {
    _forward_fun(message);
  }

 private:
  std::function<void(event_processing::IMessage *)> _forward_fun;
};

}