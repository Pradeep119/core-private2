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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-20.
*/

#pragma once

#include <atomic>
#include <uWebSockets/App.h>
#include <cassert>
#include <string>

#include <opentelemetry/exporters/ostream/metrics_exporter.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/sdk/metrics/controller.h>
#include <opentelemetry/sdk/metrics/meter.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/ungrouped_processor.h>

#include "../../foundation/base_types.h"
#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"
#include "../../event_processing/runnable.h"
#include "config_messages.h"
#include "../../event_processing/dynamic_message.h"
#include "../../event_processing/messages/http_request_message.h"
#include "../../event_processing/messages/http_response_message.h"
#include "../../services/store_services/key_value_store.h"

namespace adl::axp::core::stages::http_server_stage {

/**
 * An http server stage which uses uWebSockets under the hood.
 * No uWebSocket related concepts are leaked through its interfaces.
 */

inline std::map<std::string, std::string> extract_query(std::string_view query_url) {
  size_t pos_and = 0;
  size_t pos_equal = 0;
  std::string query_pair;
  std::string key;
  std::string value;

  std::map<std::string, std::string> query_data;

  while (!query_url.empty()) {
    if (query_url.find('&') != std::string::npos) {
      pos_and = query_url.find('&');
      query_pair = query_url.substr(0, pos_and);
      query_url =
          query_url.substr(pos_and + 1, query_url.length() - 1);
    } else {
      query_pair = query_url.substr(0, query_url.length());
      query_url = "";
    }
    pos_equal = query_pair.find('=');
    key = query_pair.substr(0, pos_equal);
    value = query_pair.substr(pos_equal + 1, query_pair.length() - 1);
    query_data[key] = value;
  }
  return query_data;
}

inline std::vector<std::string> extract_path_parameters(std::string_view resource_path) {
  std::string_view delimiter = "/";
  std::vector<std::string> resource_path_words = {};
  auto resource_path_str = std::string(resource_path) + "/";

  size_t position = 0;
  while ((position = resource_path_str.find(delimiter)) != std::string::npos) {
    resource_path_words.push_back(resource_path_str.substr(0, position));
    resource_path_str.erase(0, position + delimiter.length());
  }

  std::vector<std::string> path_parameter_vector = {};
  for (auto word : resource_path_words) {
    if (word[0] == ':') {
      path_parameter_vector.push_back(word.substr(1));
    }
  }
  return path_parameter_vector;
}

template<class request_out_message_factory_t>
class HttpServerStage : public event_processing::details::BaseStage,
                        public event_processing::ISink {

 private:
  class ConfiguratorSink : public event_processing::ISink {
   public:
    ConfiguratorSink(HttpServerStage &server_stage) :
        _server_stage(server_stage) {
    }

    void on_message(event_processing::IMessage *message) noexcept override {
      AddRouteConfigMessage *add_route_config_message = static_cast<AddRouteConfigMessage *>(message);
      try {
        if (add_route_config_message->get_method() == foundation::HttpMethods::GET) {
          on_add_route_get(add_route_config_message);
        } else if (add_route_config_message->get_method() == foundation::HttpMethods::HEAD) {
          on_add_route_head(add_route_config_message);
        } else if (add_route_config_message->get_method() == foundation::HttpMethods::POST) {
          on_add_route_post(add_route_config_message);
        } else if (add_route_config_message->get_method() == foundation::HttpMethods::PUT) {
          on_add_route_put(add_route_config_message);
        } else if (add_route_config_message->get_method() == foundation::HttpMethods::DELETE) {
          on_add_route_delete(add_route_config_message);
        } else if (add_route_config_message->get_method() == foundation::HttpMethods::CONNECT) {
          on_add_route_connect(add_route_config_message);
        } else if (add_route_config_message->get_method() == foundation::HttpMethods::OPTIONS) {
          on_add_route_options(add_route_config_message);
        } else if (add_route_config_message->get_method() == foundation::HttpMethods::TRACE) {
          on_add_route_trace(add_route_config_message);
        } else {
          add_route_config_message->on_error(std::make_exception_ptr(std::runtime_error("Unknown http method type")));
        }

        add_route_config_message->on_complete();

      } catch (const std::exception &ex) {
        add_route_config_message->on_error(std::current_exception());
        return;
      } catch (...) {
        add_route_config_message->on_error(std::make_exception_ptr(std::runtime_error("Unknown error occurred")));
      }

      _server_stage._http_route_counter->add(1, _server_stage._kvite);
    }

   private:
    // specific add route config handlers
    void on_add_route_get(AddRouteConfigMessage *config_message) noexcept(false) {
      //TODO: handle placeholders in the resource path
      //TODO: what happens if we install same route multiple times? Check with the community
      const auto route_key_1 = config_message->get_route_key_1();
      const auto route_key_2 = config_message->get_route_key_2();
      const auto resource_path = config_message->get_resource();
      const auto method = config_message->get_method();
      const auto path_parameter_vector = extract_path_parameters(resource_path);
      const auto logger = _server_stage._logger;

      _server_stage._server->get(std::string(resource_path),
                                 [route_key_1,
                                     route_key_2,
                                     resource_path,
                                     method,
                                     path_parameter_vector,
                                     logger,
                                     this](uWS::HttpResponse<true> *uws_response,
                                           uWS::HttpRequest *uws_request) {
                                   logger->info("........A new Request.........");
                                   //print(uws_request);

//                                   _server_stage._http_request_counter->add(1, _server_stage._kvite);

                                   core::event_processing::messages::HttpRequestMessage
                                       *message = _server_stage._request_out_message_factory.create();

                                   // private fields (group start and ends with __. These must not be used by other stages.
                                   // Used by the stage based on the protocol
                                   message->template set_field("__HttpServerStage__", "uws_response", uws_response);

                                   message->template set_field<int32_t>("http_info",
                                                                        "method",
                                                                        method);

                                   message->template set_field<std::string>("http_info",
                                                                            "route_key_1",
                                                                            std::string(route_key_1));
                                   message->template set_field<std::string>("http_info",
                                                                            "route_key_2",
                                                                            std::string(route_key_2));

                                   message->template set_field<std::string>("http_info",
                                                                            "resource_path",
                                                                            std::string(resource_path));


                                   //TODO: move to a function, and handle other resolutions
                                   auto to_millies = [](const timespec &time_stamp) {
                                     return time_stamp.tv_nsec / 1'000'000 + time_stamp.tv_sec * 1000;
                                   };

                                   timespec time_stamp;
                                   clock_gettime(CLOCK_REALTIME, &time_stamp);
                                   size_t time_stamp_millies = to_millies(time_stamp);

                                   message->template set_field<int64_t>("info",
                                                                        "request_time",
                                                                        time_stamp_millies);

                                   message->template create_typed_group<std::string>("http_headers");

                                   for (auto ite = uws_request->begin(); ite != uws_request->end(); ++ite) {
                                     const auto&[key, value] = *ite;
                                     message->template set_typed_field("http_headers", key, std::string(value));
                                   }

                                   message->template create_typed_group<std::string>("query_parameters");

                                   const auto &query_map = extract_query(uws_request->getQuery());
                                   for (const auto &ite : query_map) {
                                     const auto&[key, value] = ite;
                                     message->template set_typed_field("query_parameters", key, std::string(value));
                                   }

                                   message->template create_typed_group<std::string>("path_parameters");
                                   int path_index = 0;
                                   for (const auto ite : path_parameter_vector) {
                                     message->template set_typed_field("path_parameters",
                                                                       ite,
                                                                       std::string(uws_request->getParameter(
                                                                           path_index)));
                                     ++path_index;
                                   }

                                   message->template set_field<std::string>("http_info",
                                                                            "sender_ip_6",
                                                                            std::string(uws_response->getRemoteAddressAsText()));

                                   message->template set_field<std::string>("http_info",
                                                                            "sender_user_agent",
                                                                            std::string(uws_request->getHeader(
                                                                                "user-agent")));

                                   message->template set_field<std::string>("http_info",
                                                                            "access_token",
                                                                            std::string(uws_request->getHeader(
                                                                                "authorization")));

                                   // TODO: params and body
                                   uws_response->onWritable([uws_response](int offset) {
                                     return true;
                                   })->onAborted([logger] {
                                     logger->warn("On aborted");
                                   })->onData([this, message](std::string_view data, bool last) {
                                     //      std::cout << data << last << std::endl;
                                     //TODO: append the message with new data
                                     //TODO: Handling large payloads
                                     if (last) {
                                       _server_stage._http_request_out.get_sink()->on_message(message);
                                     }
                                   });
                                 });
    }

    void on_add_route_head(AddRouteConfigMessage *config_message) noexcept(false) {

    }
    void on_add_route_post(AddRouteConfigMessage *config_message) noexcept(false) {

    }
    void on_add_route_put(AddRouteConfigMessage *config_message) noexcept(false) {

    }
    void on_add_route_delete(AddRouteConfigMessage *config_message) noexcept(false) {

    }
    void on_add_route_connect(AddRouteConfigMessage *config_message) noexcept(false) {

    }
    void on_add_route_options(AddRouteConfigMessage *config_message) noexcept(false) {

    }
    void on_add_route_trace(AddRouteConfigMessage *config_message) noexcept(false) {

    }

    // TODO: See if we can get this feature from uWS
    // specific remove route config handlers

   private:
    HttpServerStage &_server_stage;
    std::shared_ptr<spdlog::logger> logger;
  };

 public:

  HttpServerStage(std::string_view name,
                  std::string_view key_file,
                  std::string_view cert_file,
                  std::string_view pass_phrase,
                  int32_t port,
                  request_out_message_factory_t &request_out_message_factory) :
      event_processing::details::BaseStage(name),
      _configuration_in(*this),
      _key_file(key_file),
      _cert_file(cert_file),
      _pass_phrase(pass_phrase),
      _port(port),
      _request_out_message_factory(request_out_message_factory) {

    register_source("http_request_out", &_http_request_out);
    register_sink("http_response_in", this);

    register_sink("configuration_in", &_configuration_in);
    _logger = spdlog::get(std::string(name));
  }

  // This method MUST be called from the event loop thread
  // applicable for this stage
  void start() override {
    _server = new uWS::SSLApp({
                                  .key_file_name = _key_file.c_str(),
                                  .cert_file_name = _cert_file.c_str(),
                                  .passphrase = _pass_phrase.c_str()
                              });

    _server->listen(_port, [this](auto *listen_socket) {
      if (listen_socket) {
        _logger->info("Listening on port {}", _port);
      }
    });

    // retrieves the event loop associated with this stage.
    _event_loop = uWS::Loop::get();

    auto provider = opentelemetry::metrics::Provider::GetMeterProvider();
    auto meter = provider->GetMeter("Http Server Stage");
    _http_route_counter =
        meter->NewIntUpDownCounter(std::string(BaseStage::get_name()) + "__installed_http_routes",
                                   "Total number of routes installed",
                                   "inch",
                                   true);
    _http_request_counter =
        meter->NewIntUpDownCounter(std::string(BaseStage::get_name()) + "__total_http_req",
                                   "Total number of http requests received",
                                   "inch",
                                   true);

    _labels.template emplace(std::string(BaseStage::get_name()) + "_key",
                             std::string(BaseStage::get_name()) + "_value");
  }

  void on_message(core::event_processing::IMessage *message) noexcept override {

    const core::event_processing::messages::HttpResponseMessage
        *response = static_cast<const core::event_processing::messages::HttpResponseMessage *>(message);
    const core::event_processing::messages::HttpRequestMessage &request =
        reinterpret_cast<const core::event_processing::messages::HttpRequestMessage &>(response->get_request_message());
    const uWS::HttpResponse<true>
        *res = reinterpret_cast<const uWS::HttpResponse<true> *>(request.template get_field<void *>(
        "__HttpServerStage__",
        "uws_response"));

    auto *uws_reponse = const_cast<uWS::HttpResponse<true> *>(res);
    if (response->get_return_code() == 500) {
      //TODO: remove hardcoding
      uws_reponse->writeStatus("429 Too Many Requests");
      uws_reponse->end(response->get_body());
//      _logger->info("sent {} response ",500);
    } else {
      uws_reponse->end(response->get_body());
    }
  }

 protected:
  std::shared_ptr<spdlog::logger> _logger;

 private:
// sources and sinks

  event_processing::details::BaseSource _http_request_out;
  ConfiguratorSink _configuration_in;
  uWS::SSLApp *_server;

//
  const std::string _key_file;
  const std::string _cert_file;
  const std::string _pass_phrase;
  const int32_t _port;

  request_out_message_factory_t &_request_out_message_factory;

// the event loop attached to this stage
  uWS::Loop *_event_loop;

  std::atomic_bool _stop = false;

// observability
  opentelemetry::nostd::shared_ptr<opentelemetry::metrics::UpDownCounter<int>> _http_route_counter;
  opentelemetry::nostd::shared_ptr<opentelemetry::metrics::UpDownCounter<int>> _http_request_counter;

  std::map<std::string, std::string> _labels;
  opentelemetry::common::KeyValueIterableView<decltype(_labels)> _kvite{_labels};

  friend class ConfiguratorSink;
};
}
