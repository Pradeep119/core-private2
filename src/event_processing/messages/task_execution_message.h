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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-23.
 */


#pragma once

#include "../message.h"
#include <functional>

namespace adl::axp::core::event_processing::messages {

class TaskExecutionMessage : public event_processing::IMessage {
 public:
  TaskExecutionMessage(const std::function<void(void)> &execution_function) :
      _execution_function(execution_function) {}

  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::event_processing::messages::TaskExecutionMessage");
  }

  void execute() {
    _execution_function();
  }

 private:
  // return type of the task should be void
  std::function<void(void)> _execution_function;
};

}