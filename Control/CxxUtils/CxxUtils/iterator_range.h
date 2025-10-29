// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/iterator_range.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2025
 * @brief Simple range from a pair of iterators.
 */


#ifndef CXXUTILS_ITERATOR_RANGE_H
#define CXXUTILS_ITERATOR_RANGE_H


#include <utility>


namespace CxxUtils {


/**
 * @brief Simple range from a pair of iterators.
 *
 * This adapts a pair of iterators to work as a range.
 *
 * @c std::ranges::subrange can't always be used, because it requires that
 * the sentinel type be assignable and default-constructible, which is not
 * the case for the concurrent map iterators.  In that case, this class
 * can be used instead.  (@c boost::iterator_range could also work,
 * but it brings in the boost type traits implementation as header
 * dependencies.)
 */
template <class ITER>
class iterator_range : public std::pair<ITER, ITER>
{
public:
  using std::pair<ITER, ITER>::pair;
  ITER begin() const { return this->first; }
  ITER end() const { return this->second; }
};


} // namespace CxxUtils


#endif // not CXXUTILS_ITERATOR_RANGE_H
