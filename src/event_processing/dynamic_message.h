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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-17.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <typeindex>
#include <stdexcept>

#include "../foundation/extensions.h"
#include "message.h"

namespace adl::axp::core::event_processing {

// exceptions
class FieldGroupDoesNotExistException : public std::runtime_error {
 public:
  FieldGroupDoesNotExistException(std::string_view field_group) :
      std::runtime_error("Field group - " + std::string(field_group)
                             + " does not exist") { //TODO: avoid std::string creations. TODO: use a format library
  }
};

class FieldNameDoesNotExistException : public std::runtime_error {
 public:
  FieldNameDoesNotExistException(std::string_view field_group, std::string_view field_name) :
      std::runtime_error("Field by name - " + std::string(field_name)
                             + " does not exist inside tghe field group "
                             + std::string(field_group)) { //TODO: avoid std::string creations. TODO: use a format library
  }
};

class TypedGroupTypeMismatchException : public std::runtime_error {
 public:
  TypedGroupTypeMismatchException(std::string_view field_group, std::type_index requested_type, std::type_index type) :
      std::runtime_error("Field group by name " + std::string(field_group)
                             + " is of " + type.name() + "." + " Requested type is " + requested_type.name()) {
  }
};

/**
 * Represents a message having flexible set of fields
 * as field_name/field_value pairs
 * TODO: optimize setters and getters when key string is known at compile time.
 * For example using a int hashmap and calculating the hash at compile time.
 * Also improvements are possible by using multi index maps
 */

class DynamicMessage : public event_processing::IMessage {
  using variant_t = std::variant<int32_t, int64_t, std::string, void *>;

  template<typename value_t>
  class FieldGroupIterator : public std::iterator<
      std::input_iterator_tag,   // iterator_category
      std::pair<std::string, value_t>,                      // value_type
      int32_t,                      // difference_type TODO:
      const std::pair<std::string, value_t> *,               // pointer
      std::pair<std::string, value_t>                       // reference
  > {

   public:
    explicit FieldGroupIterator(const std::unordered_map<std::string, variant_t>::const_iterator &current) : _current(
        current) {
    }

    FieldGroupIterator &operator++() {
      _current++;
      return *this;
    }
    FieldGroupIterator operator++(int) {
      FieldGroupIterator ret_val = *this;
      ++(*this);
      return ret_val;
    }
    bool operator==(FieldGroupIterator other) const { return _current == other._current; }
    bool operator!=(FieldGroupIterator other) const { return !(*this == other); }
    typename FieldGroupIterator::reference operator*() const {
      return std::pair(_current->first,
                       std::get<value_t>(_current->second));
    }

   private:
    std::unordered_map<std::string, variant_t>::const_iterator _current;
  };

 public:
  virtual size_t message_type() const noexcept {
    return extensions::compile_time_hash("adl::axp::core::event_processing::DynamicMessage");
  }

  template<typename value_t>
  DynamicMessage &set_field(std::string_view group, std::string_view key, const value_t &value) {
    _field_values[std::string(group)].insert_or_assign(std::string(key), value); // overwrites existing if present
    return *this;
  }

  template<typename value_t>
  value_t get_field(std::string_view group, std::string_view name) const {

    auto ite = _field_values.find(group);
    if (ite != _field_values.end()) {
      auto ite2 = ite->second.find(name);
      if (ite2 == ite->second.end()) {
        throw FieldNameDoesNotExistException(group, name);
      } else {
        return std::get<value_t>(ite2->second);
      }
    } else {
      throw FieldGroupDoesNotExistException(group);
    }
  }

  template<typename value_t>
  bool create_typed_group(std::string_view group) {
    auto ite = _typed_field_groups.find(group);
    if (ite != _typed_field_groups.end()) {
      return false; // we already have a group by the name.
    } else {
      _typed_field_groups.template try_emplace(std::string(group), std::type_index(typeid(value_t)), 0);
      return true;
    }
  }

  template<typename value_t>
  void set_typed_field(std::string_view group, std::string_view field, const value_t &value) {
    auto ite = _typed_field_groups.find(group);
    if (ite == _typed_field_groups.end()) {
      throw FieldGroupDoesNotExistException(group);
    } else {
      if (ite->second.first == std::type_index(typeid(value_t))) {
        ite->second.second.template insert_or_assign(std::string(field), value);
      } else {
        throw TypedGroupTypeMismatchException(group, typeid(value_t), ite->second.first);
      }
    }
  }

  template<typename value_t>
  value_t get_typed_field(std::string_view group, std::string_view field) const {

    auto ite = _typed_field_groups.find(group);
    if (ite != _typed_field_groups.end()) {
      if (ite->second.first == std::type_index(typeid(value_t))) {
        const auto &ite2 = ite->second.second.find(field);
        if (ite2 == ite->second.second.end()) {
          throw FieldNameDoesNotExistException(group, field);
        } else {
          return std::get<value_t>(ite2->second);
        }
      } else {
        throw TypedGroupTypeMismatchException(group, typeid(value_t), ite->second.first);
      }
    } else {
      throw FieldGroupDoesNotExistException(group);
    }
  }

  template<typename value_t>
  std::pair<FieldGroupIterator<value_t>, FieldGroupIterator<value_t>> get_typed_group(std::string_view group) const {

    auto ite = _typed_field_groups.find(group);
    if (ite != _typed_field_groups.end()) {
      if (ite->second.first == std::type_index(typeid(value_t))) {
        return std::pair<FieldGroupIterator<value_t>, FieldGroupIterator<value_t>>(ite->second.second.begin(),
                                                                                   ite->second.second.end());
      } else {
        throw TypedGroupTypeMismatchException(group, typeid(value_t), ite->second.first);
      }
    } else {
      throw FieldGroupDoesNotExistException(group);
    }
  }

 private:
  std::unordered_map<std::string, std::unordered_map<std::string, variant_t, extensions::string_hash,
                                                     std::equal_to<>>, extensions::string_hash,
                     std::equal_to<>> _field_values;
  std::unordered_map<std::string,
                     std::pair<std::type_index,
                               std::unordered_map<std::string, variant_t, extensions::string_hash,
                                                  std::equal_to<>>
                     >,
                     extensions::string_hash,
                     std::equal_to<>> _typed_field_groups;
};
}
