// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/ranges.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief C++20 range helpers.
 *
 * Provides a C++23-like to() function for converting a range to a container.
 */


#ifndef CXXUTILS_RANGES_H
#define CXXUTILS_RANGES_H


#include <ranges>


namespace CxxUtils {


/// Concept for an input range over a given type.
template <class RANGE, class T>
concept InputRangeOverT =
  std::ranges::input_range<RANGE> &&
  std::convertible_to<std::ranges::range_value_t<RANGE>, T>;


#if __cpp_lib_ranges_to_container
// If we're using C++23, just take the library version.
using std::ranges::to;
#else

// Simplified version of C++23 to().
template <class CONT, class RANGE>
CONT to (RANGE&& r)
{
  return CONT (std::ranges::begin (r), std::ranges::end (r));
}

#endif

} // namespace CxxUtils


#endif // not CXXUTILS_RANGES_H
