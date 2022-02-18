/*
 *
 *   © Copyrights 2021 Axiata Digital Labs Pvt Ltd.
 *   All Rights Reserved.
 *
 *   These material are unpublished, proprietary, confidential source
 *   code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
 *   SECRET of ADL.
 *
 *   ADL retains all title to and intellectual property rights in these
 *   materials.
 *
 *
 *   @author Braveenan Sritharan (braveenan.sritharan@axiatadigitallabs.com) on 2021/12/23.
 *
 */

#pragma once

#include <JWT/jwt.h>

#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"
#include "../../event_processing/messages/http_request_message.h"
#include "../../services/store_services/key_value_store.h"

namespace adl::axp::core::stages::jwt_token {

class JwtValidationStage : public event_processing::ISink, public event_processing::details::BaseStage {
 public:
  using field_combination_t = std::pair<std::string, std::string>;

  JwtValidationStage(std::string_view name,
                     const field_combination_t &token_field,
                     std::string_view decoded_group,
                     std::string_view rsa_pub_key)
      : event_processing::details::BaseStage(name),
        _token_field(token_field),
        _decoded_group(decoded_group),
        _rsa_public_key(rsa_pub_key) {
    register_source("valid_out", &_valid_out);
    register_source("invalid_out", &_invalid_out);
    register_source("expired_out", &_expired_out);
    register_source("non_jwt_out", &_non_jwt_out);
    register_sink("message_in", this);
    _logger = spdlog::get(std::string(name));
  }

  void on_message(event_processing::IMessage *message) noexcept override {
    auto *dynamic_message = static_cast<event_processing::DynamicMessage *>(message);

    const auto &access_token = dynamic_message->template get_field<std::string>(
        _token_field.first, _token_field.second);

    try {
      auto jwt_verifier = jwt::verify().allow_algorithm(jwt::algorithm::rs256(_rsa_public_key, "", "", ""));
      auto jwt_decoded = jwt::decode(access_token);

      jwt_verifier.verify(jwt_decoded);

      for (const auto &e2 : jwt_decoded.get_header_claims()) {
        dynamic_message->set_field<std::string>(_decoded_group, e2.first, e2.second.as_string());
      }
      for (const auto &e1 : jwt_decoded.get_payload_claims()) {
        dynamic_message->set_field<std::string>(_decoded_group, e1.first, e1.second.to_json().to_str());
      }
      _valid_out.get_sink()->on_message(message);
    }
    catch (jwt::error::token_verification_exception e) {
      _logger->error("Expired JWT access token");
      _expired_out.get_sink()->on_message(message);
    }
    catch (std::invalid_argument e) {
      _logger->error("Non JWT access token");
      _non_jwt_out.get_sink()->on_message(message);
    }
    catch (jwt::error::signature_verification_exception e) {
      _logger->error("Invalid JWT access token");
      _invalid_out.get_sink()->on_message(message);
    }
    catch (std::runtime_error e) {
      _logger->error("Invalid JWT access token");
      _invalid_out.get_sink()->on_message(message);
    }
  }

 private:
  event_processing::details::BaseSource _valid_out;
  event_processing::details::BaseSource _invalid_out;
  event_processing::details::BaseSource _expired_out;
  event_processing::details::BaseSource _non_jwt_out;

  const field_combination_t _token_field;
  const std::string _decoded_group;
  const std::string _rsa_public_key;

  std::shared_ptr<spdlog::logger> _logger;
};

}
