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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-17.
*/

#pragma once

#include "../../event_processing/message.h"
#include "../../event_processing/messages/async_config_message.h"

namespace adl::axp::core::stages::http_server_stage {

class AddRouteConfigMessage : public event_processing::messages::AsyncConfigurationMessage<void> {
 public:
  AddRouteConfigMessage(std::string_view route_tag_1,
                        std::string_view route_tag_2,
                        std::string_view resource,
                        foundation::HttpMethods method) :
      _route_tag_1(route_tag_1),
      _route_tag_2(route_tag_2),
      _resource(resource),
      _method(method) {}

  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::stages::http_server_stage::AddRouteConfigMessage");
  }

  std::string_view get_route_key_1() const noexcept { return _route_tag_1; }
  std::string_view get_route_key_2() const noexcept { return _route_tag_2; }
  std::string_view get_resource() const noexcept { return _resource; }
  foundation::HttpMethods get_method() const noexcept { return _method; }
 private:
  const std::string _route_tag_1;
  const std::string _route_tag_2;
  const std::string _resource;
  const foundation::HttpMethods _method;
};

}
