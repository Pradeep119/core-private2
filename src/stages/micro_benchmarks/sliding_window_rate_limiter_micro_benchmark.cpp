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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-29.
*/

#include <iostream>
#include <benchmark/benchmark.h>
#include "../throttling_stages/sliding_log_throttler.h"

namespace adl::axp::core::stages::throttling::micobenchmarks {

static void BM_RateLimiter(benchmark::State &state) {
//  auto to_millies = [](const timespec &time_stamp) {
//    return time_stamp.tv_nsec / 1'000'000 + time_stamp.tv_sec * 1000;
//  };
//
//  for (auto _ : state) {
//    state.PauseTiming();
//    SlidingLogRateLimiter limiter{(size_t) state.range(0), (size_t) state.range(1)};
//    timespec time_stamp;
//    clock_gettime(CLOCK_MONOTONIC, &time_stamp);
//    size_t time_stamp_millies = to_millies(time_stamp);
//    state.ResumeTiming();
//    limiter.grant(time_stamp_millies,
//                  reinterpret_cast<void *>(time_stamp_millies),
//                  [](void *tag) {},
//                  [](void *tag) {});
//  }
}

// Register the function as a benchmark
BENCHMARK(BM_RateLimiter)->Ranges({{0, 86400000}, {0, 90000000}});
}



