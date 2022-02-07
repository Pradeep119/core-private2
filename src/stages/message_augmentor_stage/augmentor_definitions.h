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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-11-10.
 */

#pragma once

#include <ranges>
#include <string>
#include <utility>
#include <sstream>

#include "message_augmentor.h"

namespace adl::axp::core::stages::message_augmentor {

/**
 * concats a given set of string fields
 */
struct ConcatenatingAugmentorDefs {

  struct Config {
    std::list<std::pair<std::string, std::string>> field_names;
    std::string separator;
    std::string prefix;
    std::string postfix;
    std::string target_field_group;
    std::string target_field_name;
  };

  static constexpr auto augment_operator = [](const Config &configuration,
                                              event_processing::DynamicMessage *message) {
    std::stringstream ss;
    ss << configuration.prefix;
    for (const auto &field : configuration.field_names) {
      //TODO: use a faster library for this
      ss << configuration.separator
         << message->get_field<std::string>(field.first, field.second);
    }
    if (!configuration.postfix.empty()) {
      ss << configuration.separator;
    }
    ss << configuration.postfix;
    message->set_field<std::string>(configuration.target_field_group, configuration.target_field_name, ss.str());
  };
};

/**
 * Parses a given field value to extract different tokens wrapped inside
 * given markers. eg:
 * /test/[token 1]/testing/[token 2]/[token 3]/bar
 * Here starting marker is [ and ending marker is ]
 * This augmentor will extract token 1, token 2, token 3
 * in that order.
 */
struct TokenExtractingAugmentorDefs {
  struct Config {
    std::pair<std::string, std::string> field;
    char token_start; // starting str/char that wrap a token
    char token_end; // ending str/char that wrap a token
    int n_tokens; // number of tokens to extract while parsing from left to right
    std::list<std::pair<std::string, std::string>>
        target_fields; // target field group/names that must be used to store extracted tokens. This order must be based token extraction order from left to right
  };

  // TODO: This implementation will have to be modified to cater for various edge cases.
  // Also, if it gets complicated, advisable to use a grammar parser like boost Spirit
  // https://www.boost.org/doc/libs/1_67_0/libs/spirit/doc/html/spirit/introduction.html
  static constexpr auto augment_operator = [](const Config &configuration,
                                              event_processing::DynamicMessage *message) {

    const auto &field_value = message->get_field<std::string>(configuration.field.first, configuration.field.second);

    auto ite = configuration.target_fields.begin();

    bool token_in_progress = false;
    std::stringstream current_token;
    uint8_t n_extracted_tokens = 0;

    for (const auto c : field_value) {
      if (token_in_progress) {
        if (c == configuration.token_start) {
          throw std::runtime_error("Malformed"); //TODO: new exception type
        } else if (c == configuration.token_end) {
          token_in_progress = false;
          message->set_field((*ite).first, (*ite).second, current_token.str());
          current_token.str("");
          if (++n_extracted_tokens == configuration.n_tokens) {
            break;
          }
          ++ite;
        } else {
          current_token << c;
        }
      } else {
        if (c == configuration.token_end) {
          throw std::runtime_error("Malformed"); //TODO: new exception type
        } else if (c == configuration.token_start) {
          token_in_progress = true;
        }
      }
    }
  }; // operator
}; // struct TokenExtractingAugmentorDefs
}