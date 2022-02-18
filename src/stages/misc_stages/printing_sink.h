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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-30.
*/

#pragma once

#include <string>
#include <iostream>

#include "../../event_processing/sink.h"

/**
 * This sink prints the message content
 */
namespace adl::axp::core::sinks {

class PrintingSink : public event_processing::ISink {
 public:
  PrintingSink(std::string_view prefix) : _prefix(prefix) {

  }

  void on_message(event_processing::IMessage *message) noexcept override {
    std::cout << _prefix << std::endl;
    //TODO: insert this to the logger
    //TODO: print message content
  }

 private:
  const std::string _prefix;
};
}