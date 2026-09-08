// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/UIntConv.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Helpers for converting between uintptr_t and a pointer or integer.
 *
 * The concurrent hashmap classes internally represent entries as a
 * uintptr_t.  But the user-facing classes get templated on types that
 * could be any POD-like type that can fit in that size.
 * To convert from a uintptr_t to such a type T, we need to first
 * convert to a properly-sized unsigned type using static_cast,
 * and then use a union to convert to T.
 */


#ifndef CXXUTILS_UINTCONV_H
#define CXXUTILS_UINTCONV_H


#include "CxxUtils/SizedUInt.h"
#include <cstdint>
#include <bit>
#include <type_traits>


namespace CxxUtils {
namespace detail {


/**
 * @brief Helpers for converting between uintptr_t and a pointer or integer.
 */
template <class T>
struct UIntConv
{
  // Unsigned type of the same size as T.
  using uint_t = typename SizedUInt<sizeof(T)>::type;
  //
  //sensible convertible types?
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(sizeof(uint_t) == sizeof(T));
  static_assert(sizeof(T) <= sizeof(uintptr_t));
  /**
   * @brief Convert a T to a uintptr_t.
   */
  static uintptr_t valToUInt (T x) {
    const uint_t ui =  std::bit_cast<uint_t>(x);
    return static_cast<uintptr_t> (ui);
  };


  /**
   * @brief Convert a uintptr_t to a T.
   */
  static T uintToVal (uintptr_t ui) {
    const uint_t narrowed = static_cast<uint_t>(ui);
    return std::bit_cast<T>(narrowed);
  };
};


// Specialization for the case where no conversion is required.
template <>
struct UIntConv<uintptr_t>
{
  /**
   * @brief Convert a T to a uintptr_t.
   */
  static uintptr_t valToUInt (uintptr_t x) {
    return x;
  };


  /**
   * @brief Convert a uintptr_t to a T.
   */
  static uintptr_t uintToVal (uintptr_t ui) {
    return ui;
  };
};


} // namespace detail
} // namespace CxxUtils


#endif // not CXXUTILS_UINTCONV_H
