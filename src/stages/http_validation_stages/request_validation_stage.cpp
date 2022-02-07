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


#include <iostream>
#include "request_validation_stage.h"

namespace adl::axp::core::stages::http::validation {

namespace {
bool evaluate_validity(const auto &http_req_message,
                       const auto &validator,
                       const auto &header_typed_group_name,
                       const auto &path_typed_group_name,
                       const auto &query_typed_group_name) {

  const auto &header_validator = validator->_header_validator;
  const auto[start_h, end_h] = http_req_message->template get_typed_group<std::string>(header_typed_group_name);

  const auto &path_validator = validator->_path_validator;
  const auto[start_p, end_p] = http_req_message->template get_typed_group<std::string>(path_typed_group_name);

  const auto &query_validator = validator->_query_validator;
  const auto[start_q, end_q] = http_req_message->template get_typed_group<std::string>(query_typed_group_name);

  if (!header_validator.validate(start_h, end_h)) {
    return false;
  } else if (!path_validator.validate(start_p, end_p)) {
    return false;
  } else if (!query_validator.validate(start_q, end_q)) {
    return false;
  } else {
    return true;
  }
}

validation::RequestValidator *get_validation_spec(event_processing::IMessage *message,
                                                  const services::store_services::KeyComplexValueStore<RequestValidator> &_store,
                                                  std::string_view lookup_field_group,
                                                  std::string_view lookup_field_name) {
  try {
    const auto &dynamic_message = static_cast<event_processing::DynamicMessage *>(message);
    std::string spec_key = dynamic_message->get_field<std::string>(lookup_field_group, lookup_field_name);
    return _store.contains(spec_key) ? _store.get_value(spec_key) : nullptr;

  } catch (const event_processing::FieldGroupDoesNotExistException &ex) {
    throw ex; // TODO
  } catch (const event_processing::FieldNameDoesNotExistException &ex) {
    throw ex; // TODO
  } catch (const std::runtime_error &ex) {
    throw ex; // TODO
  }
}

}

HttpRequestValidationStage::HttpRequestValidationStage(std::string_view name,
                                                       const validation_spec_store &store,
                                                       const field_combination_t &validator_key_combinations,
                                                       std::string_view header_combinations,
                                                       std::string_view path_combinations,
                                                       std::string_view query_combinations) :
    event_processing::details::BaseStage(name),
    _store(store),
    _validator_key_combinations(validator_key_combinations),
    _header_combinations(std::string(header_combinations)),
    _path_combinations(std::string(path_combinations)),
    _query_combinations(std::string(query_combinations)) {

  register_source("valid_requests_out", &_valid_requests_out);
  register_source("invalid_requests_out", &_invalid_requests_out);
  register_sink("in", this);
}

void HttpRequestValidationStage::on_message(event_processing::IMessage *message) noexcept {

  auto *http_req_message = static_cast<core::event_processing::messages::HttpRequestMessage *>(message);
  const auto &validator_lookup_group = _validator_key_combinations.first;
  const auto &validator_lookup_field = _validator_key_combinations.second;
  auto validator = get_validation_spec(http_req_message,_store,validator_lookup_group, validator_lookup_field);

  const auto &header_typed_group_name = _header_combinations;
  const auto &path_typed_group_name = _path_combinations;
  const auto &query_typed_group_name = _query_combinations;
  bool validity = true;
  if (validator != nullptr) {
    validity = evaluate_validity(http_req_message,
                                 validator,
                                 header_typed_group_name,
                                 path_typed_group_name,
                                 query_typed_group_name);
  }

  validity ? std::cout << "Http validation Success" << std::endl : std::cout << "Http validation Failed" << std::endl;
  validity ? _valid_requests_out.get_sink()->on_message(message) : _invalid_requests_out.get_sink()->on_message(
      message);
}

}
