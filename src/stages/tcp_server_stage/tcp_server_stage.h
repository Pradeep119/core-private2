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

//TODO: remove void* data
class TcpRequestMessage : public event_processing::IMessage {
 public:
  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::stages::TcpRequestMessage");
  }

  void set_user_data(void *user_data) {
    _user_data = user_data;
  }
  const void *get_user_data() const noexcept {
    return _user_data;
  }
  void *get_user_data() noexcept {
    return _user_data;
  }

 private:
  void *_user_data;
};

class TcpResponseMessage : public event_processing::IMessage {
 public:
  TcpResponseMessage(TcpRequestMessage *request, std::string_view str_response) :
      _request(request),
      _str_response(str_response) {
  }

  size_t message_type() const noexcept override {
    return extensions::compile_time_hash("adl::axp::core::stages::TcpResponseMessage");
  }

  TcpRequestMessage *get_request() noexcept {
    return _request;
  }
  const TcpRequestMessage *get_request() const noexcept {
    return _request;
  }
  std::string_view get_str_response() const noexcept {
    return _str_response;
  }

 private:
  TcpRequestMessage *_request;
  const std::string _str_response;
};

template<typename message_assembler_t>
class TcpServerStage : public event_processing::details::BaseStage,
                       public event_processing::ISink {

 public:
  TcpServerStage(std::string_view name, boost::asio::io_context &io_context, const int port) :
      event_processing::details::BaseStage(name),
      _io_context(io_context),
      _port(port) {
    register_source("tcp_request_out", &_tcp_request_out);
    register_sink("tcp_response_in", this);
    _logger = spdlog::get(std::string(name));
  }

  void start() override {
    try {
      _acceptor = std::make_unique<ip::tcp::acceptor>(_io_context, ip::tcp::endpoint(ip::tcp::v4(), _port));
    } catch (const std::runtime_error &ex) {
      _logger->error(ex.what());
    }
    async_accept_next();
  }

  void on_message(core::event_processing::IMessage *message) noexcept override {
    const auto &logger = this->_logger;
    auto handler = [message, &logger]() {
      auto *response = static_cast<TcpResponseMessage *>(message);
      //TODO: after a disconnection, this object may become invalid
      auto *socket = reinterpret_cast<asio::ip::tcp::socket *>(response->get_request()->get_user_data());
      asio::async_write(*socket,
                        asio::buffer(response->get_str_response()),
                        [&logger](const boost::system::error_code &error, std::size_t bytes_transferred) {
                          if (error) {
                            //TODO: handle error
                            logger->error("error sending the TCP response");
                          }
                        });
    };

    boost::asio::post(_io_context.get_executor(), handler);
  }

 private:

  struct TCPConnection {
    explicit TCPConnection(asio::io_context &io_context) :
        _socket(io_context),
        _message_assembler() {
    }

    ip::tcp::socket _socket;
    message_assembler_t _message_assembler;
    std::array<char, 8192> read_buffer; //TODO: Size - get from config
  };

  void async_read(TCPConnection *tcp_connection) {
    // start a new async read
    const auto &buff = asio::buffer(tcp_connection->read_buffer, 8192);
    tcp_connection->_socket.template async_read_some(buff,
                                                     [this, buff, tcp_connection](const boost::system::error_code &error,
                                                                                  std::size_t bytes_transferred) {
                                                       if (error) {
                                                         this->_logger->error("tcp read error {}", error.message());
                                                       }
                                                       // call assembler to get messages
                                                       std::list<TcpRequestMessage *> messages =
                                                           tcp_connection->_message_assembler.assemble(static_cast<char *>(buff.data()),
                                                                                                       bytes_transferred,
                                                                                                       tcp_connection);

                                                       for (auto message : messages) {
                                                         message->set_user_data(&tcp_connection->_socket);
                                                         _tcp_request_out.get_sink()->on_message(message);
                                                       }
                                                       async_read(tcp_connection);
                                                     });
  }

  void async_accept_next() {
    //TODO: use pools
    auto *tcp_connection = _tcp_connections.emplace_back(new TCPConnection(_io_context)).get();
    _acceptor->async_accept(tcp_connection->_socket, [this, tcp_connection](const boost::system::error_code &error) {

      if (error) {
        //TODO:
        _logger->error("error {}", error.message());
      } else {
        // start a new async read
        _logger->info("new connection");
        async_read(tcp_connection);
        async_accept_next();
      }
    });
  }

  // sources
  event_processing::details::BaseSource _tcp_request_out;

  asio::io_context &_io_context;
  const int _port;
  std::unique_ptr<ip::tcp::acceptor> _acceptor;
  std::list<std::unique_ptr<TCPConnection>> _tcp_connections;
  std::shared_ptr<spdlog::logger> _logger;
};
}