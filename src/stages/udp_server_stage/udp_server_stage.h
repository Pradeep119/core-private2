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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-10-14.
*/

#pragma once

#include <list>
#include <iostream>

#include <boost/pool/object_pool.hpp>
#include <boost/asio.hpp>

#include "../../event_processing/messages/http_request_message.h"
#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"
#include "../../event_processing/runnable.h"

namespace adl::axp::core::stages {

namespace asio = boost::asio;
namespace ip = boost::asio::ip;

template<typename message_assembler_t>
class UdpServerStage : public event_processing::details::BaseStage {

 public:
  UdpServerStage(std::string_view name,
                 boost::asio::io_context &io_context,
                 std::string_view host,
                 const int port,
                 std::string_view multicast_address) :
      event_processing::details::BaseStage(name),
      _io_context(io_context),
      _host(host),
      _port(port),
      _multicast_address(multicast_address),
      _socket(io_context) {

    register_source("udp_data_out", &_udp_data_out);
  }

  void start() override {
    try {

      // Create the socket so that multiple may be bound to the same address.
      boost::asio::ip::udp::endpoint listen_endpoint(boost::asio::ip::address::from_string(_host), _port);
      _socket.open(listen_endpoint.protocol());
      _socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
      _socket.bind(listen_endpoint);
      // Join the multicast group.
      _socket.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address::from_string(_multicast_address)));

      async_receive();

    } catch (const std::runtime_error &ex) {
      std::cout << ex.what() << std::endl; //TODO: log
    }
  }

  void async_receive() {
    _socket.async_receive_from(
        boost::asio::buffer(_buffer, _buffer_size), _sender_endpoint, [this](const boost::system::error_code &error,
                                                                             size_t bytes_received) {
          std::cout << "received data" << std::endl;
          async_receive();
        });
  }

  // sources
  event_processing::details::BaseSource _udp_data_out;

  asio::io_context &_io_context;
  const std::string _host;
  const int _port;
  const std::string _multicast_address;

  boost::asio::ip::udp::socket _socket;
  boost::asio::ip::udp::endpoint _sender_endpoint;

  constexpr static size_t _buffer_size = 1024;
  std::array<char, _buffer_size> _buffer;
};

}
