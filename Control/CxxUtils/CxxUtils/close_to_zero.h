/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/close_to_zero.h
 * @author shaun roe
 * @date Oct 2025
 * @brief test if a value is close enough to zero to be an unreliable denominator.
 */
 
#ifndef CXXUTILS_CLOSE_TO_ZERO_H
#define CXXUTILS_CLOSE_TO_ZERO_H

#include <limits>  //for epsilon
#include <cmath>   //for abs
#include <type_traits> //for std::is_integral_v etc

 
namespace CxxUtils {
/**
 * @brief Checks if a value is close to zero.
 * 
 * This function determines whether a given value is "close to zero" based
 * on its type. For integral types, only exact zero is considered. For
 * floating-point types, the value is considered close to zero if its
 * absolute value is less than a specified epsilon.
 * 
 * @tparam T The type of the value. Must be an arithmetic type.
 * @param value The value to check.
 * @param eps The tolerance for floating-point comparisons. Defaults to
 *        `std::numeric_limits<T>::epsilon()`. Ignored for integral types.
 * 
 * @return true if the value is zero (for integers) or within epsilon of
 *         zero (for floating-point types), false otherwise.
 * 
 * @note This function only supports arithmetic types (integers and floating-point).
 *       Attempting to use it with non-arithmetic types will trigger a compile-time
 *       static assertion.
 * 
 * @example
 * int i = 0;
 * bool result_int = close_to_zero(i); // returns true
 * 
 * double d = 1e-12;
 * bool result_double = close_to_zero(d, 1e-10); // returns true
 */
  template <typename T>
  bool close_to_zero(T value, T eps = std::numeric_limits<T>::epsilon()) {
    if constexpr (std::is_integral_v<T>) {
      // For integers, only exact zero is invalid
      return value == 0;
    } else if constexpr (std::is_floating_point_v<T>) {
      return std::abs(value) < eps;
    } else {
      static_assert(std::is_arithmetic_v<T>, "close_to_zero: Only arithmetic types supported");
      return false;
    }
  }
}

#endif

