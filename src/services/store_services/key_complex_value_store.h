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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-22.
 */

#pragma once

#include <exception>
#include <string>
#include <unordered_map>
#include <stdexcept>

#include "../../foundation/extensions.h"

namespace adl::axp::core::services::store_services {

// thread safe - no
template<class complex_value_t>
class KeyComplexValueStore {
 public:
  void add_value(std::string_view key, complex_value_t *value) {
    _key_complex_values[std::string(key)] = value;
  }
  complex_value_t *get_value(std::string_view key) const {
    auto ite = _key_complex_values.find(key);
    if (ite != _key_complex_values.end()) {
      return ite->second;
    } else {
      throw std::runtime_error(std::string("no key in complex store ") + std::string(key)); //TODO:
    }
  }
  bool contains(std::string_view key) const {
    return _key_complex_values.contains(key);
  }

 private:
  std::unordered_map<std::string, complex_value_t *, extensions::string_hash,
                     std::equal_to<>> _key_complex_values;
};

}