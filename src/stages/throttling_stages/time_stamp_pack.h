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
#include <array>
#include <zlib.h>

namespace adl::axp::core::stages {

template<size_t time_stamp_count>
struct TimeStampPack {
  uLong _crc;
  std::array<size_t, time_stamp_count> _time_stamps;
};

template<size_t batch_capacity>
struct TimeStampPackBatch {
  std::array<TimeStampPackBatch, batch_capacity> _packs;
  size_t _packet_count = {0};
};

}