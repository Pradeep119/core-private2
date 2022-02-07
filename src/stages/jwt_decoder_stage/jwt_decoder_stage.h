
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
 *   @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on 2021/09/29.
 *
 */

#pragma once

#include <JWT/jwt.h>
#include <picojson/picojson.h>

#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl//base_source.h"
#include "../../event_processing/message.h"

namespace adl::axp::core::stages::Jwt {

// Thread safe - no
// get field group name as a config
class JwtDecoderStage : public event_processing::ISink, public event_processing::details::BaseStage {
 public:

  JwtDecoderStage(std::string_view name, std::string_view token_field_name) : event_processing::details::BaseStage(
      name), _token_field_name(token_field_name) {
    register_source("decoded_token_out", &_decoded_token_out);
    register_source("error_decoding_out", &_error_decoding_out);
    register_sink("message_in", this);
  }

  void on_message(
      event_processing::IMessage *message) noexcept {
    auto *dynamic_message = static_cast<event_processing::DynamicMessage *>(message);

    try {
      const auto
          &access_token = dynamic_message->get_field<std::string>("auth", _token_field_name); //TODO: get this injected
      const auto &decoded_token = jwt::decode(access_token);

      /**This block will pass all the claims to the message
       * claims are not fixed though
       * -- In that case we may need to end up with fixed policy or
       * couple of alternative policies to do the filtering*/
      for (const auto &e2: decoded_token.get_header_claims()) {
        dynamic_message->set_field<std::string>("jwt decoding", e2.first, e2.second.as_string());
      }
      for (const auto &e1: decoded_token.get_payload_claims()) {
        dynamic_message->set_field<std::string>("jwt decoding", e1.first, e1.second.to_json().to_str());
        //Here some claims are having different types like time ; Tried different
        // things.   Only this worked. Actually otherwise it is throwing an exception due to conversion error.
      }
      dynamic_message->set_field<std::string>("jwt decoding", "header_base64", decoded_token.get_header_base64());
      dynamic_message->set_field<std::string>("jwt decoding", "payload_base64", decoded_token.get_payload_base64());
      dynamic_message->set_field<std::string>("jwt decoding", "signature_base64", decoded_token.get_signature_base64());

      _decoded_token_out.get_sink()->on_message(dynamic_message);

    }
    catch (const std::runtime_error &e) {
      dynamic_message->set_field<std::string>("jwt decoding", "jwt_decoder_error_description", e.what());
      _error_decoding_out.get_sink()->on_message(dynamic_message);
    }
    catch (const std::exception &e) {
      dynamic_message->set_field<std::string>("jwt decoding", "jwt_decoder_error_description", e.what());
      _error_decoding_out.get_sink()->on_message(dynamic_message);
    }
  }

 private:
  event_processing::details::BaseSource _decoded_token_out;
  event_processing::details::BaseSource _error_decoding_out;
  const std::string _token_field_name;
};
}