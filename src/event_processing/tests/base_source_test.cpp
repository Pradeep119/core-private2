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

#include <gtest/gtest.h>

#include "../base_impl//base_source.h"

namespace adl::axp::core::event_processing::test {

TEST(source, set_get_sink) {
  details::BaseSource source;
  auto *sink = reinterpret_cast<ISink *>(2312441);
  source.set_sink(sink);
  EXPECT_EQ(reinterpret_cast<ISink *>(2312441), source.get_sink());
}

TEST(source, set_get_sink_const) {
  details::BaseSource source;
  auto *sink = reinterpret_cast<ISink *>(2312441);
  source.set_sink(sink);

  // to get a const context for stage
  auto runner = [](const details::BaseSource &source2) {
    EXPECT_EQ(reinterpret_cast<ISink *>(2312441), source2.get_sink());
  };

  runner(source);
}

}