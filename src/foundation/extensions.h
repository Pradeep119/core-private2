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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-07-29.
 */

#pragma once

#include <cstddef>
#include <string>

#define interface struct

namespace adl::axp::core::extensions {

/**
 * Use this hash when you want to use Heterogeneous lookups for unordered
 * containers with key type = std::string
 */
struct string_hash {
  using hash_type = std::hash<std::string_view>;
  using is_transparent = void;

  size_t operator()(const char *str) const { return hash_type{}(str); }
  size_t operator()(std::string_view str) const { return hash_type{}(str); }
  size_t operator()(std::string const &str) const { return hash_type{}(str); }
};

/**
 * Helper class for compile time hash. Should NOT be used directly
 * @tparam N
 * @tparam I
 */
template<size_t N, size_t I = 0>
struct compile_time_hash_calc {
  static constexpr size_t apply(const char (&s)[N]) {
    return (compile_time_hash_calc<N, I + 1>::apply(s) ^ s[I]) * 16777619u;
  };
};

/**
 * Helper class for compile time hash. Should NOT be used directly
 * @tparam N
 * @tparam I
 */
template<size_t N>
struct compile_time_hash_calc<N, N> {
  static constexpr size_t apply(const char (&s)[N]) {
    return 2166136261u;
  };
};

/**\
 * Creates a compile time hash for a given string.
 * Note that this could generate collissions
 * @tparam N
 * @param s The string to calculate the hash
 * @return Hash value
 */
template<size_t N>
constexpr size_t compile_time_hash(const char (&s)[N]) {
  return compile_time_hash_calc<N>::apply(s);
}

}