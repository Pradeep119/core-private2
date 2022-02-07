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

#include "../source.h"
#include "../sink.h"

namespace adl::axp::core::event_processing::details {

/**
 * A simple source that can be as the base for all other sources.
 * thread safe - no
 */
class BaseSource : public ISource {

 public:
  BaseSource() = default;
  BaseSource(const BaseSource &rhs) = delete;
  BaseSource &operator=(const BaseSource &rhs) = delete;

  ISink *get_sink() noexcept override { return _sink; }
  const ISink *get_sink() const noexcept override { return _sink; }
  void set_sink(ISink *sink) noexcept override { this->_sink = sink; }

 private:
  ISink *_sink{};
};

}
