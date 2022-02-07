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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-11-10.
 */

#include <gtest/gtest.h>

#include "../message_augmentor_stage/message_augmentor.h"
#include "../message_augmentor_stage/augmentor_definitions.h"
#include "../message_augmentor_stage/message_augmentor_stage.h"

namespace adl::axp::core::stages::message_augmentor::tests {

TEST(ConcatnatingAugmentor, smoke) {
  ConcatenatingAugmentorDefs::Config cfg{
      {
          {"grp_1", "field_1"},
          {"grp_1", "field_2"},
          {"grp_2", "field_1"}
      },
      "|",
      "!!",
      "!!!",
      "target_grp",
      "target_name"
  };
  MessageAugmentor<ConcatenatingAugmentorDefs::Config>
      aug{"my aug", "fun aug", cfg, ConcatenatingAugmentorDefs::augment_operator};

  event_processing::DynamicMessage msg;
  msg
      .set_field("grp_1", "field_1", "this")
      .set_field("grp_1", "field_2", "is")
      .set_field("grp_2", "field_1", "fun");

  aug(&msg);

  EXPECT_EQ("!!|this|is|fun|!!!", msg.get_field<std::string>("target_grp", "target_name"));
}

TEST(AugmentorStage, smoke) {
  ConcatenatingAugmentorDefs::Config cfg_concat{
      {
          {"grp_1", "field_1"},
          {"grp_1", "field_2"},
          {"grp_2", "field_1"}
      },
      "|",
      "!!",
      "!!!",
      "target_grp",
      "target_name"
  };
  MessageAugmentor<ConcatenatingAugmentorDefs::Config>
      aug_1{"my aug", "fun aug", cfg_concat, ConcatenatingAugmentorDefs::augment_operator};

  TokenExtractingAugmentorDefs::Config cfg_token{
      {"grp_4", "field_1"},
      '[',
      ']',
      5,
      {{"grp_10", "fld_1"}, {"grp_10", "fld_2"}, {"grp_10", "fld_3"}}
  };

  MessageAugmentor<TokenExtractingAugmentorDefs::Config>
      aug_2{"my aug 2", "fun again", cfg_token, TokenExtractingAugmentorDefs::augment_operator};

  std::list<MessageAugmentorBase *> augmentors{&aug_1, &aug_2};
  MessageAugmentorStage stage("test", augmentors.begin(), augmentors.end());

  //

  event_processing::DynamicMessage msg;
  msg
      .set_field("grp_1", "field_1", "this")
      .set_field("grp_1", "field_2", "is")
      .set_field("grp_2", "field_1", "fun")
      .set_field("grp_4", "field_1", "foo/[xxx] yyy [v]bar/test/[zzzz]");

  class DummySink : public event_processing::ISink {
    void on_message(event_processing::IMessage *message) noexcept override {}
  } dummy_sink;

  stage.get_source("message_out")->set_sink(&dummy_sink);
  stage.on_message(&msg);

  EXPECT_EQ("!!|this|is|fun|!!!", msg.get_field<std::string>("target_grp", "target_name"));
  EXPECT_EQ("xxx", msg.get_field<std::string>("grp_10", "fld_1"));
  EXPECT_EQ("v", msg.get_field<std::string>("grp_10", "fld_2"));
  EXPECT_EQ("zzzz", msg.get_field<std::string>("grp_10", "fld_3"));
}
}
