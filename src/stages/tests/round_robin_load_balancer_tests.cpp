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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-17.
 */

#include <gtest/gtest.h>
#include "../../services/store_services/key_value_store.h"
#include "../../stages/load_balancer_stages/round_robin_load_balancer_stage.h"

// store is lightweight, so we dont mock it here. Instead we use the real one.

namespace adl::axp::core::stages::load_balancers::test {

namespace kv = services::store_services;

TEST(round_robin_load_balancer, constuction) {
  kv::KeyValueStore store;
//  RoundRobinLoadBalancerStage stage("stage test", store, "channel", "end_points");
}

}