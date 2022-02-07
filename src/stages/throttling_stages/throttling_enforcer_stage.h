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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-30.
 */

#pragma once

#include <mutex>
#include <list>

#include "../../foundation/extensions.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/base_impl/forwarding_sink.h"

#include "sliding_log_throttler.h"

namespace adl::axp::core::stages::throttling {

/**
 * Thread safe - no
 */
class ThrottlingEnforcerStage : public core::event_processing::details::BaseStage,
                                public core::event_processing::ISink {

 public:
  // TODO: use ranges for lookup_filed_names
  ThrottlingEnforcerStage(std::string_view name, std::list<std::string> lookup_filed_names);

  void on_message(core::event_processing::IMessage *message) noexcept override;
  void on_throttle_state_message(core::event_processing::IMessage *message) noexcept;

 private:
  event_processing::details::BaseSource _allowed_out;
  event_processing::details::BaseSource _throttled_out;
  event_processing::details::BaseSource _throttle_error_out;

  event_processing::base_impl::ForwardingSink _throttle_state_in;

  const std::list<std::string> _lookup_filed_names;

  // throttling state (allowed or supressed)
  std::unordered_map<std::string, bool> _throttler_states;

};

}
