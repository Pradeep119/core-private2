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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-02.
*/

#include <iostream>

#include <benchmark/benchmark.h>
#include "../misc_stages/router_stage.h"

namespace adl::axp::core::stages::micobenchmarks {

class TwoWayRouterBenchmarkFixture : public benchmark::Fixture {
 protected:
  class DummyMessage : public event_processing::IMessage {
    virtual size_t message_type() const noexcept {
      return adl::axp::core::extensions::compile_time_hash("adl::axp::core::stages::tests::DummyMessage");
    }
  };

  class DummyMessage2 : public event_processing::IMessage {
    virtual size_t message_type() const noexcept {
      return adl::axp::core::extensions::compile_time_hash("adl::axp::core::stages::tests::DummyMessage2");
    }
  };

  class Sink1 : public event_processing::ISink {
   public:
    size_t _total = 0; // to avoid optimization
    void on_message(event_processing::IMessage *message) noexcept override {
      ++_total;
    }
  };
  class Sink2 : public event_processing::ISink {
   public:
    size_t _total = 0;
    void on_message(event_processing::IMessage *message) noexcept override {
      ++_total;
    };
  };

 public:
  TwoWayRouterBenchmarkFixture() :
      _router("router") {
  }

  void SetUp(const ::benchmark::State &state) {
    _router.get_source("source_1")->set_sink(&_sink_1);
    _router.get_source("remaining_source")->set_sink(&_sink_2);
    _input_sink = _router.get_sink("message_in");
  }

  void TearDown(const ::benchmark::State &state) {
  }

  static constexpr size_t
      message_id = adl::axp::core::extensions::compile_time_hash("adl::axp::core::stages::tests::DummyMessage");
  TwoWayRouterStage<message_id> _router;
  DummyMessage _msg_1;
  DummyMessage2 _msg_2;
  event_processing::ISink *_input_sink;
  \

  Sink1 _sink_1;
  Sink2 _sink_2;
};

BENCHMARK_F(TwoWayRouterBenchmarkFixture, Routing)(benchmark::State &st) {
  for (auto _ : st) {
    _input_sink->on_message(&_msg_1);
    _input_sink->on_message(&_msg_2);
  }
  // to avoid optimization
  std::cout << _sink_1._total << std::endl;
  std::cout << _sink_2._total << std::endl;
}

BENCHMARK_REGISTER_F(TwoWayRouterBenchmarkFixture, Routing);
}


