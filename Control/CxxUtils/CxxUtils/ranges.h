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


#ifdef __CLING__


// Allow begin/end functions to be findable by ADL.
// Needed to work around problems seen with cling (as of 6.32.00 at least)
// and gcc14
namespace std { namespace ranges {
template <class RANGE, class XFORM>
auto begin (transform_view<RANGE, XFORM>& s) { return s.begin(); }
template <class RANGE, class XFORM>
auto begin (const transform_view<RANGE, XFORM>& s) { return s.begin(); }
template <class RANGE, class XFORM>
auto end (transform_view<RANGE, XFORM>& s) { return s.end(); }
template <class RANGE, class XFORM>
auto end (const transform_view<RANGE, XFORM>& s) { return s.end(); }
}} // namespace std::ranges


#endif


#endif // not CXXUTILS_RANGES_H
