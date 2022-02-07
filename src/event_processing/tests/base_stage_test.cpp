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

#include <exception>
#include <gtest/gtest.h>

#include "../base_impl/base_stage.h"

namespace adl::axp::core::event_processing::test {

namespace {
class ProtectedAccessor : public details::BaseStage {
 public:
  ProtectedAccessor() : details::BaseStage("tetst") {}

  virtual void register_source(const std::string &source_name, ISource *source) {
    details::BaseStage::register_source(source_name, source);
  }
  virtual void register_sink(const std::string &sink_name, ISink *sink) {
    details::BaseStage::register_sink(sink_name, sink);
  }
};
}

TEST(stage, name) {
  details::BaseStage stage("stage 1");
  EXPECT_EQ("stage 1", stage.get_name());
}

TEST(stage, get_sink) {
  ProtectedAccessor stage;
  EXPECT_THROW(stage.get_sink("test"), details::SinkDoesNotExistException);

  auto *sink_1 = reinterpret_cast<ISink *>(555352314);
  auto *sink_2 = reinterpret_cast<ISink *>(213124);

  stage.register_sink("s1", sink_1);
  stage.register_sink("s2", sink_2);

  EXPECT_THROW(stage.get_sink("test"), details::SinkDoesNotExistException);
  EXPECT_EQ(sink_1, stage.get_sink("s1"));
  EXPECT_EQ(sink_2, stage.get_sink("s2"));
}

TEST(stage, register_sink_ex) {
  ProtectedAccessor stage;

  auto *sink_1 = reinterpret_cast<ISink *>(555352314);
  auto *sink_2 = reinterpret_cast<ISink *>(3721394);

  EXPECT_NO_THROW(stage.register_sink("s1", sink_1));
  EXPECT_THROW(stage.register_sink("s1", sink_2), details::SinkExistException);
  EXPECT_NO_THROW(stage.register_sink("s2", sink_2));
}

TEST(stage, get_sink_const) {
  ProtectedAccessor stage;
  EXPECT_THROW(stage.get_sink("test"), details::SinkDoesNotExistException);

  auto *sink_1 = reinterpret_cast<ISink *>(555352314);
  auto *sink_2 = reinterpret_cast<ISink *>(213124);

  stage.register_sink("s1", sink_1);
  stage.register_sink("s2", sink_2);

  // to call get_sink in a const context
  auto runner = [sink_1, sink_2](const ProtectedAccessor &stage2) {
    EXPECT_THROW(stage2.get_sink("test"), details::SinkDoesNotExistException);
    EXPECT_EQ(sink_1, stage2.get_sink("s1"));
    EXPECT_EQ(sink_2, stage2.get_sink("s2"));
  };

  runner(stage);
}

TEST(stage, register_source_ex) {
  ProtectedAccessor stage;

  auto *source_1 = reinterpret_cast<ISource *>(555352314);
  auto *source_2 = reinterpret_cast<ISource *>(213124);

  EXPECT_NO_THROW(stage.register_source("s1", source_1));
  EXPECT_THROW(stage.register_source("s1", source_2), details::SourceExistException);
  EXPECT_NO_THROW(stage.register_source("s2", source_2));
}

TEST(stage, get_source) {
  ProtectedAccessor stage;
  EXPECT_THROW(stage.get_source("test"), details::SourceDoesNotExistException);

  auto *source_1 = reinterpret_cast<ISource *>(555352314);
  auto *source_2 = reinterpret_cast<ISource *>(213124);

  stage.register_source("s1", source_1);
  stage.register_source("s2", source_2);

  EXPECT_THROW(stage.get_source("test"), details::SourceDoesNotExistException);
  EXPECT_EQ(source_1, stage.get_source("s1"));
  EXPECT_EQ(source_2, stage.get_source("s2"));
}

TEST(stage, get_source_const) {
  ProtectedAccessor stage;
  EXPECT_THROW(stage.get_source("test"), details::SourceDoesNotExistException);

  auto *source_1 = reinterpret_cast<ISource *>(555352314);
  auto *source_2 = reinterpret_cast<ISource *>(213124);

  stage.register_source("s1", source_1);
  stage.register_source("s2", source_2);

  // to call get_sink in a const context
  auto runner = [source_1, source_2](const ProtectedAccessor &stage2) {
    EXPECT_THROW(stage2.get_source("test"), details::SourceDoesNotExistException);
    EXPECT_EQ(source_1, stage2.get_source("s1"));
    EXPECT_EQ(source_2, stage2.get_source("s2"));
    EXPECT_EQ(source_2, stage2.get_source("s2"));
  };

  runner(stage);
}

TEST(stage, start) {
  details::BaseStage stage("stage");
  ASSERT_NO_THROW(stage.start());
}

}