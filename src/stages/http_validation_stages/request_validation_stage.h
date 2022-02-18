/*
 * © Copyrights 2022 Axiata Digital Labs Pvt Ltd.
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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-21.
 * @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on 2022-01-20.
 */

#pragma once

#include <iostream>
#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"

#include "../../event_processing/messages/http_request_message.h"
#include "request_validation.h"
#include "../../services/store_services/key_complex_value_store.h"
#include "../../services/store_services/key_value_store.h"

namespace adl::axp::core::stages::http::validation {

class HttpRequestValidationStage : public event_processing::details::BaseStage,
                                   public event_processing::ISink {

 public:
  using field_combination_t = std::pair<std::string, std::string>;
  using validation_spec_store = services::store_services::KeyComplexValueStore<validation::RequestValidator>;

  HttpRequestValidationStage(std::string_view name,
                             const validation_spec_store &store,
                             const field_combination_t &validator_key_combinations,
                             std::string_view header_combinations,
                             std::string_view path_combinations,
                             std::string_view query_combinations);

  void on_message(event_processing::IMessage *message) noexcept override;

 private:

  const services::store_services::KeyComplexValueStore<RequestValidator> &_store;

  const field_combination_t _validator_key_combinations;
  const std::string _header_combinations;
  const std::string _path_combinations;
  const std::string _query_combinations;

  // sources/sinks
  event_processing::details::BaseSource _valid_requests_out;
  event_processing::details::BaseSource _invalid_requests_out;
  std::shared_ptr<spdlog::logger> _logger;

};
}

