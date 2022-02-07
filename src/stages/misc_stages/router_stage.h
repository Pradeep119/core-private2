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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-02.
 */

#pragma once

#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"

namespace adl::axp::core::stages {

/**
 * Simple two way router.
 * Cascade these to get more outgoing paths
 * messages which dont match given id's are emitted in 'remaining_source'
 *
 * @tparam message_type_id message type for for source_1
 */
template<size_t message_type_id>
class TwoWayRouterStage : public event_processing::details::BaseStage,
                          public event_processing::ISink {
 public:
  TwoWayRouterStage(std::string_view name) : event_processing::details::BaseStage(name) {
    register_source("source_1", &_source_1);
    register_source("remaining_source", &_remaining_source);

    register_sink("message_in", this);
  }

  virtual void on_message(event_processing::IMessage *message) noexcept {
    if (message_type_id == message->message_type()) {
      _source_1.get_sink()->on_message(message);
    } else {
      _remaining_source.get_sink()->on_message(message);
    }
  }

 private:
  event_processing::details::BaseSource _source_1;
  event_processing::details::BaseSource _remaining_source;
};

/**
 * Simple three way router.
 * Cascade these to get more outgoing paths
 * messages which dont match given id's are emitted in 'remaining_source'
 *
 * @tparam message_type_1_id message type for first source
 * @tparam message_type_2_id message type for second source
 */
template<size_t message_type_1_id, size_t message_type_2_id>
class ThreeWayRouterStage : public event_processing::details::BaseStage,
                            public event_processing::ISink {
 public:
  ThreeWayRouterStage(std::string_view name) : event_processing::details::BaseStage(name) {
    register_source("source_1", &_source_1);
    register_source("source_2", &_source_2);
    register_source("remaining_source", &_remaining_source);

    register_sink("message_in", this);
  }

  virtual void on_message(event_processing::IMessage *message) noexcept {
    if (message_type_1_id == message->message_type()) {
      _source_1.get_sink()->on_message(message);
    } else if (message_type_2_id == message->message_type()) {
      _source_2.get_sink()->on_message(message);
    } else {
      _remaining_source.get_sink()->on_message(message);
    }
  }

 private:
  event_processing::details::BaseSource _source_1;
  event_processing::details::BaseSource _source_2;
  event_processing::details::BaseSource _remaining_source;
};

/**
 * Simple four way router.
 * Cascade these to get more outgoing paths
 * messages which dont match given id's are emitted in 'remaining_source'
 *
 * @tparam message_type_1_id message type for first source
 * @tparam message_type_2_id message type for second source
 * @tparam message_type_3_id message type for second source
 */
template<size_t message_type_1_id, size_t message_type_2_id, size_t message_type_3_id>
class FourWayRouterStage : public event_processing::details::BaseStage,
                           public event_processing::ISink {
 public:
  FourWayRouterStage(std::string_view name) : event_processing::details::BaseStage(name) {
    register_source("source_1", &_source_1);
    register_source("source_2", &_source_2);
    register_source("source_3", &_source_3);
    register_source("remaining_source", &_remaining_source);

    register_sink("message_in", this);
  }

  virtual void on_message(event_processing::IMessage *message) noexcept {
    if (message_type_1_id == message->message_type()) {
      _source_1.get_sink()->on_message(message);
    } else if (message_type_2_id == message->message_type()) {
      _source_2.get_sink()->on_message(message);
    } else if (message_type_3_id == message->message_type()) {
      _source_3.get_sink()->on_message(message);
    } else {
      _remaining_source.get_sink()->on_message(message);
    }
  }

 private:
  event_processing::details::BaseSource _source_1;
  event_processing::details::BaseSource _source_2;
  event_processing::details::BaseSource _source_3;
  event_processing::details::BaseSource _remaining_source;
};

}
