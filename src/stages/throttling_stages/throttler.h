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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-26.
 */

#pragma once

#include <functional>
#include "../../foundation/extensions.h"

namespace adl::axp::core::stages::throttling {

/**
 * Interface for throttlers
 * All implementations should derive from this interface
 */
interface IThrottler {
  virtual ~IThrottler() = default;
  virtual void add_timestamp(size_t time_stamp) noexcept = 0;
  virtual void re_evaluate(size_t time_stamp) noexcept = 0;
  virtual bool is_allowed() const noexcept = 0;
  virtual size_t get_allowed_request_count() const noexcept = 0;
  virtual size_t get_window_duration() const noexcept = 0;
};

}
