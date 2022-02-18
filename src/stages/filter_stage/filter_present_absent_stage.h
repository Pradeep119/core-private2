/*
 *
 *   © Copyrights 2021 Axiata Digital Labs Pvt Ltd.
 *   All Rights Reserved.
 *
 *   These material are unpublished, proprietary, confidential source
 *   code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
 *   SECRET of ADL.
 *
 *   ADL retains all title to and intellectual property rights in these
 *   materials.
 *
 *
 *   @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on 19/01/22.
 *
 */


#pragma once

#include <unordered_map>
#include <functional>
#include <list>
#include <iostream>

#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"
#include "../../event_processing/dynamic_message.h"
#include "../../services/store_services/key_value_store.h"

namespace adl::axp::core::stages::filter_stage {

template<bool if_present>
class FilterPresentAbsentStage :
    public event_processing::details::BaseStage,
    public event_processing::ISink {
 public:
  using field_combination_t = std::list<std::pair<std::pair<std::string, std::string>,
                                                  std::pair<std::string, std::string>>>;

  FilterPresentAbsentStage(std::string_view name,
                           const services::store_services::KeyValueStore &store,
                           field_combination_t &field_combinations)
      :
      event_processing::details::BaseStage(name),
      _store(store),
      _field_combinations(field_combinations) {

    register_sink("message_in", this);
    register_source("out", &_out);
    register_source("filtered", &_filtered);
    register_source("exception_out", &_exception_out);
    _logger = spdlog::get(std::string(name));
  }

  void on_message(event_processing::IMessage *message) noexcept {

    _logger->info("running {}", (if_present ? "whitelisting " : "blacklisting"));

    auto *dynamic_message = static_cast<core::event_processing::DynamicMessage *>(message);
    try {
      bool filtered = false;
      for (const auto &field_combination : _field_combinations) {

        const auto &[set_lookup_field, value_lookup_field] = field_combination;

        const auto &set_key = dynamic_message->get_field<std::string>(set_lookup_field.first, set_lookup_field.second);
        const auto
            &value = dynamic_message->get_field<std::string>(value_lookup_field.first, value_lookup_field.second);

        const auto &set_exist = _store.set_key_exist(set_key);
        if (!set_exist) {
          // no set found by the key. In this case the actual verification
          // is disabled for both present and absent cases.
          continue;
        }

        // set by the key exists
        const auto &value_in_set = _store.contains_in_set<std::string>(set_key, std::string(value));

        // if_present =1 >> whitelist
        if (if_present && !value_in_set) {
          _logger->info("source name : filtered");
          filtered = true;
        } else if (!if_present && value_in_set) {
          _logger->info("source name : filtered");
          filtered = true;
        }
      }

      _logger->info((filtered ? "filtered" : "not filtered"));
      filtered ? _filtered.get_sink()->on_message(message) : _out.get_sink()->on_message(message);

    } catch (const core::services::store_services::KeyDoesNotExistException &e) {
      /**: KeyDoesNotExistException*/
      dynamic_message->template set_field<std::string>("exceptions",
                                                       "FilterPresentAbsentStage",
                                                       e.what());

      _logger->critical("source name : exception out", e.what());

      _exception_out.get_sink()->on_message(message);
    } catch (const std::exception &e) {
      dynamic_message->template set_field<std::string>("exceptions",
                                                       "FilterPresentAbsentStage",
                                                       e.what());
      _logger->critical("source name : exception out", e.what());
      _exception_out.get_sink()->on_message(message);
    }
  }

 private:
  const services::store_services::KeyValueStore &_store;

  event_processing::details::BaseSource _out;
  event_processing::details::BaseSource _filtered;
  event_processing::details::BaseSource _exception_out;

  const field_combination_t _field_combinations;
  std::shared_ptr<spdlog::logger> _logger;
};

using WhitelistingStage = FilterPresentAbsentStage<true>;
using BlacklistingStage = FilterPresentAbsentStage<false>;
}
