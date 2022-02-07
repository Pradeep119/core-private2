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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-06.
 */

#include "../extensions.h"
#include <gtest/gtest.h>

namespace adl::axp::core::foundation::test {

TEST(compile_time_hash, compile_time) {
  // this will not compile if hash cannot be evaluated at compile time
  [[maybe_unused]] constexpr size_t hash = extensions::compile_time_hash("hello");

}

TEST(compile_time_hash, same_hash) {
  EXPECT_EQ(extensions::compile_time_hash("hello"), extensions::compile_time_hash("hello"));
}

TEST(compile_time_hash, uniquness_basic) {
  EXPECT_NE(extensions::compile_time_hash("hello"), extensions::compile_time_hash("hello2"));
}

}