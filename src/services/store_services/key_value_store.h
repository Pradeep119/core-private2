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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-13.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <typeindex>
#include <unordered_set>

#include "../../foundation/extensions.h"

namespace adl::axp::core::services::store_services {

/**
 * This is a mon thread safe key value store which can be used to store application state in
 * memory. Rather than supporting any arbitrary data type and structure, we
 * want to keep this simple by giving an interface that only support limited
 * number of data structures and data types. Although this reduces flexibility,
 * this avoids abuse of the store to store 'almost anything' from primitives to TCP connection
 * pointers. Intended purpose of this store is to store 'data', not arbitrary information.
 *
 * Following rules apply to all interface methods
 * 1. Any data that is passed to the store moved
 * 2.
 */

// exceptions
class KeyDoesNotExistException : public std::runtime_error {
 public:
  KeyDoesNotExistException(std::string_view key) :
      std::runtime_error("Key [" + std::string(key)
                             + "] does not exist") { //TODO: avoid std::string creations. TODO: use a format library
  }
};

class InvalidListElementTypeException : public std::runtime_error {
 public:
  InvalidListElementTypeException(std::string_view key,
                                  const std::type_index &existing_type,
                                  const std::type_index &type) :
      std::runtime_error(
          std::string("Type of existing elements [") + existing_type.name() + "] does not match the type ["
              + type.name() + "]") {
  }
};

class InvalidSetElementTypeException : public std::runtime_error {
 public:
  InvalidSetElementTypeException(std::string_view key,
                                 const std::type_index &existing_type,
                                 const std::type_index &type) :
      std::runtime_error(
          std::string("Type of existing elements [") + existing_type.name() +
              "] does not match the type ["
              + type.name() + "]") {
  }
};

class TypeMismatchException : public std::runtime_error {
 public:
  TypeMismatchException(std::string_view key, std::type_index expected_type) :
      std::runtime_error("Type of the value of the given key [" + std::string(key) +
          "] is different from requested type [" + expected_type.name() + "]") {
  }
};

class ValueExistInSetException : public std::runtime_error {
 public:
  ValueExistInSetException(std::string_view message) : std::runtime_error(message.data()) {}
};

template<typename value_t>
class ValueExistInSetExceptionT : public ValueExistInSetException {
 public:
  ValueExistInSetExceptionT(std::string_view key, const value_t &value) :
      ValueExistInSetException(
          std::string("value [") + std::to_string(value) + "] already exist in the set with key [" + std::string(key)
              + "]") {
  }
};
// specialization of above for string. TODO: can we avoid this?
template<>
class ValueExistInSetExceptionT<std::string> : public ValueExistInSetException {
 public:
  ValueExistInSetExceptionT(std::string_view key, const std::string &value) :
      ValueExistInSetException(
          std::string("value [") + value + "] already exist in the set with key [" + std::string(key) + "]") {
  }
};

//TODO: Add a concept of a service at generic level?
class KeyValueStore {
  using variant_t = std::variant<int32_t, int64_t, std::string>;
 public:
  template<typename value_t>
  class ListItemIterator : public std::iterator<
      std::input_iterator_tag,   // iterator_category
      value_t,                      // value_type
      value_t,                      // difference_type TODO:
      const value_t *,               // pointer
      value_t                       // reference
  > {

   public:
    explicit ListItemIterator(const std::vector<variant_t>::const_iterator &current) : _current(current) {
    }

    ListItemIterator &operator++() {
      _current++;
      return *this;
    }
    ListItemIterator operator++(int) {
      ListItemIterator ret_val = *this;
      ++(*this);
      return ret_val;
    }
    bool operator==(ListItemIterator other) const { return _current == other._current; }
    bool operator!=(ListItemIterator other) const { return !(*this == other); }
    typename ListItemIterator::reference operator*() const { return std::get<value_t>(*_current); }

   private:
    std::vector<variant_t>::const_iterator _current;
  };

  // single values
  template<typename value_t>
  void add(std::string_view key, const value_t &value) {
    _single_value_keys.template insert_or_assign(std::string(key), value); // over writes existing if present
  }

  template<typename value_t>
  value_t get(std::string_view key) const {

    auto ite = _single_value_keys.find(key);
    if (ite != _single_value_keys.end()) {
      try {
        return std::get<value_t>(ite->second);
      } catch (std::exception ex) {
        throw TypeMismatchException(key, std::type_index(typeid(value_t)));
      }
    } else {
      throw KeyDoesNotExistException(key);
    }
  }

  // list values
  template<typename value_t>
  void add_to_list(std::string_view key, const value_t &value) {
    auto[ite, created] = _list_value_keys.template try_emplace(std::string(key), std::type_index(typeid(value_t)), 0);
    if (!created) {
      // we already have a list with that key
      auto&[type_index, list] = ite->second;
      if (type_index == std::type_index(typeid(value_t))) {
        // new element matches, so add
        list.template emplace_back(value);
      } else {
        throw InvalidListElementTypeException(key, type_index, std::type_index(typeid(value_t)));
      }
    } else {
      // new key
      ite->second.first = std::type_index(typeid(value_t));
      ite->second.second.template emplace_back(value);
    }
  }

  template<typename value_t>
  const value_t &get_front_list(std::string_view key) const {
    auto ite = _list_value_keys.find(key);
    if (ite != _list_value_keys.end()) {
      return std::get<value_t>(ite->second.second.front());
    } else {
      throw KeyDoesNotExistException(key);
    }
  }

  template<typename value_t>
  const value_t &get_back_list(std::string_view key) const {
    auto ite = _list_value_keys.find(key);
    if (ite != _list_value_keys.end()) {
      return std::get<value_t>(ite->second.second.back());
    } else {
      throw KeyDoesNotExistException(key);
    }
  }

  template<typename value_t>
  const value_t &get_at_list(std::string_view key, std::uint32_t position) const {
    auto ite = _list_value_keys.find(key);
    if (ite != _list_value_keys.end()) {
      return std::get<value_t>(ite->second.second.at(position)); // could throw
    } else {
      throw KeyDoesNotExistException(key);
    }
  }

  size_t get_list_count(std::string_view key) const {
    auto ite = _list_value_keys.find(key);
    if (ite != _list_value_keys.end()) {
      return ite->second.second.size();
    } else {
      throw KeyDoesNotExistException(key);
    }
  }

  template<typename value_t>
  std::pair<ListItemIterator<value_t>, ListItemIterator<value_t>> get_list(std::string_view key) const {
    auto ite = _list_value_keys.find(key);
    if (ite != _list_value_keys.end()) {
      // first check if the expected type is same as items in the given list
      if (ite->second.first != std::type_index(typeid(value_t))) {
        throw InvalidListElementTypeException(key, ite->second.first, std::type_index(typeid(value_t)));
      } else {
        // all good
        return std::pair<ListItemIterator<value_t>, ListItemIterator<value_t>>(ite->second.second.cbegin(),
                                                                               ite->second.second.cend());
      }
    } else {
      throw KeyDoesNotExistException(key);
    }
  };

  template<typename value_t>
  void add_to_set(std::string_view key, const value_t &value) {

    auto[ite, created] = _set_value_keys.template try_emplace(std::string(key), std::type_index(typeid(value_t)), 0);

    if (!created) {
      auto&[type_index, set] = ite->second;
      if (type_index == std::type_index(typeid(value_t))) {
        const auto&[ite, added] = set.emplace(value);
        if (!added) {
          throw ValueExistInSetExceptionT(key, value);
        }
      } else {
        throw InvalidSetElementTypeException(key, type_index, std::type_index(typeid(value_t)));
      }
    } else {

      ite->second.first = std::type_index(typeid(value_t));
      ite->second.second.insert(value);
    }
  }

  bool set_key_exist(std::string_view set_key) const noexcept {
    return _set_value_keys.contains(set_key);
  }

  bool list_key_exist(std::string_view list_key) const noexcept {
    return _list_value_keys.contains(list_key);
  }

  bool value_key_exist(std::string_view value_key) const noexcept {
    return _single_value_keys.contains(value_key);
  }

  template<typename value_t>
  bool contains_in_set(std::string_view key, const value_t &value) const {

    auto ite = _set_value_keys.find(std::string(key));

    auto&[type_index_1, set] = ite->second;

    if (ite != _set_value_keys.end()) {
      if (type_index_1
          == std::type_index(typeid(value_t))) { // Had to move this inside to avoid the segmentaion fault in case of null type_index_1......................
        auto ite_set = ite->second.second.find(value);
        return (ite_set != ite->second.second.end());
      } else {
        throw InvalidSetElementTypeException(key, type_index_1, std::type_index(typeid(value_t)));
      }
    } else {
      throw KeyDoesNotExistException(key);
    }
  }

 private:
  std::unordered_map<std::string, variant_t, extensions::string_hash, std::equal_to<>> _single_value_keys;
  std::unordered_map<std::string,
                     std::pair<std::type_index, std::vector<variant_t>>,
                     extensions::string_hash,
                     std::equal_to<>>
      _list_value_keys; // type index is set during first list add for each key

  std::unordered_map<std::string,
                     std::pair<std::type_index, std::unordered_set<variant_t>>,
                     extensions::string_hash,
                     std::equal_to<>>
      _set_value_keys;
};

}
