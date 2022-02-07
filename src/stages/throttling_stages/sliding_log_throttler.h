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
#include <list>
#include <algorithm>

#include "./throttler.h"

namespace adl::axp::core::stages::throttling {

/**
 * Sliding log throttler
 * thread safe - no
 *
 */
class SlidingLogThrottler : public IThrottler {
 public:
  SlidingLogThrottler(size_t
                      window_duration,
                      size_t allowed_request_count
  ) :
      _window_duration(window_duration),
      _allowed_request_count(allowed_request_count) {
  };

  void add_timestamp(size_t time_stamp) noexcept override {
    _request_time_stamp_log.emplace_back(time_stamp);
    if (_request_time_stamp_log.size() >= _allowed_request_count) {
      // total request count is more than allowed count.
      // however this does not mean that all requests in the log fall under last time window.
      // but it is good idea to truncate the log at this time

      truncate(time_stamp);
    }
  }
  void re_evaluate(size_t time_stamp) noexcept override {
    if (_request_time_stamp_log.size() >= _allowed_request_count) {
      truncate(time_stamp);
    }
  }
  bool is_allowed() const noexcept override {
    return _request_time_stamp_log.size() <= _allowed_request_count;
  }

  size_t get_window_duration() const noexcept override {
    return _window_duration;
  }
  void set_window_duration(size_t window_duration) noexcept {
    _window_duration = window_duration;
  }
  size_t get_allowed_request_count() const noexcept override {
    return _allowed_request_count;
  }
  void set_allowed_request_count(size_t allowed_request_count) noexcept {
    _allowed_request_count = allowed_request_count;
  }
 private:
  void truncate(size_t time_stamp) {
    auto ite = std::lower_bound(std::begin(_request_time_stamp_log),
                                std::end(_request_time_stamp_log),
                                time_stamp - _window_duration);
    _request_time_stamp_log.erase(_request_time_stamp_log.begin(), ite);
  }

  size_t _window_duration{};
  size_t _allowed_request_count{};

  // ownership is not ours. We expect no writes will be performed on this from a different thread
  std::list<size_t> _request_time_stamp_log;
};

}
