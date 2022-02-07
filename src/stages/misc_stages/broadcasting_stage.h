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

#include <ranges>

#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"

namespace adl::axp::core::stages::misc {
template<size_t output_sink_count>
class BroadcastingState : public core::event_processing::details::BaseStage,
                          public core::event_processing::ISink {
 public:
  BroadcastingState(std::string_view name) :
      core::event_processing::details::BaseStage(name) {

    for (int i = 0; i < output_sink_count; ++i) {
      register_source("out-" + std::to_string(i), &_outputs[i]);
    }

    register_sink("in", this);
  }

  void on_message(core::event_processing::IMessage *message) noexcept override {
    std::ranges::for_each(_outputs, [message](auto &source) { source.get_sink()->on_message(message); });
  }

 private:
  core::event_processing::details::BaseSource _outputs[output_sink_count];
};
}

