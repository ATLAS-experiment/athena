// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/minmax_transformed_element.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2025
 * @brief Return the minimum or maximum element from a transformed range.
 *
 * These are like std::ranges::min_element and std::ranges::max_element,
 * except that we apply a transform to the input range first.
 * An example would be to take a range of vectors and find the one
 * closest to some other vector.
 * Why not just use std::ranges::min_element along with std::ranges::transform?
 * First, it's a bit awkward without the enumerate/zip views from C++23.
 * But more importantly, it's hard to avoid having the transform
 * evaluated multiple times per element.  This way, the transform is
 * evaluated exactly once per element.
 */


#ifndef CXXUTILS_MINMAX_TRANSFORMED_ELEMENT_H
#define CXXUTILS_MINMAX_TRANSFORMED_ELEMENT_H


#include <ranges>
#include <utility>
#include <cstdlib>


namespace CxxUtils {


/**
 * @brief Find the minimum transformed element in a range.
 * @param r Input range.
 * @param f Transform function.
 *
 * Evaluates f(x) for each x in r and finds the minimum.
 * Returns a pair (fmin, itmin), where fmin is this minimum value,
 * and itmin is an iterator pointing at the corresponding element in r.
 * If more than one element has the same minimum value, the iterator
 * of the first will be returned.
 *
 * Precondition: Range r is not empty.
 */
template <class RANGE, class FUNC>
inline
auto min_transformed_element (RANGE&& r, FUNC&& f)
{
  if (std::ranges::empty (r)) std::abort();
  auto it = std::ranges::begin(r);
  auto itmin = it;
  auto val = f(*it);
  for (++it; it != std::ranges::end(r); ++it) {
    auto val2 = f(*it);
    if (val2 < val) {
      val = val2;
      itmin = it;
    }
  }
  return std::make_pair (val, itmin);
}


/**
 * @brief Find the maximum transformed element in a range.
 * @param r Input range.
 * @param f Transform function.
 *
 * Evaluates f(x) for each x in r and finds the maximum.
 * Returns a pair (fmax, itmax), where fmax is this maximum value,
 * and itmax is an iterator pointing at the corresponding element in r.
 * If more than one element has the same maximum value, the iterator
 * of the first will be returned.
 *
 * Precondition: Range r is not empty.
 */
template <class RANGE, class FUNC>
inline
auto max_transformed_element (RANGE&& r, FUNC&& f)
{
  if (std::ranges::empty (r)) std::abort();
  auto it = std::ranges::begin(r);
  auto itmax = it;
  auto val = f(*it);
  for (++it; it != std::ranges::end(r); ++it) {
    auto val2 = f(*it);
    if (val2 > val) {
      val = val2;
      itmax = it;
    }
  }
  return std::make_pair (val, itmax);
}


} // namespace CxxUtils


#endif // not CXXUTILS_MINMAX_TRANSFORMED_ELEMENT_H
