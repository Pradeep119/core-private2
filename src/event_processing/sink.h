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

/**
 * Represent a get_sink. A get_sink can accept messages and process them.
 * IThread safety - implementations can be either thread safe not thread safe
 */
interface ISink {
  virtual ~ISink() = default;

  virtual void on_message(IMessage *message) noexcept = 0;
};

}