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

/**
 * Represent a message.
 * IThread safety - implementations can be either thread safe not thread safe
 */
interface IMessage {
  virtual ~IMessage() = default;
  virtual size_t message_type() const noexcept = 0;
};
}

/**
 * Use this macro to test if a given message is of a given type.
 * The hash calculation is done at compile time, there is no string
 * processing overhead at runtime.
 */
#define if_message_type(type, message) \
  if(adl::axp::core::extensions::compile_time_hash(type) == message.message_type())

