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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-01.
 */

#pragma once

#include <unordered_map>

#include "../../foundation/extensions.h"
#include "../stage.h"

namespace adl::axp::core::event_processing::details {

// exceptions
class SourceExistException : public std::runtime_error {
 public:
  explicit SourceExistException(std::string_view what) : std::runtime_error(std::string(what)) {}
};

class SinkExistException : public std::runtime_error {
 public:
  explicit SinkExistException(std::string_view what) : std::runtime_error(std::string(what)) {}
};

class SinkDoesNotExistException : public std::runtime_error {
 public:
  explicit SinkDoesNotExistException(std::string_view what) : std::runtime_error(std::string(what)) {}
};

class SourceDoesNotExistException : public std::runtime_error {
 public:
  explicit SourceDoesNotExistException(std::string_view what) : std::runtime_error(std::string(what)) {}
};

/**
 * A simple stage that can be used as the base for all other stages.
 * thread safe - no
 */
class BaseStage : public IStage {
 public:
  explicit BaseStage(std::string_view name) : _name(name) {
  }
  BaseStage(const BaseStage &rhs) = delete;
  BaseStage &operator=(const BaseStage &rhs) = delete;
  ~BaseStage() override = default;

  std::string_view get_name() const noexcept override { return _name; }

  ISink *get_sink(std::string_view name) override;
  const ISink *get_sink(std::string_view name) const override;
  ISource *get_source(std::string_view name) override;
  const ISource *get_source(std::string_view name) const override;
  void start() override {
    // nothing to do for a base stage
  }

 protected:
  void register_source(std::string_view source_name, ISource *source);
  void register_sink(std::string_view sink_name, ISink *sink);

 private:
  const std::string _name;

  std::unordered_map<std::string, ISource *, extensions::string_hash, std::equal_to<>> _sources;
  std::unordered_map<std::string, ISink *, extensions::string_hash, std::equal_to<>> _sinks;
};

}
