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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-27.
 */

#pragma once

#include "../../event_processing/sink.h"
#include "../../event_processing/messages/task_execution_message.h"

namespace adl::axp::core::sinks {
/**
 * This is a sink implementation, not a stage
 * This stage converts the incoming message to a TaskExecutionMessage and calls its execute method.
 * Thread safe - Yes given that TaskExecutionMessage is thread safe
 *
 * TODO: handle asynchrnous/synchrnous return values/errors
 */
class TaskExecutionSink : public core::event_processing::ISink {
 public:
  void on_message(core::event_processing::IMessage *message) noexcept override {
    auto *task_message = static_cast<core::event_processing::messages::TaskExecutionMessage *>(message);
    task_message->execute();
  }
};

}
