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

#include <unordered_map>
#include <functional>

#include "../../services/store_services/key_value_store.h"
#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"
#include "../../event_processing/dynamic_message.h"

namespace adl::axp::core::stages::load_balancers {

// thread safe - no
class RoundRobinLoadBalancerStage : public event_processing::details::BaseStage,
                                    public event_processing::ISink {
 public:
  RoundRobinLoadBalancerStage(std::string_view name,
                              const std::pair<std::string, std::string> &pool_lookup_field,
                              const services::store_services::KeyValueStore &store) :
      event_processing::details::BaseStage(name),
      _pool_lookup_field(pool_lookup_field),
      _store(store),
      _last_selection_index() {

    register_sink("in", this);
    register_source("out", &_out);
  }

  void on_message(event_processing::IMessage *message) noexcept override {
    try {
      auto *dynamic_message = static_cast<core::event_processing::DynamicMessage *>(message);
      const auto &backend_pool_key =
          dynamic_message->get_field<std::string>(_pool_lookup_field.first, _pool_lookup_field.second);

      const uint32_t end_point_count = _store.get_list_count(backend_pool_key);
      auto &index = _last_selection_index[backend_pool_key];
      ++index;
      index = index % end_point_count;
      const auto &endpoint_key = _store.get_at_list<std::string>(backend_pool_key, index);
      const auto &endpoint_host = _store.get<std::string>(endpoint_key + "/host");
      const auto &endpoint_port = _store.get<std::string>(endpoint_key + "/port");

      dynamic_message->template set_field<std::string>("load_balancing", "endpoint_host", endpoint_host);
      dynamic_message->template set_field<std::string>("load_balancing", "endpoint_port", endpoint_port);

      _out.get_sink()->on_message(message);
    } catch (const std::exception &ex) {

    }
  }

 private:
  const std::pair<std::string, std::string> _pool_lookup_field;
  const services::store_services::KeyValueStore &_store;

  // state
  std::unordered_map<std::string, uint8_t> _last_selection_index; // 255 max supported

  event_processing::details::BaseSource _out;
};

}