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

#include <thread>
#include <atomic>

#include <folly/ProducerConsumerQueue.h>

#include "../../event_processing/base_impl/base_stage.h"
#include "../../event_processing/sink.h"
#include "../../event_processing/base_impl/base_source.h"
#include "../../event_processing/message.h"
#include "../../event_processing/runnable.h"

namespace adl::axp::core::stages {

class MPMCQueue : public event_processing::details::BaseStage,
                  public event_processing::ISink,
                  public event_processing::IRunnable {
 public:
  MPMCQueue(std::string_view name, uint32_t capacity);
  MPMCQueue(const MPMCQueue &) = delete;
  MPMCQueue &operator=(const MPMCQueue &rhs) = delete;
  ~MPMCQueue() override = default;

  void on_message(event_processing::IMessage *message) noexcept override;

 private:
  int run(void *data) noexcept override;
  int stop() noexcept override;

  std::thread *_worker = nullptr;
  std::atomic_bool _stop = false;
  folly::ProducerConsumerQueue<event_processing::IMessage *>
      _message_queue; //TODO: use folly/MPMCQueue and waiting enqueue
  event_processing::details::BaseSource _message_out; // messages taken from queue are dispatched through this source
  event_processing::details::BaseSource
      _buffer_full_out; // new messages are dispatched through this source when queue is full through the calling thread

};

}