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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-24.
*/

#pragma once

#include <thread>
#include <chrono>

#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"

/**
 * Introduces a delay from in to out.
 */
namespace adl::axp::core::stages::misc {

class DelayStage : public core::event_processing::details::BaseStage,
                   public core::event_processing::ISink {
 public:
  DelayStage(std::string_view name, int64_t delay_us) :
      core::event_processing::details::BaseStage(name),
      _delay_us(delay_us) {
    register_source("out", &_out);
    register_sink("in", this);
  }

  virtual void on_message(core::event_processing::IMessage *message) noexcept {
    //TODO: optimize. should not be creating threads.
    auto *delay_thread = new std::thread([this, message]() {
      std::this_thread::sleep_for(std::chrono::microseconds(_delay_us));
      _out.get_sink()->on_message(message);
    });
  }

 private:
  const int64_t _delay_us;
  core::event_processing::details::BaseSource _out;
};
}