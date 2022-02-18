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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-24.
*/

#pragma once

#include "../../event_processing/sink.h"

/**
 * This sink swallows the incoming message
 */
namespace adl::axp::core::stages::misc { //TODO: change namespace

class NoOpSink : public event_processing::ISink {
 public:
  void on_message(event_processing::IMessage *message) noexcept override {

  }
};
}