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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-21.
 */

#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include <type_traits>
#include <ranges>
#include <algorithm>

#include "../../event_processing/messages/http_request_message.h"

/*
 * types - header, query params, path params
 * validators - compulsory, optional, data type, pattern (strings), less than/gt (numbers)
 */

namespace adl::axp::core::stages::http::validation {

// exceptions
class HeaderValidatorException : public std::runtime_error {
 public:
  HeaderValidatorException(std::string_view header_key, std::string_view error) :
      std::runtime_error("Validator error in header " + std::string(header_key) + "[" + std::string(error) + "]") {
  }
};

class QueryValidatorException : public std::runtime_error {
 public:
  QueryValidatorException(std::string_view query_key, std::string_view error) :
      std::runtime_error("Validator error in query " + std::string(query_key) + "[" + std::string(error) + "]") {
  }
};

class PathValidatorException : public std::runtime_error {
 public:
  PathValidatorException(std::string_view header_key, std::string_view error) :
      std::runtime_error("Validator error in header " + std::string(header_key) + "[" + std::string(error) + "]") {
  }
};

class RequestValidatorException : public std::runtime_error {
 public:
  RequestValidatorException(std::string_view header_key, std::string_view error) :
      std::runtime_error("Validator error in header " + std::string(header_key) + "[" + std::string(error) + "]") {
  }
};

/**
 * Validates if a header is compulsory.
 * visit must be called for each headers present
 */
class CompulsoryValidator {
 public:
  // For a compulsory validator, getting a call
  // for visit indicates that the compulsory
  // header is present, so the validator becomes valid
  void visit() { _valid = true; }
  bool is_validated() const { return _valid; }
  void reset() noexcept {
    _valid = false;
  }
 private:
  bool _valid = false;
};

// generic template class for data type validators
// Specialize for each data type
template<typename data_t>
class DataTypeValidator {};

// specialization for int64_t
template<>
class DataTypeValidator<int64_t> {
 public:
  void visit(std::string_view str_value) {
    // this gets called with the header value.
    // verify this value can be converted to an integer.
    _valid = std::ranges::all_of(str_value.begin(), str_value.end(),
                                 [](char c) { return isdigit(c) != 0; });
  }

  bool is_validated() const { return _valid; }
  void reset() noexcept {
    _valid = true;
  }

 private:
  bool _valid = true;
};

// specialization for double
template<>
class DataTypeValidator<double> {
 public:

  void visit(std::string_view str_value) {
    if (std::count(str_value.begin(), str_value.end(), '.') == 1) {
      int position = str_value.find('.');
      _valid = std::ranges::all_of(str_value.substr(0, position).begin(),
                                   str_value.substr(0, position).end(),
                                   [](char c) { return isdigit(c) != 0; })
          and std::ranges::all_of(str_value.substr(position + 1, str_value.length()).begin(),
                                  str_value.substr(position + 1, str_value.length()).end(),
                                  [](char c) { return isdigit(c) != 0; });
    } else {
      _valid = false;
    }
  }

  bool is_validated() const { return _valid; }

  void reset() noexcept {
    _valid = true;
  }

 private:
  bool _valid = true;
};

//TODO: add other data type validators

// represents a single header validator
interface IHeaderValidator {
  virtual void visit(std::string_view key, std::string_view value) = 0;
  virtual bool is_validated() const noexcept = 0;
  virtual void reset() noexcept = 0;
};

template<typename validator_t>
class HeaderValidatorWrapper : public IHeaderValidator {
 public:
  virtual void visit(std::string_view key, std::string_view value) {
    _validator.visit(value);
  }
  virtual bool is_validated() const noexcept { return _validator.is_validated(); }
  virtual void reset() noexcept { _validator.reset(); }

 private:
  validator_t _validator;
};

template<>
class HeaderValidatorWrapper<CompulsoryValidator> : public IHeaderValidator {
 public:
  virtual void visit(std::string_view key, std::string_view value) {
    _validator.visit();
  }
  virtual bool is_validated() const noexcept { return _validator.is_validated(); }
  virtual void reset() noexcept { _validator.reset(); }

 private:
  CompulsoryValidator _validator;
};

class HeaderSectionValidator {
 public:
  /**
   * Validates a range of headers.
   * @tparam input_ite_t input iterator that gives std::pair<std::string, std::string> as value type
   * @param start start of the header range
   * @param end end of the header range
   * @return true if header validation passes
   */
  template<typename input_ite_t>
  bool validate(input_ite_t start, input_ite_t end) const {

    // reset all validators
    std::ranges::for_each(_validators.begin(), _validators.end(), [](const auto &item) { item.second->reset(); });

    bool valid = true;

    // TODO: use for_each
    while (start != end) {
      // go through each header
      const auto &key_val_pair = *start;

      // extract header key and header value
      const std::string_view s_key = key_val_pair.first;
      const std::string_view s_value = key_val_pair.second;

      // find all validators that apply to this header.
      // There can be multiple validators for each header key.
      auto ite = _validators.equal_range(key_val_pair.first);

      // call visit of each validator with this header info
      std::for_each(ite.first,
                    ite.second,
                    [s_key, s_value](auto &validator) { validator.second->visit(s_key, s_value); });

      ++start;
    }

    // check if all validators have passed
    valid &= std::all_of(_validators.begin(),
                         _validators.end(),
                         [](const auto &item) { return item.second->is_validated(); });

    return valid;
  }

  HeaderSectionValidator &add_compulsory_validator(std::string_view header_key) {
    _validators.emplace(std::make_pair(std::string(header_key), new HeaderValidatorWrapper<CompulsoryValidator>()));
    return *this;
  }

  template<typename data_t>
  HeaderSectionValidator &add_data_type_validator(std::string_view header_key) {
    if (std::is_integral<data_t>::value) {
      _validators.emplace(std::make_pair(std::string(header_key),
                                         new HeaderValidatorWrapper<DataTypeValidator<int64_t>>()));
    } else if (std::is_floating_point<data_t>::value) {
      _validators.emplace(std::make_pair(std::string(header_key),
                                         new HeaderValidatorWrapper<DataTypeValidator<double>>()));
    } //TODO: other types

    return *this;
  }

 private:
  std::unordered_multimap<std::string, IHeaderValidator *> _validators;

  friend class HeaderValidatorBuilder;
};

interface IQueryValidator {
  virtual void visit(std::string_view key, std::string_view value) = 0;
  virtual bool is_validated() const noexcept = 0;
  virtual void reset() noexcept = 0;
};

template<typename validator_t>
class QueryValidatorWrapper : public IQueryValidator {
 public:
  virtual void visit(std::string_view key, std::string_view value) {
    _validator.visit(value);
  }
  virtual bool is_validated() const noexcept { return _validator.is_validated(); }
  virtual void reset() noexcept { _validator.reset(); }

 private:
  validator_t _validator;
};

template<>
class QueryValidatorWrapper<CompulsoryValidator> : public IQueryValidator {
 public:
  virtual void visit(std::string_view key, std::string_view value) {
    _validator.visit();
  }
  virtual bool is_validated() const noexcept { return _validator.is_validated(); }
  virtual void reset() noexcept { _validator.reset(); }

 private:
  CompulsoryValidator _validator;
};

class QueryValidator {
 public:
  template<typename input_ite_t>
  bool validate(input_ite_t start, input_ite_t end) const {

    // reset all validators
    std::ranges::for_each(_validators.begin(), _validators.end(), [](const auto &item) { item.second->reset(); });

    bool valid = true;

    while (start != end) {
      const auto &key_val_pair = *start;

      const std::string_view s_key = key_val_pair.first;
      const std::string_view s_value = key_val_pair.second;

      auto ite = _validators.equal_range(key_val_pair.first);

      std::for_each(ite.first,
                    ite.second,
                    [s_key, s_value](auto &validator) { validator.second->visit(s_key, s_value); });

      ++start;
    }

    // check if all validators have passed
    valid &= std::all_of(_validators.begin(),
                         _validators.end(),
                         [](const auto &item) { return item.second->is_validated(); });

    return valid;
  }

  QueryValidator &add_compulsory_validator(std::string_view query_key) {
    _validators.emplace(std::make_pair(std::string(query_key), new QueryValidatorWrapper<CompulsoryValidator>()));
    return *this;
  }

  template<typename data_t>
  QueryValidator &add_data_type_validator(std::string_view query_key) {
    if (std::is_integral<data_t>::value) {
      _validators.emplace(std::make_pair(std::string(query_key),
                                         new QueryValidatorWrapper<DataTypeValidator<int64_t>>()));
    } else if (std::is_floating_point<data_t>::value) {
      _validators.emplace(std::make_pair(std::string(query_key),
                                         new QueryValidatorWrapper<DataTypeValidator<double>>()));
    } //TODO: other types

    return *this;
  }

 private:
  std::unordered_multimap<std::string, IQueryValidator *> _validators;

  friend class QueryValidatorBuilder;
};

interface IPathValidator {
  virtual void visit(std::string_view key, std::string_view value) = 0;
  virtual bool is_validated() const noexcept = 0;
  virtual void reset() noexcept = 0;
};

template<typename validator_t>
class PathValidatorWrapper : public IPathValidator {
 public:
  virtual void visit(std::string_view key, std::string_view value) {
    _validator.visit(value);
  }
  virtual bool is_validated() const noexcept { return _validator.is_validated(); }
  virtual void reset() noexcept { _validator.reset(); }

 private:
  validator_t _validator;
};

template<>
class PathValidatorWrapper<CompulsoryValidator> : public IPathValidator {
 public:
  virtual void visit(std::string_view key, std::string_view value) {
    _validator.visit();
  }
  virtual bool is_validated() const noexcept { return _validator.is_validated(); }
  virtual void reset() noexcept { _validator.reset(); }

 private:
  CompulsoryValidator _validator;
};

class PathValidator {
 public:
  /**
   * Validates a range of path.
   * @tparam input_ite_t input iterator that gives std::pair<std::string, std::string> as value type
   * @param start start of the path range
   * @param end end of the path range
   * @return true if path validation passes
   */
  template<typename input_ite_t>
  bool validate(input_ite_t start, input_ite_t end) const {

    // reset all validators
    std::ranges::for_each(_validators.begin(), _validators.end(), [](const auto &item) { item.second->reset(); });

    bool valid = true;

    // TODO: use for_each
    while (start != end) {
      // go through each path
      const auto &key_val_pair = *start;

      // extract header key and path value
      const std::string_view s_key = key_val_pair.first;
      const std::string_view s_value = key_val_pair.second;

      // find all validators that apply to this path.
      // There can be multiple validators for each path key.
      auto ite = _validators.equal_range(key_val_pair.first);

      // call visit of each validator with this path info
      std::for_each(ite.first,
                    ite.second,
                    [s_key, s_value](auto &validator) { validator.second->visit(s_key, s_value); });

      ++start;
    }

    // check if all validators have passed
    valid &= std::all_of(_validators.begin(),
                         _validators.end(),
                         [](const auto &item) { return item.second->is_validated(); });

    return valid;
  }

  PathValidator &add_compulsory_validator(std::string_view path_key) {
    _validators.emplace(std::make_pair(std::string(path_key), new PathValidatorWrapper<CompulsoryValidator>()));
    return *this;
  }

  template<typename data_t>
  PathValidator &add_data_type_validator(std::string_view path_key) {
    if (std::is_integral<data_t>::value) {
      _validators.emplace(std::make_pair(std::string(path_key),
                                         new PathValidatorWrapper<DataTypeValidator<int64_t>>()));
    } else if (std::is_floating_point<data_t>::value) {
      _validators.emplace(std::make_pair(std::string(path_key),
                                         new PathValidatorWrapper<DataTypeValidator<double>>()));
    } //TODO: other types

    return *this;
  }

 private:
  std::unordered_multimap<std::string, IPathValidator *> _validators;

  friend class PathValidatorBuilder;
};

struct RequestValidator {
  HeaderSectionValidator _header_validator;
  QueryValidator _query_validator;
  PathValidator _path_validator;
};

}