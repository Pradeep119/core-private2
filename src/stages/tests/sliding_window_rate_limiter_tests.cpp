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

#include <mutex>
#include <thread>
#include <random>
#include <atomic>
#include <queue>

#include <gtest/gtest.h>
#include "../throttling_stages/sliding_log_throttler.h"

namespace adl::axp::core::stages::throttling::test {

namespace {
struct DummyStateSyncPolicy {};
struct NoLocker {
  void lock_queue() {
  }
  void unlock_queue() {
  }
  void lock_log() {
  }
  void unlock_log() {
  }
};
}

TEST(SlidingWindowRateLimiter, construction) {
//  SlidingLogRateLimiter<DummyStateSyncPolicy, NoLocker> limiter(10, 50, false);
}

TEST(SlidingWindowRateLimiter, smoke) {
  // rate control window is 10 (say milliseconds).
//  SlidingLogRateLimiter<DummyStateSyncPolicy, NoLocker> limiter(10, 5, false);
//
//  EXPECT_TRUE(limiter.grant(100, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_TRUE(limiter.grant(101, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_TRUE(limiter.grant(102, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_TRUE(limiter.grant(103, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_TRUE(limiter.grant(105, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_FALSE(limiter.grant(106, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_FALSE(limiter.grant(107, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_FALSE(limiter.grant(110, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//
//  EXPECT_TRUE(limiter.grant(115, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_TRUE(limiter.grant(116, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_TRUE(limiter.grant(116, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_TRUE(limiter.grant(117, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_TRUE(limiter.grant(118, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_FALSE(limiter.grant(119, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
//  EXPECT_FALSE(limiter.grant(120, -1, nullptr, [](void *) {}, [](void *) {}, [](void *) {}));
}

namespace {

struct TestParams {
  size_t _window_duration;
  size_t _allowed_rate;
  int _time_to_run;
  int _update_threshold_ratio;
};

class FuzzyFixture : public testing::TestWithParam<TestParams> {

 public:
  static std::list<TestParams> get_params() {
    std::list<TestParams> params{50};

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution window_duration_distribution(1, 9999);
    std::uniform_int_distribution allowed_rate_distribution(0, 10000);
    std::uniform_int_distribution time_to_run_distribution(10, 5000);
    std::uniform_int_distribution update_threshold_ratio_distribution(50, 100);

    std::generate(params.begin(),
                  params.end(),
                  [&gen, &window_duration_distribution, &allowed_rate_distribution, &time_to_run_distribution, &update_threshold_ratio_distribution]() {
                    TestParams param;
                    param._window_duration = window_duration_distribution(gen);
                    param._allowed_rate = allowed_rate_distribution(gen);
                    param._time_to_run = time_to_run_distribution(gen);
                    param._update_threshold_ratio = update_threshold_ratio_distribution(gen);

                    return param;
                  });
    return params;
  }

  void SetUp() override {
    _global_ts_log.clear();
    _update_count = 0;
  }

  size_t truncate(size_t time) {
    std::cout << "before truncated " << _global_ts_log.size() << " time " << time << std::endl;
    for (auto ts : _global_ts_log) {
      std::cout << ts << std::endl;
    }
    std::cout << std::endl;

    auto ite = std::lower_bound(std::begin(_global_ts_log), std::end(_global_ts_log), time);
    _global_ts_log.erase(_global_ts_log.begin(), ite);
    std::cout << "truncated " << _global_ts_log.size() << std::endl;
    return _global_ts_log.size();
  }

 protected:
  std::list<size_t> _global_ts_log;
  size_t _update_count;
};
}

TEST_P(FuzzyFixture, no_queue_fuzzy) {

//  const auto&[window_duration, allowed_rate, time_to_run, update_threshold_ratio] = GetParam();
//  SlidingLogRateLimiter limiter(window_duration, allowed_rate);
//
//  int update_threshold = allowed_rate / update_threshold_ratio;
//  if (update_threshold == 0) update_threshold = 1;
//
//  std::cout << "==================================================" << std::endl;
//  std::cout << "window_duration " << window_duration << std::endl;
//  std::cout << "allowed_rate " << allowed_rate << std::endl;
//  std::cout << "time_to_run " << time_to_run << std::endl;
//  std::cout << "update_threshold ratio " << update_threshold_ratio << std::endl;
//  std::cout << "update_threshold " << update_threshold << std::endl;
//  std::cout << "--------------------------------------------------" << std::endl;
//
//  std::atomic_bool stop = false;
//
//  std::deque<size_t> allowed_requests;
//  std::deque<size_t> suppressed_requests;
//
//  auto to_millies = [](const timespec &time_stamp) {
//    return time_stamp.tv_nsec / 1'000'000 + time_stamp.tv_sec * 1000;
//  };
//
//  auto generator =
//      [this, update_threshold, &stop, &to_millies, &limiter, &allowed_requests, &suppressed_requests, &window_duration]() {
//        while (!stop) {
//          std::random_device rd;
//          std::mt19937 gen(rd());
//          std::uniform_int_distribution inter_request_sleep_duration_distribution(0, 1000);
//          timespec time_stamp;
//          clock_gettime(CLOCK_MONOTONIC, &time_stamp);
//          size_t time_stamp_millies = to_millies(time_stamp);
//
//          _global_ts_log.push_back(time_stamp_millies);
//
//          if (++_update_count == update_threshold) {
//            truncate(time_stamp_millies - window_duration);
//            limiter.set_time_stamp_log(&_global_ts_log);
//            _update_count = 0;
////            std::cout << "updating" << std::endl;
//          }
//
//          limiter.grant(time_stamp_millies,
//                        reinterpret_cast<void *>(time_stamp_millies),
//                        [&allowed_requests](void *tag) {
//                          allowed_requests.push_back(reinterpret_cast<size_t>(tag));
//                        },
//                        [&suppressed_requests](void *tag) {
//                          suppressed_requests.push_back(reinterpret_cast<size_t>(tag));
//                        });
//
//          std::this_thread::sleep_for(std::chrono::duration<int, std::micro>(inter_request_sleep_duration_distribution(
//              gen)));
//        }
//      };
//
//  std::thread generator_thread(generator);
//  std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(time_to_run));
//  stop = true;
//  generator_thread.join();
//
//  for (auto ite = allowed_requests.begin(); ite < allowed_requests.end(); ++ite) {
//
//    // find request count from ite to sliding ite + window duration
//    auto window_end = std::lower_bound(ite, allowed_requests.end(), (*ite) + window_duration);
//    // find number of time stamps in this duration
//    auto count = std::count_if(ite, window_end, [](size_t) { return true; });
//
//    // this should be less or equal to allowed count. Slight deviations are possible due to
//    // periodic state updates
//    ASSERT_LE(count, allowed_rate + allowed_rate * 0.1); // 10% deviation allowed
//    // also, allowed request time stamps should be monotonically increasing.
//    if (ite != allowed_requests.end()) {
//      ASSERT_LE(*ite, *++ite);
//    }
//  }
}

INSTANTIATE_TEST_SUITE_P(NoQueueFuzzySuit,
                         FuzzyFixture,
                         testing::ValuesIn(FuzzyFixture::get_params()));

}