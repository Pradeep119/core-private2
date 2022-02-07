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

#pragma once

#include <functional>
#include <list>
#include <ranges>
#include <algorithm>

#include "message_augmentor.h"
#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/sink.h"

namespace adl::axp::core::stages::message_augmentor {

/**
 * Only works with DynamicMessage type (or derived)
 */
class MessageAugmentorStage : public event_processing::details::BaseStage,
                              public event_processing::ISink {
 public:
  template<typename augmentor_iterator_t>
  MessageAugmentorStage(std::string_view name, augmentor_iterator_t start, augmentor_iterator_t end) :
      BaseStage(name) {

    std::ranges::for_each(start, end, [this](auto &augmentor) {
      _augmentors.template emplace_back(augmentor);
    });

    register_source("message_out", &_message_out);
    register_sink("message_in", this);
  }

  void on_message(core::event_processing::IMessage *message) noexcept override {
    // TODO: handle exceptions
    auto *dynamic_message = static_cast<event_processing::DynamicMessage *>(message);
    for (const auto &augmentor: _augmentors) {
      (*augmentor)(dynamic_message);
    }
    _message_out.get_sink()->on_message(message);
  }

 private:
  event_processing::details::BaseSource _message_out;
  std::list<MessageAugmentorBase *> _augmentors;
};

}