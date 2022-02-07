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

#include "../../event_processing/dynamic_message.h"

namespace adl::axp::core::stages::message_augmentor {

/**
 */
class MessageAugmentorBase {
 public:
  virtual ~MessageAugmentorBase() = default;
  virtual void operator()(event_processing::DynamicMessage *message) const noexcept(false) = 0;
};

/**
 * Represents an augmentation
 */
template<typename configuration_t>
class MessageAugmentor : public MessageAugmentorBase {
 public:
  MessageAugmentor(std::string_view name,
                   std::string_view description,
                   const configuration_t &configuration,
                   const std::function<void(const configuration_t &configuration,
                                            event_processing::DynamicMessage *)> &augment_operator) :
      _name(name),
      _description(description),
      _configuration(configuration),
      _augment_operator(augment_operator) {
  }

  std::string_view get_name() const { return _name; }
  std::string_view get_description() const { return _description; }
  const std::function<void(event_processing::DynamicMessage *)> &get_augment_function() const { return _augment_operator; }

  void operator()(event_processing::DynamicMessage *message) const noexcept(false) override {
    _augment_operator(_configuration, message);
  }

 private:
  const std::string _name;
  const std::string _description;
  const configuration_t _configuration;
  std::function<void(const configuration_t &configuration, event_processing::DynamicMessage *)> _augment_operator;
};
}