// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/ranges.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief C++20 range helpers.
 *
 * Provides the InputRangeOverT concept.
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


} // namespace CxxUtils


#endif // not CXXUTILS_RANGES_H
