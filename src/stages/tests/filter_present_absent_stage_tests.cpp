/*
 *
 *   © Copyrights 2021 Axiata Digital Labs Pvt Ltd.
 *   All Rights Reserved.
 *
 *   These material are unpublished, proprietary, confidential source
 *   code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
 *   SECRET of ADL.
 *
 *   ADL retains all title to and intellectual property rights in these
 *   materials.
 *
 *
 *   @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on 10/15/21.
 *
 */

#include <list>

#include <gtest/gtest.h>

#include "../filter_stage/filter_present_absent_stage.h"

namespace adl::axp::core::stages::filter::filter_present_absent::test {

/**Message collector class*/
namespace {
class MessageCollectorSink : public event_processing::ISink {
 public:
  MessageCollectorSink(const std::string &name) : _name(name) {
  }

  void on_message(event_processing::IMessage *message) noexcept override {
    _received_messages.emplace_back(message);
    executed = true;

    if (_name == "except") {
      const auto *dynamic_message = static_cast<event_processing::DynamicMessage *>(message);
      const auto error_information = dynamic_message->get_field<std::string>("exceptions", "FilterPresentAbsentStage");
      std::cout << "Generated Exception : " << error_information << std::endl;
    }
  }

/**This will return "wrong" if the on_message of this sink is not hit*/
  std::string get_input_source_name() {
    if (executed) {
      executed = false;
      return _name;
    }
    return "wrong";
  }

  std::vector<event_processing::IMessage *> get_received_messages() {
    return _received_messages;
  }

 private:
  std::vector<event_processing::IMessage *> _received_messages;
  const std::string &_name;
  bool executed = false;
};
}

// whitelist
TEST(filter_stage, empty_message_whitelist) {

  std::cout << "................................................. " << std::endl;
  std::cout << "empty message test... " << std::endl;

  //The store
  services::store_services::KeyValueStore store;

  stages::filter_stage::WhitelistingStage::field_combination_t wl_field_combinations
      {
          {
              {"wl_set_grp_1", "wl_set_fld_1"},
              {"wl_grp1", "wl_fld1"}
          },
          {
              {"wl_set_grp_2", "wl_set_fld_2"},
              {"wl_grp2", "wl_fld2"}
          }
      };

  stages::filter_stage::WhitelistingStage::field_combination_t bl_field_combinations
      {
          {
              {"bl_set_grp_1", "bl_set_fld_1"},
              {"bl_grp1", "bl_fld1"}
          },
          {
              {"bl_set_grp_2", "bl_set_fld_2"},
              {"bl_grp2", "bl_fld2"}
          }
      };

  stages::filter_stage::WhitelistingStage whitelist_stage
      {"white_stage_issuer", store, wl_field_combinations};
  stages::filter_stage::BlacklistingStage blacklist_stage
      {"black_stage_issuer", store, bl_field_combinations};


  /** one black list and one white list*/
  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");

  auto *dynamic_message_1 = new event_processing::DynamicMessage();

  //sinks
  MessageCollectorSink filtered{"Filtered"};
  MessageCollectorSink out{"out"};
  MessageCollectorSink except{"except"};

  whitelist_stage.get_source("out")->set_sink(&out);
  whitelist_stage.get_source("filtered")->set_sink(&filtered);
  whitelist_stage.get_source("exception_out")->set_sink(&except);

  dynamic_message_1->set_field("testing", "key", "my key 1");

  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
  EXPECT_EQ(0, out.get_received_messages().size());
  EXPECT_EQ(0, filtered.get_received_messages().size());
  EXPECT_EQ(1, except.get_received_messages().size());

  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());

  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");

  std::cout << "................................................. " << std::endl;

}

TEST(filter_stage, value_is_in_whitelist) {

  std::cout << "................................................. " << std::endl;
  std::cout << "clean output test... " << std::endl;
  std::cout << "The value is in whiteList" << std::endl;

  //The store
  services::store_services::KeyValueStore store;

  stages::filter_stage::WhitelistingStage::field_combination_t wl_field_combinations
      {
          {
              {"wl_set_grp_1", "wl_set_fld_1"},
              {"wl_grp1", "wl_fld1"}
          },
          {
              {"wl_set_grp_2", "wl_set_fld_2"},
              {"wl_grp2", "wl_fld2"}
          }
      };

  stages::filter_stage::WhitelistingStage whitelist_stage
      {"white_stage_issuer", store, wl_field_combinations};


  /** one black list and one white list*/
  store.add_to_set<std::string>("set_key_1", "abc");
  store.add_to_set<std::string>("set_key_1", "pqr");
  store.add_to_set<std::string>("set_key_2", "lmn");
  store.add_to_set<std::string>("set_key_2", "xyz");

  // this message should pass
  event_processing::DynamicMessage dynamic_message_1;
  dynamic_message_1.set_field("testing", "key", "my key 1");

  dynamic_message_1.set_field<std::string>("wl_set_grp_1", "wl_set_fld_1", "set_key_1");
  dynamic_message_1.set_field<std::string>("wl_grp1", "wl_fld1", "abc");
  dynamic_message_1.set_field<std::string>("wl_set_grp_2", "wl_set_fld_2", "set_key_2");
  dynamic_message_1.set_field<std::string>("wl_grp2", "wl_fld2", "xyz");

  // this message should get filtered because set_key_2
  // does not contain 123
  event_processing::DynamicMessage dynamic_message_2;
  dynamic_message_2.set_field("testing", "key", "my key 2");

  dynamic_message_2.set_field<std::string>("wl_set_grp_1", "wl_set_fld_1", "set_key_1");
  dynamic_message_2.set_field<std::string>("wl_grp1", "wl_fld1", "abc");
  dynamic_message_2.set_field<std::string>("wl_set_grp_2", "wl_set_fld_2", "set_key_2");
  dynamic_message_2.set_field<std::string>("wl_grp2", "wl_fld2", "123");

  // this message should pass as set_key_4 is not in the store
  event_processing::DynamicMessage dynamic_message_3;
  dynamic_message_3.set_field("testing", "key", "my key 3");

  dynamic_message_3.set_field<std::string>("wl_set_grp_1", "wl_set_fld_1", "set_key_1");
  dynamic_message_3.set_field<std::string>("wl_grp1", "wl_fld1", "abc");
  dynamic_message_3.set_field<std::string>("wl_set_grp_2", "wl_set_fld_2", "set_key_3");
  dynamic_message_3.set_field<std::string>("wl_grp2", "wl_fld2", "123");

  //sinks
  MessageCollectorSink filtered{"Filtered"};
  MessageCollectorSink out{"out"};
  MessageCollectorSink except{"except"};

  whitelist_stage.get_source("out")->set_sink(&out);
  whitelist_stage.get_source("filtered")->set_sink(&filtered);
  whitelist_stage.get_source("exception_out")->set_sink(&except);

  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(&dynamic_message_1));
  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(&dynamic_message_2));
  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(&dynamic_message_3));
  EXPECT_EQ(2, out.get_received_messages().size());
  EXPECT_EQ(1, filtered.get_received_messages().size());
  EXPECT_EQ(0, except.get_received_messages().size());

  // passed
  const auto *passed_msg_1 = static_cast<event_processing::DynamicMessage *>(out.get_received_messages()[0]);
  const auto *passed_msg_2 = static_cast<event_processing::DynamicMessage *>(out.get_received_messages()[1]);

  EXPECT_EQ("my key 1", passed_msg_1->template get_field<std::string>("testing", "key"));
  EXPECT_EQ("my key 3", passed_msg_2->template get_field<std::string>("testing", "key"));

  // filtered
  const auto *filtered_msg_1 = static_cast<event_processing::DynamicMessage *>(filtered.get_received_messages()[0]);

  EXPECT_EQ("my key 2", filtered_msg_1->template get_field<std::string>("testing", "key"));

  std::cout << "................................................. " << std::endl;

}
//TEST(filter_stage, value_is_not_in_whitelist) {
//
//  std::cout << "" << std::endl;
//  std::cout << "................................................. " << std::endl;
//  std::cout << "filtering test... " << std::endl;
//  std::cout << "The value is not in the whitelist" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto dynamic_message_1 = new event_processing::DynamicMessage();
//
//  //setting values to the message
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "https://localhost:4000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(1, filtered.get_received_messages().size());
//  EXPECT_EQ(0, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*filtered.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << ".................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, wrong_set_postfix_in_info_provider_whitelist) {
//
//  std::cout << "" << std::endl;
//  std::cout << "................................................. " << std::endl;
//  std::cout << "\"Set postfix\" mismatch test... " << std::endl;
//  std::cout << "\"Set postfix\" is wrong in set_info_provider" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1_wrong"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "https://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << ".................................................. " << std::endl;
//
//}
//TEST(filter_stage, wrong_set_postfix_in_store_whitelist) {
//
//  std::cout << "" << std::endl;
//  std::cout << "................................................. " << std::endl;
//  std::cout << "\"Set postfix\" mismatch test...... " << std::endl;
//  std::cout << "\"Set postfix\" is wrong in store" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//  store.add_to_set<std::string>("prefix_1/postfix_1_wrong", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1_wrong", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "https://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << ".................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, wrong_set_prefix_in_message_whitelist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << "\"Set prefix\" mismatch test......" << std::endl;
//  std::cout << "\"Set prefix\" is wrong in message" << std::endl;
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_wrong");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, wrong_set_prefix_in_store_whitelist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"Set prefix\" mismatch test... " << std::endl;
//  std::cout << " \"Set prefix\" is wrong in store" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1_wrong/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1_wrong/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, wrong_set_prefix_lookup_name_in_message_whitelist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"Set prefix lookup name\" mismatch test... " << std::endl;
//  std::cout << " \"Set prefix lookup name\" is wrong in set_info_provider" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1_wrong", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, wrong_set_prefix_lookup_name_in_info_provider_whitelist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"Set prefix lookup name\" mismatch test... " << std::endl;
//  std::cout << " \"Set prefix lookup name\" is wrong in the set info provider.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1_wrong", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, wrong_message_lookup_field_name_in_message_whitelist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"message lookup field name\" mismatch test... " << std::endl;
//  std::cout << " \"message lookup field name\" is wrong in the message.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name_wrong",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, wrong_message_lookup_field_name_in_stage_whitelist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"message lookup field name\" mismatch test... " << std::endl;
//  std::cout << " \"message lookup field name\" is wrong in the stage.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group",
//       "message_lookup_field_name_wrong",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, whitelist_is_a_blacklist_clean) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << "wrong stage test... " << std::endl;
//  std::cout << "The whiteList is actually a blacklist.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(1, filtered.get_received_messages().size());
//  EXPECT_EQ(0, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*filtered.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, whitelist_is_a_blacklist_filtered) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << "wrong stage test... " << std::endl;
//  std::cout << "The whiteList is actually a blacklist.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_1", "prefix_1");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5001/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  whitelist_stage.get_source("out")->set_sink(&out);
//  whitelist_stage.get_source("filtered")->set_sink(&filtered);
//  whitelist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(whitelist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(1, out.get_received_messages().size());
//  EXPECT_NE(1, filtered.get_received_messages().size());
//  EXPECT_EQ(0, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*out.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//
//
//// blacklist
//TEST(filter_stage, empty_message_blacklist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << "empty message test... " << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, value_is_in_blacklist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << "clean output test... " << std::endl;
//  std::cout << "The value is in blacklist" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:8080/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(1, filtered.get_received_messages().size());
//  EXPECT_EQ(0, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*filtered.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, value_is_not_in_blacklist) {
//
//  std::cout << "" << std::endl;
//  std::cout << "................................................. " << std::endl;
//  std::cout << "filtering test... " << std::endl;
//  std::cout << "The value is not in the whitelist" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto dynamic_message_1 = new event_processing::DynamicMessage();
//
//  //setting values to the message
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "https://localhost:4000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(1, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(0, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*out.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << ".................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, wrong_set_postfix_in_info_provider_blacklist) {
//
//  std::cout << "" << std::endl;
//  std::cout << "................................................. " << std::endl;
//  std::cout << "\"Set postfix\" mismatch test... " << std::endl;
//  std::cout << "\"Set postfix\" is wrong in set_info_provider" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2_wrong"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:8080/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << ".................................................. " << std::endl;
//
//}
//TEST(filter_stage, wrong_set_postfix_in_store_blacklist) {
//
//  std::cout << "" << std::endl;
//  std::cout << "................................................. " << std::endl;
//  std::cout << "\"Set postfix\" mismatch test...... " << std::endl;
//  std::cout << "\"Set postfix\" is wrong in store" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2_wrong", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2_wrong", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:8080/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << ".................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, wrong_set_prefix_in_message_blacklist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << "\"Set prefix\" mismatch test......" << std::endl;
//  std::cout << "\"Set prefix\" is wrong in message" << std::endl;
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_wrong");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"Filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, wrong_set_prefix_in_store_blacklist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"Set prefix\" mismatch test... " << std::endl;
//  std::cout << " \"Set prefix\" is wrong in store" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2_wrong/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2_wrong/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:8080/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, wrong_set_prefix_lookup_name_in_message_blacklist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"Set prefix lookup name\" mismatch test... " << std::endl;
//  std::cout << " \"Set prefix lookup name\" is wrong in set_info_provider" << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2_wrong", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:8080/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, wrong_set_prefix_lookup_name_in_info_provider_blacklist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"Set prefix lookup name\" mismatch test... " << std::endl;
//  std::cout << " \"Set prefix lookup name\" is wrong in the set info provider.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2_wrong", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, wrong_message_lookup_field_name_in_message_blacklist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"message lookup field name\" mismatch test... " << std::endl;
//  std::cout << " \"message lookup field name\" is wrong in the message.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name_wrong",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, wrong_message_lookup_field_name_in_stage_blacklist) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << " \"message lookup field name\" mismatch test... " << std::endl;
//  std::cout << " \"message lookup field name\" is wrong in the stage.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group",
//       "message_lookup_field_name_wrong",
//       false};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(0, out.get_received_messages().size());
//  EXPECT_EQ(0, filtered.get_received_messages().size());
//  EXPECT_EQ(1, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*except.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//
//TEST(filter_stage, blacklist_is_a_whitelist_clean) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << "wrong stage test... " << std::endl;
//  std::cout << "The blacklist is actually a whitelist.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:5000/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_NE(1, out.get_received_messages().size());
//  EXPECT_EQ(1, filtered.get_received_messages().size());
//  EXPECT_EQ(0, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*filtered.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}
//TEST(filter_stage, blacklist_is_a_whitelist_filered) {
//
//  std::cout << "................................................. " << std::endl;
//  std::cout << "wrong stage test... " << std::endl;
//  std::cout << "The blacklist is actually a whitelist.." << std::endl;
//
//  //The store
//  services::store_services::KeyValueStore store;
//
//  stages::filter_stage::SetInfoProvider
//      whitelist_set_info_provider{store, "filter", "set_prefix_lookup_name_1", "postfix_1"};
//  stages::filter_stage::SetInfoProvider
//      blacklist_set_info_provider{store, "filter", "set_prefix_lookup_name_2", "postfix_2"};
//
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      whitelist_stage
//      {"white_stage_issuer", whitelist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//  stages::filter_stage::FilterPresentAbsentStage<stages::filter_stage::SetInfoProvider>
//      blacklist_stage
//      {"black_stage_issuer", blacklist_set_info_provider, "message_lookup_field_group", "message_lookup_field_name",
//       true};
//
//
//  /** one black list and one white list*/
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:5000/oauth");
//  store.add_to_set<std::string>("prefix_1/postfix_1", "http://localhost:3000/test");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8080/oauth");
//  store.add_to_set<std::string>("prefix_2/postfix_2", "http://localhost:8081/oauth");
//
//  auto *dynamic_message_1 = new event_processing::DynamicMessage();
//
//  dynamic_message_1->set_field<std::string>("filter", "set_prefix_lookup_name_2", "prefix_2");
//  dynamic_message_1->set_field<std::string>("message_lookup_field_group",
//                                            "message_lookup_field_name",
//                                            "http://localhost:8080/oauth");
//
//  //sinks
//  MessageCollectorSink filtered{"filtered"};
//  MessageCollectorSink out{"out"};
//  MessageCollectorSink except{"except"};
//
//  blacklist_stage.get_source("out")->set_sink(&out);
//  blacklist_stage.get_source("filtered")->set_sink(&filtered);
//  blacklist_stage.get_source("exception_out")->set_sink(&except);
//
//  dynamic_message_1->set_field("testing", "key", "my key 1");
//
//  EXPECT_NO_THROW(blacklist_stage.get_sink("message_in")->on_message(dynamic_message_1));
//  EXPECT_EQ(1, out.get_received_messages().size());
//  EXPECT_NE(1, filtered.get_received_messages().size());
//  EXPECT_EQ(0, except.get_received_messages().size());
//
//  const auto *received_msg = static_cast<event_processing::DynamicMessage *>(*out.get_received_messages().begin());
//
//  EXPECT_EQ(received_msg->template get_field<std::string>("testing", "key"), "my key 1");
//
//  std::cout << "................................................. " << std::endl;
//
//}

}
