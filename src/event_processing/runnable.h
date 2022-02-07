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
 *  All stages that require a run loop should implement this.
 *  They should NEVER spawn threads.
 *  The caller will call run methods with a dedicated thread
 *
 *  all implementations should be thread safe
 */
interface IRunnable {
  virtual int run(void *data) noexcept = 0;
  virtual int stop() noexcept = 0;
};

}