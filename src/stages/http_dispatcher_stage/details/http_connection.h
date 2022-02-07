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

#include <queue>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <utility>
#include <utility>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "../../../foundation/extensions.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace adl::axp::core::stages::details::http_dispatcher {

template<typename request_body_t,
    typename response_body_t,
    typename response_allocator_t>
struct Connection {
  Connection(net::io_context &io_context,
             std::string_view host,
             std::string_view port,
             response_allocator_t &response_allocator) :
      _host(host),
      _port(port),
      _resolver(io_context.get_executor()),
      _stream(io_context.get_executor()),
      _response_allocator(response_allocator) {
  }

  struct WriteInfo {
    WriteInfo(const http::request<request_body_t> &request,
              const std::function<void(void)> &callback,
              const std::function<void(beast::error_code)> &error_callback) :
        _request(request),
        _callback(callback),
        _error_callback(error_callback) {
    }

    http::request<request_body_t> _request;
    std::function<void(void)> _callback;
    std::function<void(beast::error_code)> _error_callback;
  };

  struct ReadInfo {
    ReadInfo(const std::function<void(http::response<response_body_t> *)> &callback,
             const std::function<void(beast::error_code)> &error_callback) :
        _callback(callback),
        _error_callback(error_callback) {
    }

    std::function<void(http::response<response_body_t> *)> _callback;
    std::function<void(beast::error_code)> _error_callback;
  };

  const std::string _host;
  const std::string _port;
  tcp::resolver _resolver;
  tcp::resolver::results_type _resolve_results;
  beast::tcp_stream _stream;
  beast::flat_buffer _buffer; // (Must persist between reads)
  std::queue<WriteInfo> _write_queue;
  std::queue<ReadInfo> _read_queue;
  response_allocator_t &_response_allocator;
};

}