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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-07-29.
 */

#pragma once

#include <string>
#include <exception>
#include <stdexcept>

#include "../foundation/extensions.h"
#include "spdlog/spdlog.h"

namespace adl::axp::core::event_processing {

interface ISink;
interface ISource;

/**
 * This exception is thrown during startup of a stage if
 * an error is encountered.
 */
class StageStartException : public std::runtime_error {
 public:
  explicit StageStartException(const char *what) : std::runtime_error(what) {}
};

/**
 * IStage represents a message processing step. A stage can accept
 * messages from multiple sinks and emit messages through multiple
 * sources. It is advisable however to keep sinks and sources to one
 * each unless absolutely necessary.
 */
interface IStage {
  virtual ~IStage() = default;

  virtual std::string_view get_name() const noexcept = 0;
  virtual ISink *get_sink(std::string_view name) = 0;
  virtual const ISink *get_sink(std::string_view name) const = 0;
  virtual ISource *get_source(std::string_view name) = 0;
  virtual const ISource *get_source(std::string_view name) const = 0;
  virtual void start() noexcept(false) = 0;
};

}