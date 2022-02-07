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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-14.
 */


#include <gtest/gtest.h>
#include "../../services/store_services/key_value_store.h"

namespace adl::axp::core::services::store_services::tests {

template<typename value_t>
void add_get_int_test(const value_t &value) {
  KeyValueStore store;
  store.add<value_t>("k1", value);
  EXPECT_EQ(value, store.get<value_t>("k1"));
}

template<typename value_t>
void add_get_int_multiple(const value_t &value1, const value_t &value2) {
  KeyValueStore store;
  store.add<value_t>("k1", value1);
  store.add<value_t>("k2", value2);
  EXPECT_EQ(value1, store.get<value_t>("k1"));
  EXPECT_EQ(value2, store.get<value_t>("k2"));
}

template<typename value_t>
void add_get_int_overwrite(const value_t &value1, const value_t &value2) {
  KeyValueStore store;
  store.add<value_t>("k1", value1);
  store.add<value_t>("k1", value2);
  EXPECT_EQ(value2, store.get<value_t>("k1"));
}

template<typename value_t>
void no_key_test() {
  KeyValueStore store;
  store.add<value_t>("k1", value_t());
  EXPECT_THROW(store.get<int32_t>("k2"), KeyDoesNotExistException);
}

TEST(key_value_store, add_get_int) {
  add_get_int_test<int32_t>(1234);
}

TEST(key_value_store, add_get_string) {
  add_get_int_test<std::string>("s1");
}

TEST(key_value_store, add_get_int_multiple) {
  add_get_int_multiple<int32_t>(1234, 5678);
}

TEST(key_value_store, add_get_string_multiple) {
  add_get_int_multiple<std::string>("s1", "s2");
}

TEST(key_value_store, add_get_int_overwrite) {
  add_get_int_overwrite<int32_t>(3212, 67);
}

TEST(key_value_store, add_get_string_overwrite) {
  add_get_int_overwrite<std::string>("s1", "s2");
}

TEST(key_value_store, int_no_key) {
  no_key_test<int32_t>();
}

TEST(key_value_store, string_no_key) {
  no_key_test<int32_t>();
}

TEST(key_value_store, ovewrite_with_diff_type_int32_t_TO_int64_t) {
  KeyValueStore store;
  store.add<int32_t>("k1", 1234);
  store.add<int64_t>("k1", 123432112);
  EXPECT_EQ(123432112, store.get<int64_t>("k1"));
}

TEST(key_value_store, ovewrite_with_diff_type_int32_t_TO_string) {
  KeyValueStore store;
  store.add<int32_t>("k1", 1234);
  store.add<std::string>("k1", "s1");
  EXPECT_EQ("s1", store.get<std::string>("k1"));
}

TEST(key_value_store, wrong_type_get) {
  KeyValueStore store;
  store.add<int32_t>("k1", 1234);
  EXPECT_THROW(store.get<std::string>("k1"), TypeMismatchException);
}

TEST(key_value_store, add_get_front_list) {
  KeyValueStore store;
  store.add_to_list<int32_t>("k1", 1234);
  EXPECT_EQ(1234, store.get_front_list<int32_t>("k1"));
}

TEST(key_value_store, add_get_front_list_2) {
  KeyValueStore store;
  store.add_to_list<int32_t>("k1", 1234);
  store.add_to_list<int32_t>("k1", 3333);
  EXPECT_EQ(1234, store.get_front_list<int32_t>("k1"));
}

TEST(key_value_store, add_get_back_list) {
  KeyValueStore store;
  store.add_to_list<int32_t>("k1", 1234);
  EXPECT_EQ(1234, store.get_back_list<int32_t>("k1"));
}

TEST(key_value_store, add_get_back_list_2) {
  KeyValueStore store;
  store.add_to_list<int32_t>("k1", 1234);
  store.add_to_list<int32_t>("k1", 3333);
  EXPECT_EQ(3333, store.get_back_list<int32_t>("k1"));
}

TEST(key_value_store, add_get_list_different_types_throws) {
  KeyValueStore store;
  store.add_to_list<int32_t>("k1", 1234);
  EXPECT_NO_THROW(store.add_to_list<int32_t>("k1", 3333));
  EXPECT_THROW(store.add_to_list<std::string>("k1", "s1"), InvalidListElementTypeException);
}

TEST(key_value_store, get_list) {
  KeyValueStore store;
  store.add_to_list<int32_t>("k1", 1234);
  store.add_to_list<int32_t>("k1", 3333);
  store.add_to_list<int32_t>("k1", 87878);

  auto[ite, end] = store.get_list<int32_t>("k1");
  EXPECT_EQ(1234, *ite);
  EXPECT_EQ(3333, *++ite);
  EXPECT_EQ(3333, *ite++);
  EXPECT_EQ(87878, *ite);

  EXPECT_EQ(++ite, end);

  // also test usage with std algos
  auto[ite2, end2] = store.get_list<int32_t>("k1"); // get a fresh ite
  std::for_each(ite2, end2, [](int32_t value) { std::cout << value << " "; });
}

TEST(key_value_store, contains_in_set_wrong_key) {
  KeyValueStore store;
  store.add<int32_t>("k1", 1234);
  EXPECT_THROW(store.get<int32_t>("k2"), KeyDoesNotExistException);
}

TEST(key_value_store, contains_in_set_different_types_throws) {
  KeyValueStore store;
  store.add_to_set<int32_t>("k1", 1234);
  EXPECT_NO_THROW(store.add_to_set<int32_t>("k1", 3333));
  EXPECT_THROW(store.add_to_set<std::string>("k1", "s1"),
               InvalidSetElementTypeException);// Why this should throw InvalidSet... :
  //since set is already made according to type of the first element which is in variant_t and then it will throw this?
}

TEST(key_value_store, contains_in_set) {
  KeyValueStore store;
  // store.add_to_set<int32_t>("k1", 1234);
  store.add_to_set<int32_t>("k1", 3333);
  store.add_to_set<int32_t>("k1", 87878);


  // I have not something similar to get_list  or get for sets...
  EXPECT_EQ(store.contains_in_set("k1", 1234), true);
  EXPECT_EQ(store.contains_in_set("k1", 3333), true);
  EXPECT_EQ(store.contains_in_set("k1", 87878), true);
  EXPECT_EQ(store.contains_in_set("k1", 10000), false);

}
}