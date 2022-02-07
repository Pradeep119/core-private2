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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-06.
 */

#pragma once

#include "../../../foundation/extensions.h"
#include "http_connection.h"

namespace adl::axp::core::stages::details::http_dispatcher {

template<typename request_body_t,
    typename response_body_t,
    typename response_allocator_t>
class HTTPConnectionStateMachine {
 public:
  enum STATE_ID {
    not_connected,
    resolving,
    resolved,
    connecting,
    connected,
    writing,
    reading,
    writing_and_reading,
    error
  };

  using this_t = HTTPConnectionStateMachine<request_body_t, response_body_t, response_allocator_t>;
  using connection_t = Connection<request_body_t, response_body_t, response_allocator_t>;
  using response_t = http::response<response_body_t>;

  //TODO: remove
  static std::string_view to_str(int state_id) {
    switch (state_id) {
      case not_connected: return "not_connected";
      case resolving: return "resolving";
      case resolved: return "resolved";
      case connecting: return "connecting";
      case connected: return "connected";
      case writing: return "writing";
      case reading: return "reading";
      case writing_and_reading: return "writing_and_reading";
      case error: return "error";
      default: return "unknown";
    }
  }

 private:
  class BaseState {
   public:
    BaseState(connection_t *connection, this_t *machine) :
        _connection(connection),
        _machine(machine) {
    }
    virtual ~BaseState() = default;

    virtual STATE_ID resolve(STATE_ID state_id,
                             std::function<void()> callback,
                             std::function<void(beast::error_code)> error_callback) { return state_id; }
    virtual STATE_ID connect(STATE_ID state_id,
                             std::function<void()> callback,
                             std::function<void(beast::error_code)> error_callback) { return state_id; }
    virtual STATE_ID on_resolve(STATE_ID state_id) { return state_id; }
    virtual STATE_ID on_connect(STATE_ID state_id) { return state_id; }
    virtual STATE_ID on_resolve_failed(STATE_ID state_id) { return state_id; }
    virtual STATE_ID on_connect_failed(STATE_ID state_id) { return state_id; }
    virtual STATE_ID write(STATE_ID state_id,
                           const http::request<request_body_t> &request,
                           std::function<void(void)>,
                           std::function<void(beast::error_code)> error_callback) { return state_id; }
    virtual STATE_ID on_write(STATE_ID state_id) { return state_id; }
    virtual STATE_ID read(STATE_ID state_id,
                          std::function<void(http::response<response_body_t> *)> callback,
                          std::function<void(beast::error_code)> error_callback) { return state_id; }
    virtual STATE_ID on_read(STATE_ID state_id) { return state_id; }
    virtual STATE_ID on_write_error(STATE_ID state_id) { return state_id; }
    virtual STATE_ID on_read_error(STATE_ID state_id) { return state_id; }

   protected:
    connection_t *_connection;
    this_t *_machine;
  };

  class NotConnectedState : public BaseState {
    friend HTTPConnectionStateMachine;

    NotConnectedState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}

    STATE_ID resolve(STATE_ID state_id,
                     std::function<void()> callback,
                     std::function<void(beast::error_code)> error_callback) override {
      BaseState::_connection->_resolver.async_resolve(BaseState::_connection->_host.c_str(),
                                                      BaseState::_connection->_port,
                                                      [this, callback, error_callback](beast::error_code ec,
                                                                                       tcp::resolver::results_type resolve_results) {
                                                        if (ec) {
                                                          this->_machine->on_resolve_failed(std::move(ec),
                                                                                            error_callback);
                                                        } else {
                                                          this->_connection->_resolve_results = resolve_results;
                                                          this->_machine->on_resolve(callback);
                                                        }
                                                      });

      return resolving;
    }
  };

  class ResolvingState : public BaseState {
    friend HTTPConnectionStateMachine;

    ResolvingState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}

    STATE_ID on_resolve(STATE_ID state_id) override {
      return resolved;
    }
    STATE_ID on_resolve_failed(STATE_ID state_id) override {
      return error;
    }
  };
  class ResolvedState : public BaseState {
    friend HTTPConnectionStateMachine;

    ResolvedState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}

    STATE_ID connect(STATE_ID state_id,
                     std::function<void()> callback,
                     std::function<void(beast::error_code)> error_callback) override {
      BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO: get time out

      // Make the connection on the IP address we get from a lookup
      BaseState::_connection->_stream.async_connect(BaseState::_connection->_resolve_results,
                                                    [this, callback, error_callback](beast::error_code ec,
                                                                                     const tcp::resolver::results_type::endpoint_type &end_point_type) {
                                                      if (ec) {
                                                        this->_machine->on_connect_failed(std::move(ec),
                                                                                          error_callback);
                                                      } else {
                                                        this->_machine->on_connect(callback);
                                                      }
                                                    });
      return connecting;
    }
  };
  class ConnectingState : public BaseState {
    friend HTTPConnectionStateMachine;

    ConnectingState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}

    STATE_ID on_connect_failed(STATE_ID state_id) override {
      return not_connected;
    }
    STATE_ID on_connect(STATE_ID state_id) override {
      return connected;
    }
  };
  class ConnectedState : public BaseState {
    friend HTTPConnectionStateMachine;

    ConnectedState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}

    STATE_ID read(STATE_ID state_id,
                  std::function<void(http::response<response_body_t> *)> callback,
                  std::function<void(beast::error_code)> error_callback) override {
      // Set a timeout on the operation
      BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO:
      auto *res = BaseState::_connection->_response_allocator.allocate();

      // read from remote host
      http::async_read(BaseState::_connection->_stream,
                       BaseState::_connection->_buffer,
                       *res,
                       [this, res, callback, error_callback](beast::error_code ec, std::size_t) {
                         if (ec) {
                           this->_machine->on_read_error(std::move(ec), error_callback);
                         } else {
                           this->_machine->on_read(res, callback);
                         }
                       });

      return reading;
    }
    STATE_ID write(STATE_ID state_id,
                   const http::request<request_body_t> &request,
                   std::function<void(void)> callback,
                   std::function<void(beast::error_code)> error_callback) override {
      // Set a timeout on the operation
      BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO:

      // Send the HTTP request to the remote host
      //std::cout << "in ConnectedState::write " << std::this_thread::get_id() << std::endl;
      http::async_write(BaseState::_connection->_stream,
                        request,
                        [this, callback, error_callback](beast::error_code ec, std::size_t) {
                          if (ec) {
                            this->_machine->on_write_error(std::move(ec), error_callback);
                            //std::cout << "in ConnectedState::write - on write error" << std::this_thread::get_id() << std::endl;
                          } else {
                            this->_machine->on_write(callback);
                            //std::cout << "in ConnectedState::write on_write " << std::this_thread::get_id() << std::endl;
                          }
                        });

      //std::cout << "in ConnectedState::write exit " << std::this_thread::get_id() << std::endl;
      return writing;
    }
  };
  class WritingState : public BaseState {
    friend HTTPConnectionStateMachine;

    WritingState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}

    STATE_ID write(STATE_ID state_id,
                   const http::request<request_body_t> &request,
                   std::function<void(void)> callback,
                   std::function<void(beast::error_code)> error_callback) override {
      BaseState::_connection->_write_queue.emplace(request, callback, error_callback);
      return writing;
    }
    STATE_ID on_write(STATE_ID state_id) override {

      if (!BaseState::_connection->_write_queue.empty()) {
        BaseState::_connection->_write_queue.pop();
      }

      if (!BaseState::_connection->_write_queue.empty()) {
        auto write_info = BaseState::_connection->_write_queue.front();
        // Set a timeout on the operation
        BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO:

        // Send the HTTP request to the remote host
        http::async_write(BaseState::_connection->_stream,
                          write_info._request,
                          [this, write_info](beast::error_code ec, std::size_t) {
                            if (ec) {
                              this->_machine->on_write_error(std::move(ec), write_info._error_callback);
                            } else {
                              this->_machine->on_write(write_info._callback);
                            }
                          });

        return writing;
      }
      return connected;
    }
    STATE_ID on_write_error(STATE_ID state_id) override {
      if (!BaseState::_connection->_write_queue.empty()) {
        BaseState::_connection->_write_queue.pop();
      }
      return resolved;
    }
    STATE_ID read(STATE_ID state_id,
                  std::function<void(http::response<response_body_t> *)> callback,
                  std::function<void(beast::error_code)> error_callback) override {
      // Set a timeout on the operation
      BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO:
      auto *res = BaseState::_connection->_response_allocator.allocate();

      // read from remote host
      http::async_read(BaseState::_connection->_stream,
                       BaseState::_connection->_buffer,
                       *res,
                       [this, res, callback, error_callback](beast::error_code ec, std::size_t) {
                         if (ec) {
                           this->_machine->on_read_error(std::move(ec), error_callback);
                         } else {
                           this->_machine->on_read(res, callback);
                         }
                       });

      return writing_and_reading;
    }
  };
  class ReadingState : public BaseState {
    friend HTTPConnectionStateMachine;

    ReadingState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}

    STATE_ID read(STATE_ID state_id,
                  std::function<void(http::response<response_body_t> *)> callback,
                  std::function<void(beast::error_code)> error_callback) override {
      //TODO: queue this
      BaseState::_connection->_read_queue.emplace(callback, error_callback);
      return reading;
    }
    STATE_ID on_read(STATE_ID state_id) override {

      if (!BaseState::_connection->_read_queue.empty()) {
        BaseState::_connection->_read_queue.pop();
      }

      if (!BaseState::_connection->_read_queue.empty()) {

        auto read_info = BaseState::_connection->_read_queue.front();
        auto *res = BaseState::_connection->_response_allocator.allocate();

        // Set a timeout on the operation
        BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO:

        http::async_read(BaseState::_connection->_stream,
                         BaseState::_connection->_buffer,
                         *res,
                         [this, res, read_info](beast::error_code ec, std::size_t) {
                           if (ec) {
                             this->_machine->on_read_error(std::move(ec), read_info._error_callback);
                           } else {
                             this->_machine->on_read(res, read_info._callback);
                           }
                         });

        return reading;
      }

      return connected;
    }
    STATE_ID on_read_error(STATE_ID state_id) override {
      if (!BaseState::_connection->_read_queue.empty()) {
        BaseState::_connection->_read_queue.pop();
      }
      return resolved;
    }
    STATE_ID write(STATE_ID state_id,
                   const http::request<request_body_t> &request,
                   std::function<void(void)> callback,
                   std::function<void(beast::error_code)> error_callback) override {
      // Set a timeout on the operation
      BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO:

      // Send the HTTP request to the remote host
      http::async_write(BaseState::_connection->_stream,
                        request,
                        [this, callback, error_callback](beast::error_code ec, std::size_t) {
                          if (ec) {
                            this->_machine->on_write_error(std::move(ec), error_callback);
                          } else {
                            this->_machine->on_write(callback);
                          }
                        });

      return writing_and_reading;
    }
  };
  class ReadingAndWritingState : public BaseState {
    friend HTTPConnectionStateMachine;

    ReadingAndWritingState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}

    STATE_ID write(STATE_ID state_id,
                   const http::request<request_body_t> &request,
                   std::function<void(void)> callback,
                   std::function<void(beast::error_code)> error_callback) override {
      //TODO: queue the write
      BaseState::_connection->_write_queue.emplace(request, callback, error_callback);
      return writing_and_reading;
    }
    STATE_ID read(STATE_ID state_id,
                  std::function<void(http::response<response_body_t> *)> callback,
                  std::function<void(beast::error_code)> error_callback) override {
      //TODO: queue the read
      BaseState::_connection->_read_queue.emplace(callback, error_callback);
      return writing_and_reading;
    }
    STATE_ID on_write(STATE_ID state_id) override {
      if (!BaseState::_connection->_write_queue.empty()) {
        BaseState::_connection->_write_queue.pop();
      }

      if (!BaseState::_connection->_write_queue.empty()) {
        auto write_info = BaseState::_connection->_write_queue.front();
        // Set a timeout on the operation
        BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO:

        // Send the HTTP request to the remote host
        http::async_write(BaseState::_connection->_stream,
                          write_info._request,
                          [this, write_info](beast::error_code ec, std::size_t) {
                            if (ec) {
                              this->_machine->on_write_error(std::move(ec), write_info._error_callback);
                            } else {
                              this->_machine->on_write(write_info._callback);
                            }
                          });

        return writing_and_reading;
      }
      return reading;
    }
    STATE_ID on_write_error(STATE_ID state_id) override {
      if (!BaseState::_connection->_write_queue.empty()) {
        BaseState::_connection->_write_queue.pop();
      }
      return resolved;
    }
    STATE_ID on_read(STATE_ID state_id) override {
      if (!BaseState::_connection->_read_queue.empty()) {
        BaseState::_connection->_read_queue.pop();
      }

      if (!BaseState::_connection->_read_queue.empty()) {

        auto read_info = BaseState::_connection->_read_queue.front();
        auto *res = BaseState::_connection->_response_allocator.allocate();

        // Set a timeout on the operation
        BaseState::_connection->_stream.expires_after(std::chrono::seconds(30)); //TODO:

        http::async_read(BaseState::_connection->_stream,
                         BaseState::_connection->_buffer,
                         *res,
                         [this, res, read_info](beast::error_code ec, std::size_t) {
                           if (ec) {
                             this->_machine->on_read_error(std::move(ec), read_info._error_callback);
                           } else {
                             this->_machine->on_read(res, read_info._callback);
                           }
                         });

        return writing_and_reading;
      }
      return writing;
    }
    STATE_ID on_read_error(STATE_ID state_id) override {
      if (!BaseState::_connection->_read_queue.empty()) {
        BaseState::_connection->_read_queue.pop();
      }
      return resolved; // TODO: is this ok
    }
  };
  class ErrorState : public BaseState {
    friend HTTPConnectionStateMachine;

    ErrorState(connection_t *connection, this_t *machine) : BaseState(connection, machine) {}
  };

  NotConnectedState _not_connected_state;
  ResolvedState _resolved_state;
  ResolvingState _resolving_state;
  ConnectingState _connecting_state;
  ConnectedState _connected_state;
  WritingState _writing_state;
  ReadingState _reading_state;
  ReadingAndWritingState _reading_and_writing_state;
  ErrorState _error_state;

  STATE_ID _current_state;
  BaseState *_state_handlers[9]; //TODO: can this be made static

 public:
  explicit HTTPConnectionStateMachine(Connection<request_body_t, response_body_t, response_allocator_t> *connection) :
      _current_state(not_connected),
      _not_connected_state(connection, this),
      _resolved_state(connection, this),
      _resolving_state(connection, this),
      _connecting_state(connection, this),
      _connected_state(connection, this),
      _writing_state(connection, this),
      _reading_state(connection, this),
      _reading_and_writing_state(connection, this),
      _error_state(connection, this) {

    _state_handlers[not_connected] = &_not_connected_state;
    _state_handlers[resolved] = &_resolved_state;
    _state_handlers[resolving] = &_resolving_state;
    _state_handlers[connecting] = &_connecting_state;
    _state_handlers[connected] = &_connected_state;
    _state_handlers[writing] = &_writing_state;
    _state_handlers[reading] = &_reading_state;
    _state_handlers[writing_and_reading] = &_reading_and_writing_state;
    _state_handlers[error] = &_error_state;
  }

  STATE_ID current_state() const { return _current_state; }

  virtual void resolve(std::function<void()> callback, std::function<void(beast::error_code)> error_callback) {
    _current_state = _state_handlers[_current_state]->resolve(_current_state, callback, error_callback);
  }
  virtual void connect(std::function<void()> callback, std::function<void(beast::error_code)> error_callback) {
    _current_state = _state_handlers[_current_state]->connect(_current_state, callback, error_callback);
  }
  virtual void on_resolve(std::function<void()> callback) {
    _current_state = _state_handlers[_current_state]->on_resolve(_current_state);
    callback();
  }
  virtual void on_connect(std::function<void()> callback) {
    _current_state = _state_handlers[_current_state]->on_connect(_current_state);
    callback();
  }
  virtual void on_resolve_failed(beast::error_code ec, std::function<void(beast::error_code)> error_callback) {
    _current_state = _state_handlers[_current_state]->on_resolve_failed(_current_state);
    error_callback(ec);
  }
  virtual void on_connect_failed(beast::error_code ec, std::function<void(beast::error_code)> error_callback) {
    _current_state = _state_handlers[_current_state]->on_connect_failed(_current_state);
    error_callback(ec);
  }
  virtual void write(const http::request<request_body_t> &request,
                     std::function<void(void)> callback,
                     std::function<void(beast::error_code)> error_callback) {
    _current_state = _state_handlers[_current_state]->write(_current_state, request, callback, error_callback);
  }
  virtual void on_write(std::function<void(void)> callback) {
    _current_state = _state_handlers[_current_state]->on_write(_current_state);
    callback();
  }
  virtual void read(std::function<void(http::response<response_body_t> *)> callback,
                    std::function<void(beast::error_code)> error_callback) {
    _current_state = _state_handlers[_current_state]->read(_current_state, callback, error_callback);
  }
  virtual void on_read(http::response<response_body_t> *response,
                       std::function<void(http::response<response_body_t> *)> callback) {
    _current_state = _state_handlers[_current_state]->on_read(_current_state);
    callback(response);
  }
  virtual void on_write_error(beast::error_code ec, std::function<void(beast::error_code)> error_callback) {
    _current_state = _state_handlers[_current_state]->on_write_error(_current_state);
    error_callback(ec);
  }
  virtual void on_read_error(beast::error_code ec, std::function<void(beast::error_code)> error_callback) {
    _current_state = _state_handlers[_current_state]->on_read_error(_current_state);
    error_callback(ec);
  }

 private:
  Connection<request_body_t, response_body_t, response_allocator_t> *_connection;

};

}
 

 

