// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/range_with_at.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Add at() methods to a range class.
 */


#ifndef CXXUTILS_RANGE_WITH_AT_H
#define CXXUTILS_RANGE_WITH_AT_H


#include "CxxUtils/throw_out_of_range.h"
#include <ranges>
#include <stdexcept>


namespace CxxUtils {


/**
 * @brief Add at() methods to a range class.
 *
 * Many standard range objects allow operator[] but do not implement at().
 * Use this to add at() methods to a random_access_range class.
 */
template <class RANGE>
class range_with_at
  : public RANGE
{
public:
  using RANGE::RANGE;


  decltype(auto) at (size_t i)
  {
    if (i >= std::size(*this))
      throw_out_of_range (__PRETTY_FUNCTION__, i, std::size(*this), this);
    return this->operator[] (i);
  }


  decltype(auto) at (size_t i) const
  {
    if (i >= std::size(*this))
      throw_out_of_range (__PRETTY_FUNCTION__, i, std::size(*this), this);
    return this->operator[] (i);
  }
};


/// Helper to add at() methods to a transform_view.
template <std::ranges::input_range SPAN, class XFORM>
using transform_view_with_at =
  range_with_at<std::ranges::transform_view<SPAN, XFORM> >;


// Stand-alone begin/end functions, findable by ADL.
// Needed to work around issues seen when root 6.30.04 is used with gcc14.
template <class T>
auto begin (range_with_at<T>& s) { return s.begin(); }
template <class T>
auto begin (const range_with_at<T>& s) { return s.begin(); }
template <class T>
auto end (range_with_at<T>& s) { return s.end(); }
template <class T>
auto end (const range_with_at<T>& s) { return s.end(); }


} // namespace CxxUtils


#endif // not CXXUTILS_RANGE_WITH_AT_H
