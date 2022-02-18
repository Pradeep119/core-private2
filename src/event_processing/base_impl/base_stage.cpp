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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-01.
 */


#include "./base_stage.h"

namespace adl::axp::core::event_processing::details {

// TODO: remove temporary std::string creation below by using fmtlib/fmt library
// concat operator for string_view is not provided b y std::string!
void BaseStage::register_source(std::string_view source_name, ISource *source) {
  const auto&[_, success] = _sources.try_emplace(std::string(source_name), source);
  if (!success)
    throw SourceExistException(
        std::string("[") + _name + "] Source with the name " + std::string(source_name) + " already exists");
}

void BaseStage::register_sink(std::string_view sink_name, ISink *sink) {
  const auto&[_, success] = _sinks.try_emplace(std::string(sink_name), sink);
  if (!success)
    throw SinkExistException(
        std::string("[") + _name + "] Sink with the name " + std::string(sink_name) + " already exists");
}

ISink *BaseStage::get_sink(std::string_view name) {
  const auto &ite = _sinks.find(name);
  if (ite != _sinks.end()) {
    return ite->second;
  } else
    throw SinkDoesNotExistException(std::string("[") + _name + "]" + "No sink found by the name " + std::string(name));
}
const ISink *BaseStage::get_sink(std::string_view name) const {
  const auto &ite = _sinks.find(name);
  if (ite != _sinks.end()) {
    return ite->second;
  } else
    throw SinkDoesNotExistException(std::string("[") + _name + "]" + "No sink found by the name " + std::string(name));
}
ISource *BaseStage::get_source(std::string_view name) {
  const auto &ite = _sources.find(name);
  if (ite != _sources.end()) {
    return ite->second;
  } else
    throw SourceDoesNotExistException(
        std::string("[") + _name + "]" + "No source found by the name " + std::string(name));
}
const ISource *BaseStage::get_source(std::string_view name) const {
  const auto &ite = _sources.find(name);
  if (ite != _sources.end()) {
    return ite->second;
  } else
    throw SourceDoesNotExistException(
        std::string("[") + _name + "]" + "No source found by the name " + std::string(name));
}

}