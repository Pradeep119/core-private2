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

#include "../foundation/extensions.h"

namespace adl::axp::core::event_processing {

interface IMessage;
interface ISink;

/**
 * Represent a source. A source can emit messages to the connected get_sink.
 * IThread safety - implementations can be either thread safe not thread safe
 */
interface ISource {
  virtual ~ISource() = default;

  virtual ISink *get_sink() noexcept = 0;
  virtual const ISink *get_sink() const noexcept = 0;
  virtual void set_sink(ISink *sink) noexcept = 0;
};

}