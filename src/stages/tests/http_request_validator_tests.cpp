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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-21.
 * @author Braveenan Sritharan (braveenan.sritharan@axiatadigitallabs.com) on 2021-11-19.
 */

#include <gtest/gtest.h>

#include "../../services/store_services/key_value_store.h"
#include "../../services/store_services/key_complex_value_store.h"
#include "../../event_processing/sink.h"
#include "../http_validation_stages/request_validation_stage.h"

namespace adl::axp::core::stages::http::validation::test {

/**Message collector class*/
namespace {
class MessageCollectorSink : public event_processing::ISink {
 public:
  MessageCollectorSink(std::string_view name) : _name(name) {
  }

  void on_message(event_processing::IMessage *message) noexcept override {
    _received_messages.emplace_back(message);
    std::cout << "source name : " << _name << std::endl;
  }

  std::vector<event_processing::IMessage *> get_received_messages() {
    return _received_messages;
  }

 private:
  std::vector<event_processing::IMessage *> _received_messages;
  const std::string _name;
};
}

TEST(header_validator, compulsary_validate) {

  std::list<std::pair<std::string, std::string>> headers = {{"h1", "34"}, {"h2", "v2"}, {"h3", "54"}, {"h5", "v5"}};

  event_processing::DynamicMessage msg;
  msg.create_typed_group<std::string>("http_headers");

  for (auto ite = headers.begin(); ite != headers.end(); ++ite) {
    const auto&[key, value] = *ite;
    msg.template set_typed_field("http_headers", key, std::string(value));
  }

  auto ite_pair = msg.get_typed_group<std::string>("http_headers");

  HeaderSectionValidator val_1;
  val_1.add_compulsory_validator("h9");
  EXPECT_EQ(val_1.validate(ite_pair.first, ite_pair.second), false);

  HeaderSectionValidator val_2;
  val_2.add_compulsory_validator("h1");
  EXPECT_EQ(val_2.validate(ite_pair.first, ite_pair.second), true);

}

TEST(header_validator, int_datatype_validate) {

  std::list<std::pair<std::string, std::string>> headers = {{"h1", "34"}, {"h2", "v2"}, {"h3", "55555"}, {"h5", "v5"}};

  event_processing::DynamicMessage msg;
  msg.create_typed_group<std::string>("http_headers");

  for (auto ite = headers.begin(); ite != headers.end(); ++ite) {
    const auto&[key, value] = *ite;
    msg.template set_typed_field("http_headers", key, std::string(value));
  }

  auto ite_pair = msg.get_typed_group<std::string>("http_headers");

  HeaderSectionValidator val_1;
  val_1.add_data_type_validator<int64_t>("h5");
  EXPECT_EQ(val_1.validate(ite_pair.first, ite_pair.second), false);

  HeaderSectionValidator val_2;
  val_2.add_data_type_validator<int64_t>("h1");
  EXPECT_EQ(val_2.validate(ite_pair.first, ite_pair.second), true);

}

TEST(header_validator, double_datatype_validate) {

  std::list<std::pair<std::string, std::string>>
      headers = {{"h1", "34.8"}, {"h2", "3.r"}, {"h3", "54.7."}, {"h5", "56"}};

  event_processing::DynamicMessage msg;
  msg.create_typed_group<std::string>("http_headers");

  for (auto ite = headers.begin(); ite != headers.end(); ++ite) {
    const auto&[key, value] = *ite;
    msg.template set_typed_field("http_headers", key, std::string(value));
  }

  auto ite_pair = msg.get_typed_group<std::string>("http_headers");

  HeaderSectionValidator val_1;
  val_1.add_data_type_validator<double>("h2");
  EXPECT_EQ(val_1.validate(ite_pair.first, ite_pair.second), false);

  HeaderSectionValidator val_2;
  val_2.add_data_type_validator<double>("h1");
  EXPECT_EQ(val_2.validate(ite_pair.first, ite_pair.second), true);

  HeaderSectionValidator val_3;
  val_3.add_data_type_validator<double>("h3");
  EXPECT_EQ(val_3.validate(ite_pair.first, ite_pair.second), false);

  HeaderSectionValidator val_4;
  val_4.add_data_type_validator<double>("h5");
  EXPECT_EQ(val_4.validate(ite_pair.first, ite_pair.second), false);

}

TEST(query_validator, compulsary_validate) {
  std::list<std::pair<std::string, std::string>> query = {{"q1", "78"}, {"q2", "v8"}, {"q3", "67"}, {"q5", "v3"}};

  event_processing::DynamicMessage msg;
  msg.create_typed_group<std::string>("query_parameters");

  for (auto ite = query.begin(); ite != query.end(); ++ite) {
    const auto&[key, value] = *ite;
    msg.template set_typed_field("query_parameters", key, std::string(value));
  }

  auto ite_pair = msg.get_typed_group<std::string>("query_parameters");

  QueryValidator val_1;
  val_1.add_compulsory_validator("q1");
  EXPECT_EQ(val_1.validate(ite_pair.first, ite_pair.second), true);

  QueryValidator val_2;
  val_2.add_compulsory_validator("q8");
  EXPECT_EQ(val_2.validate(ite_pair.first, ite_pair.second), false);

}

TEST(query_validator, int_datatype_validate) {

  std::list<std::pair<std::string, std::string>> query = {{"q1", "78"}, {"q2", "v8"}, {"q3", "67"}, {"q5", "v3"}};

  event_processing::DynamicMessage msg;
  msg.create_typed_group<std::string>("query_parameters");

  for (auto ite = query.begin(); ite != query.end(); ++ite) {
    const auto&[key, value] = *ite;
    msg.template set_typed_field("query_parameters", key, std::string(value));
  }

  auto ite_pair = msg.get_typed_group<std::string>("query_parameters");

  QueryValidator val_1;
  val_1.add_data_type_validator<int64_t>("q1");
  EXPECT_EQ(val_1.validate(ite_pair.first, ite_pair.second), true);

  QueryValidator val_2;
  val_2.add_data_type_validator<int64_t>("q2");
  EXPECT_EQ(val_2.validate(ite_pair.first, ite_pair.second), false);

}

TEST(query_validator, double_datatype_validate) {

  std::list<std::pair<std::string, std::string>> query = {{"q1", "78"}, {"q2", "567.6554"}, {"q3", "ty"}, {"q5", "v3"}};

  event_processing::DynamicMessage msg;
  msg.create_typed_group<std::string>("query_parameters");

  for (auto ite = query.begin(); ite != query.end(); ++ite) {
    const auto&[key, value] = *ite;
    msg.template set_typed_field("query_parameters", key, std::string(value));
  }

  auto ite_pair = msg.get_typed_group<std::string>("query_parameters");

  QueryValidator val_1;
  val_1.add_data_type_validator<double>("q1");
  EXPECT_EQ(val_1.validate(ite_pair.first, ite_pair.second), false);

  QueryValidator val_2;
  val_2.add_data_type_validator<double>("q2");
  EXPECT_EQ(val_2.validate(ite_pair.first, ite_pair.second), true);

}

TEST(http_validation_stage_test, all_validations) {

  std::cout << "................................................. " << std::endl;
  std::cout << "http validation test... " << std::endl;

  core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator>
      validation_spec_store;

  core::stages::http::validation::HttpRequestValidationStage
      http_request_validator_stage
      {"http request validator stage", validation_spec_store, {"http_info", "http_validator_spec_key"}, "http_headers",
       "path_parameters", "query_parameters"};

  MessageCollectorSink invalid{"Invalid Requests"};
  MessageCollectorSink valid{"Valid Requests Out"};
  http_request_validator_stage.get_source("valid_requests_out")->set_sink(&valid);
  http_request_validator_stage.get_source("invalid_requests_out")->set_sink(&invalid);


  //add validator_1
  validation_spec_store.add_value("validator_1", new core::stages::http::validation::RequestValidator());
  auto *request_spec_1 = validation_spec_store.get_value("validator_1");
  request_spec_1->_header_validator.add_compulsory_validator("h1");

  //dynamic message_1
  std::list<std::pair<std::string, std::string>> headers = {{"h1", "34"}, {"h2", "v2"}, {"h3", "54"}, {"h5", "v5"}};
  std::list<std::pair<std::string, std::string>> query = {{"q1", "78"}, {"q2", "v8"}, {"q3", "67"}, {"q5", "v3"}};
  std::list<std::pair<std::string, std::string>> paths = {{"p1", "34"}, {"p2", "v2"}, {"p3", "54"}, {"p5", "v5"}};

  auto *dynamic_message_1 = new event_processing::DynamicMessage();
  dynamic_message_1->set_field("http_info", "http_validator_spec_key", "validator_1");
  dynamic_message_1->template create_typed_group<std::string>("http_headers");
  for (const auto &ite : headers) {
    const auto&[key, value] = ite;
    dynamic_message_1->template set_typed_field("http_headers", key, std::string(value));
  }

  dynamic_message_1->template create_typed_group<std::string>("query_parameters");
  for (const auto &ite : query) {
    const auto&[key, value] = ite;
    dynamic_message_1->template set_typed_field("query_parameters", key, std::string(value));
  }

  dynamic_message_1->template create_typed_group<std::string>("path_parameters");
  for (const auto &ite : paths) {
    const auto&[key, value] = ite;
    dynamic_message_1->template set_typed_field("path_parameters", key, std::string(value));
  }

  EXPECT_NO_THROW(http_request_validator_stage.get_sink("in")->on_message(dynamic_message_1));
  EXPECT_EQ(1, valid.get_received_messages().size());
  EXPECT_EQ(0, invalid.get_received_messages().size());


  //add validator_2
  validation_spec_store.add_value("validator_2", new core::stages::http::validation::RequestValidator());
  auto *request_spec_2 = validation_spec_store.get_value("validator_2");
  request_spec_2->_header_validator.add_compulsory_validator("h1");

  //dynamic message_2
  std::list<std::pair<std::string, std::string>> headers_2 = {{"h11", "34"}, {"h2", "v2"}, {"h3", "54"}, {"h5", "v5"}};
  std::list<std::pair<std::string, std::string>> query_2 = {{"q1", "78"}, {"q2", "v8"}, {"q3", "67"}, {"q5", "v3"}};
  std::list<std::pair<std::string, std::string>> paths_2 = {{"p1", "34"}, {"p2", "v2"}, {"p3", "54"}, {"p5", "v5"}};

  auto *dynamic_message_2 = new event_processing::DynamicMessage();
  dynamic_message_2->set_field("http_info", "http_validator_spec_key", "validator_2");
  dynamic_message_2->template create_typed_group<std::string>("http_headers");
  for (const auto &ite : headers_2) {
    const auto&[key, value] = ite;
    dynamic_message_2->template set_typed_field("http_headers", key, std::string(value));
  }

  dynamic_message_2->template create_typed_group<std::string>("query_parameters");
  for (const auto &ite : query_2) {
    const auto&[key, value] = ite;
    dynamic_message_2->template set_typed_field("query_parameters", key, std::string(value));
  }

  dynamic_message_2->template create_typed_group<std::string>("path_parameters");
  for (const auto &ite : paths_2) {
    const auto&[key, value] = ite;
    dynamic_message_2->template set_typed_field("path_parameters", key, std::string(value));
  }

  EXPECT_NO_THROW(http_request_validator_stage.get_sink("in")->on_message(dynamic_message_2));
  EXPECT_EQ(1, valid.get_received_messages().size());
  EXPECT_EQ(1, invalid.get_received_messages().size());


  //add validator_3
  validation_spec_store.add_value("validator_3", new core::stages::http::validation::RequestValidator());
  auto *request_spec_3 = validation_spec_store.get_value("validator_3");
  request_spec_3->_header_validator.add_compulsory_validator("h1");
  request_spec_3->_path_validator.add_compulsory_validator("p3");
  request_spec_3->_query_validator.add_compulsory_validator("q5");
  request_spec_3->_header_validator.add_data_type_validator<int64_t>("h1");
  request_spec_3->_path_validator.add_data_type_validator<int64_t>("p3");
  request_spec_3->_query_validator.add_data_type_validator<int64_t>("q1");

  //dynamic message_3
  std::list<std::pair<std::string, std::string>> headers_3 = {{"h1", "34"}, {"h2", "v2"}, {"h3", "54"}, {"h5", "v5"}};
  std::list<std::pair<std::string, std::string>> query_3 = {{"q1", "78"}, {"q2", "v8"}, {"q3", "67"}, {"q5", "v3"}};
  std::list<std::pair<std::string, std::string>> paths_3 = {{"p1", "34"}, {"p2", "v2"}, {"p3", "54"}, {"p5", "v5"}};

  auto *dynamic_message_3 = new event_processing::DynamicMessage();
  dynamic_message_3->set_field("http_info", "http_validator_spec_key", "validator_3");
  dynamic_message_3->template create_typed_group<std::string>("http_headers");
  for (const auto &ite : headers_3) {
    const auto&[key, value] = ite;
    dynamic_message_3->template set_typed_field("http_headers", key, std::string(value));
  }

  dynamic_message_3->template create_typed_group<std::string>("query_parameters");
  for (const auto &ite : query_3) {
    const auto&[key, value] = ite;
    dynamic_message_3->template set_typed_field("query_parameters", key, std::string(value));
  }

  dynamic_message_3->template create_typed_group<std::string>("path_parameters");
  for (const auto &ite : paths_3) {
    const auto&[key, value] = ite;
    dynamic_message_3->template set_typed_field("path_parameters", key, std::string(value));
  }

  EXPECT_NO_THROW(http_request_validator_stage.get_sink("in")->on_message(dynamic_message_3));
  EXPECT_EQ(2, valid.get_received_messages().size());
  EXPECT_EQ(1, invalid.get_received_messages().size());

}

}
