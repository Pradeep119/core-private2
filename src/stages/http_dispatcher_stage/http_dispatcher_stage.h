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

#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>

#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"

#include "../../event_processing/messages/http_request_message.h"
#include "../../event_processing/messages/http_response_message.h"
#include "details/http_dropped_request_message.h"

#include "details/http_connection.h"
#include "details/http_connection_state_machine.h"

namespace net = boost::asio;
namespace http = beast::http;

namespace adl::axp::core::stages {

/**
 * This stage dispatches incoming http requests to external http servers
 * and relays the response received back.
 *
 * Thread safe: no
 */
template<class response_out_message_factory_t, class dropped_out_message_factory_t>
class HttpDispatcherStage : public event_processing::details::BaseStage,
                            public event_processing::ISink {

  struct HTTPConnectionKey {
    HTTPConnectionKey(bool https, std::string_view host, const std::string_view port) :
        _https(https),
        _host(host),
        _port(port) {
    }

    bool operator==(const HTTPConnectionKey &rhs) const {
      return (this->_host == rhs._host)
          && (this->_port == rhs._port)
          && (this->_https == rhs._https);
    }

    const bool _https;
    const std::string _host;
    const std::string _port;
  };

  struct HTTPConnectionKeyHash {

    template<class T>
    static inline void hash_combine(std::size_t &seed, const T &v) {
      std::hash<T> hasher;
      seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    std::size_t operator()(HTTPConnectionKey const &key) const noexcept {
      std::size_t seed = 0;

      hash_combine<std::string>(seed, key._host);
      hash_combine<std::string>(seed, key._port);
      hash_combine<bool>(seed, key._https);

      return seed;
    }
  };

  // thread safe: yes
  class HttpResponseAllocator {
   public:
    using pool_t = boost::object_pool<http::response<http::string_body>>;

    http::response<http::string_body> *allocate() {
      return _response_pool.construct();
    }

   private:
    pool_t _response_pool;
  };

 public:
  using http_connection_t = details::http_dispatcher::HTTPConnectionStateMachine<http::empty_body,
                                                                                 http::string_body,
                                                                                 HttpResponseAllocator>;
//  using http_request_message_t = adl::axp::core::event_processing::base_impl::HttpRequestMessage;
//  using http_response_message_t = adl::axp::core::event_processing::base_impl::HttpResponseMessage;
//  using http_dropped_message_t = base_impl::http_dispatcher::HttpDroppedRequestMessage;

  // TODO: use a builder
  HttpDispatcherStage(std::string_view name,
                      size_t connection_pool_size,
                      net::io_context &io_context,
                      response_out_message_factory_t &response_out_message_factory,
                      dropped_out_message_factory_t &dropped_out_message_factory) :
      event_processing::details::BaseStage(name),
      _http_connection_pool(connection_pool_size, HTTPConnectionKeyHash()),
      _io_context(io_context),
      _response_out_message_factory(response_out_message_factory),
      _dropped_out_message_factory(dropped_out_message_factory) {

    register_source("http_response_out", &_http_response_out);
    register_source("dropped_requests_out", &_dropped_requests_out);
    register_source("connection_status_out", &_connection_status_out);

    register_sink("http_request_in", this);
  }

  void start() override {
  }

  void on_message(event_processing::IMessage *message) noexcept {

    auto create_dropped_message = [this](event_processing::IMessage *request) {
      return request;
      // TODO:Why do we have to wrap it?
//      details::http_dispatcher::HttpDroppedRequestMessage* dropped_message = _dropped_out_message_factory.create();
//      dropped_message->set_dropped_message(*request);
//      return dropped_message;
    };

    auto perform_request =
        [this, create_dropped_message](http_connection_t *state_machine,
                                       core::event_processing::IMessage *request_message) {

          auto *request = request_from_message(_http_request_pool, *request_message);
          state_machine->write(*request, [this, state_machine, request, request_message, create_dropped_message]() {
            state_machine->read([this, request, request_message](http::response<http::string_body> *response) {
              _http_request_pool.free(request);
              core::event_processing::messages::HttpResponseMessage
                  *response_message = _response_out_message_factory.create();
              response_message->set_body(response->body());
              response_message->set_return_code(200);
              response_message->set_request_message(static_cast<core::event_processing::messages::HttpRequestMessage &>(*request_message));
              _http_response_out.get_sink()->on_message(response_message);

            }, [this, request, request_message, create_dropped_message](beast::error_code ec) {
              // read failure
              _http_request_pool.free(request);
              _dropped_requests_out.get_sink()->on_message(create_dropped_message(request_message));

            });
          }, [this, request, request_message, create_dropped_message](beast::error_code ec) {
            // write failure
            _http_request_pool.free(request);
            _dropped_requests_out.get_sink()->on_message(create_dropped_message(request_message));

          });
        };

    auto handler = [this, message, perform_request, create_dropped_message] {
      auto *request_message = static_cast<core::event_processing::DynamicMessage *>(message);
      const auto
          &host = request_message->get_field<std::string>("load_balancing", "endpoint_host");
      const auto
          &port = request_message->get_field<std::string>("load_balancing", "endpoint_port");

      HTTPConnectionKey
          key(false, host, port);
      auto ite = _http_connection_pool.find(key);

      if (ite == _http_connection_pool.end()) {
        // no existing connections to satisfy the request.
        // schedule a new connection asynchronously
        auto *connection =
            new typename http_connection_t::connection_t(_io_context, key._host, key._port, _response_allocator);
        auto *state_machine = new http_connection_t(connection);
        _http_connection_pool.insert(typename http_connection_pool_t::value_type(key, state_machine));
        state_machine->resolve([this, state_machine, request_message, perform_request, create_dropped_message]() {
          state_machine->connect([this, request_message, state_machine, perform_request, create_dropped_message]() {
            // connected, do the request
            perform_request(state_machine, request_message);

          }, [this, request_message, create_dropped_message](beast::error_code ec) {
            // connect failure
            _dropped_requests_out.get_sink()->on_message(create_dropped_message(request_message));
          });
        }, [this, request_message, create_dropped_message](beast::error_code ec) {
          // resolve failure
          _dropped_requests_out.get_sink()->on_message(create_dropped_message(request_message));
        });
      } else {
        // we have a working connection
        // depending on the state, we can perform a request

        auto *state_machine = ite->second;

        if (http_connection_t::writing_and_reading == state_machine->current_state() ||
            http_connection_t::reading == state_machine->current_state() ||
            http_connection_t::writing == state_machine->current_state() ||
            http_connection_t::connected == state_machine->current_state()) {

          perform_request(state_machine, request_message);
        } else if (http_connection_t::resolved
            == state_machine->current_state()) {
          // we need to re-connect and send the request
          state_machine->connect([this, state_machine, request_message, perform_request]() {
            // connected, do the request
            perform_request(state_machine, request_message);

          }, [this, request_message, create_dropped_message](beast::error_code) {
            // connect failure
            _dropped_requests_out.get_sink()->on_message(create_dropped_message(request_message));
          });
        }
      }
    };

    // execute all connection related logic in the event loop thread.
    boost::asio::post(_io_context.get_executor(), handler);
  }

 private:
  // http responses are sent through this source
  event_processing::details::BaseSource _http_response_out;
  // if an incoming http request was dropped (say due to no connection being available)
  // they are forwarded through this source
  event_processing::details::BaseSource _dropped_requests_out;
  // when new connections are created and dropped, status messages are
  // emitted through this source.
  event_processing::details::BaseSource _connection_status_out; // http responses are sent through this source

  using http_connection_pool_t = std::unordered_map<HTTPConnectionKey, http_connection_t *, HTTPConnectionKeyHash>;
  http_connection_pool_t _http_connection_pool; // TODO: eviction.

  net::io_context &_io_context;

  // various object pools
  boost::object_pool<http::request<http::empty_body>> _http_request_pool; //TODO: handle pooling
  HttpResponseAllocator _response_allocator;

  response_out_message_factory_t &_response_out_message_factory;
  dropped_out_message_factory_t &_dropped_out_message_factory;

  static http::request<http::empty_body> *request_from_message(boost::object_pool<http::request<http::empty_body>> &request_pool,
                                                               const core::event_processing::IMessage &message) {
    auto *req = request_pool.construct();
    //  req->target(message.resource());
    req->target("/test");
    req->method(http::verb::get);
    req->keep_alive(true);
    req->version(11);
    req->set("host", "I am the Host");

    return req;
  }
};

}

