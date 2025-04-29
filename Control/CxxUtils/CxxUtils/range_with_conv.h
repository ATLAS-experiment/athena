// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/range_with_conv.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Add to a range class conversions to containers.
 */


#ifndef CXXUTILS_RANGE_WITH_CONV_H
#define CXXUTILS_RANGE_WITH_CONV_H


#include <ranges>
#include <vector>


namespace CxxUtils {


/**
 * @brief Add to a range class conversions to containers.
 */
template <class RANGE>
class range_with_conv
  : public RANGE
{
public:
  using RANGE::RANGE;
  using Elt_t = std::ranges::range_value_t<RANGE>;


  /**
   * @brief Implicit conversion of the range to a container.
   */
  template <template <class, class> class CONT, class ALLOC>
  operator CONT<Elt_t, ALLOC>() const
  {
    return CONT<Elt_t, ALLOC> (this->begin(), this->end());
  }


  /**
   * @brief Explicit conversion to a vector.  For use from Python.
   */
  std::vector<Elt_t> asVector() const
  {
    return *this;
  }
};


// Stand-alone begin/end functions, findable by ADL.
// Needed to work around issues seen when root 6.30.04 is used with gcc14.
template <class T>
auto begin (range_with_conv<T>& s) { return s.begin(); }
template <class T>
auto begin (const range_with_conv<T>& s) { return s.begin(); }
template <class T>
auto end (range_with_conv<T>& s) { return s.end(); }
template <class T>
auto end (const range_with_conv<T>& s) { return s.end(); }


} // namespace CxxUtils


#endif // not CXXUTILS_RANGE_WITH_CONV_H
