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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-28.
*/

#pragma once

#include "../../event_processing/sink.h"

namespace adl::axp::core::stages::load_balancers {

/**
 * This stage load balances incoming messages with a given number of sources.
 * Thread safe - yes
 */
template<int outbound_count>
class InProcessRoundRobinLoadBalancerStage : public core::event_processing::details::BaseStage,
                                             public core::event_processing::ISink {
 public:
  InProcessRoundRobinLoadBalancerStage(std::string_view name) : core::event_processing::details::BaseStage(name) {
    register_sink("message_in", this);
    for (int i = 0; i < outbound_count; ++i) {
      auto *source = new core::event_processing::details::BaseSource();
      _sources[i] = source;
      register_source(std::string("message_out_") + std::to_string(i), source);
    }
  }
  virtual void on_message(core::event_processing::IMessage *message) noexcept {
    auto *target_source = _sources[_index.load()];
    auto previous_value = _index.fetch_add(1, std::memory_order_relaxed);
    if (previous_value == outbound_count - 1) {
      _index.store(0);
    }

    target_source->get_sink()->on_message(message);
  }

 private:
  std::atomic_int16_t _index = {0};
  std::array<core::event_processing::details::BaseSource *, outbound_count> _sources;
};

}