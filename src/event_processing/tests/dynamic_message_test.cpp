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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-10-05.
 */

#include <gtest/gtest.h>

#include "../dynamic_message.h"

namespace adl::axp::core::event_processing::test {

TEST(dyna_message, one_group) {
  DynamicMessage msg;
  msg.set_field("grp", "test", "val");
  EXPECT_EQ("val", msg.get_field<std::string>("grp", "test"));
  msg.set_field("grp", "test2", "val2");
  EXPECT_EQ("val2", msg.get_field<std::string>("grp", "test2"));
  msg.set_field("grp", "test2", "val3");
  EXPECT_EQ("val3", msg.get_field<std::string>("grp", "test2"));
  EXPECT_THROW(msg.get_field<std::string>("grp2", "test2"), FieldGroupDoesNotExistException);
  EXPECT_THROW(msg.get_field<std::string>("grp", "test5"), FieldNameDoesNotExistException);
}

TEST(dyna_message, two_groups) {
  DynamicMessage msg;
  msg.set_field("grp", "test", "val");
  msg.set_field("grp2", "test", "val2");
  EXPECT_EQ("val", msg.get_field<std::string>("grp", "test"));
  EXPECT_EQ("val2", msg.get_field<std::string>("grp2", "test"));
  msg.set_field("grp", "test2", "val2");
  msg.set_field("grp2", "test2", "val3");
  EXPECT_EQ("val2", msg.get_field<std::string>("grp", "test2"));
  EXPECT_EQ("val3", msg.get_field<std::string>("grp2", "test2"));

  msg.set_field("grp", "test2", "val10");
  msg.set_field("grp2", "test2", "val11");
  EXPECT_EQ("val10", msg.get_field<std::string>("grp", "test2"));
  EXPECT_EQ("val11", msg.get_field<std::string>("grp2", "test2"));

  EXPECT_THROW(msg.get_field<std::string>("grp3", "test2"), FieldGroupDoesNotExistException);
  EXPECT_THROW(msg.get_field<std::string>("grp", "test50"), FieldNameDoesNotExistException);
  EXPECT_THROW(msg.get_field<std::string>("grp2", "test51"), FieldNameDoesNotExistException);
}

TEST(dyna_message, single_field_typed_group) {
  DynamicMessage msg;

  msg.create_typed_group<std::int32_t>("grp");
  msg.set_typed_field("grp", "name1", 1);
  msg.set_typed_field("grp", "name2", 2);

  EXPECT_EQ(1, msg.get_typed_field<std::int32_t>("grp", "name1"));
  EXPECT_EQ(2, msg.get_typed_field<std::int32_t>("grp", "name2"));
}

TEST(dyna_message, single_field_typed_group_overwrite) {
  DynamicMessage msg;

  msg.create_typed_group<std::int32_t>("grp");
  msg.set_typed_field("grp", "name1", 1);
  msg.set_typed_field("grp", "name2", 2);
  msg.set_typed_field("grp", "name2", 5);

  EXPECT_EQ(1, msg.get_typed_field<std::int32_t>("grp", "name1"));
  EXPECT_EQ(5, msg.get_typed_field<std::int32_t>("grp", "name2"));
}

TEST(dyna_message, group_iteration_single_item) {
  DynamicMessage msg;

  msg.create_typed_group<std::int32_t>("grp");
  msg.set_typed_field("grp", "name1", 1);

  auto[begin, end] = msg.get_typed_group<int32_t>("grp");
  EXPECT_EQ("name1", (*begin).first);
  EXPECT_EQ(1, (*begin).second);
  ++begin;
  EXPECT_TRUE(begin == end);
}

TEST(dyna_message, group_iteration_multi_item) {
  DynamicMessage msg;
  msg.create_typed_group<std::int32_t>("grp");
  msg.set_typed_field("grp", "test", 1);
  msg.set_typed_field("grp", "test2", 2);

  auto[begin, end] = msg.get_typed_group<std::int32_t>("grp");
  std::for_each(begin, end, [](const auto &pair) {
    if (pair.first == "test") {
      EXPECT_EQ(1, pair.second);
    }
    if (pair.first == "test2") {
      EXPECT_EQ(2, pair.second);
    }
  });
}

TEST(dyna_message, typed_group_bad_insert) {
  DynamicMessage msg;
  msg.create_typed_group<std::int32_t>("grp");
  msg.set_typed_field("grp", "test", 1);
  EXPECT_THROW(msg.set_typed_field("grp", "test2", "test"), TypedGroupTypeMismatchException);
}

TEST(dyna_message, typed_group_bad_get) {
  DynamicMessage msg;
  msg.create_typed_group<std::int32_t>("grp");
  msg.set_typed_field("grp", "test", 1);
  msg.set_typed_field("grp", "test2", 2);

  EXPECT_THROW(msg.get_typed_group<std::string>("grp"), TypedGroupTypeMismatchException);
}

}
