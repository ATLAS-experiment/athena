// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/flat_set.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2026
 * @brief Wrapper for C++23 @c std::flat_set.
 *
 * C++23 adds @c std::flat_set, representing a set as a sorted sequence.
 * This wraps @c std::flat_set as @c CxxUtils::flat_set, falling back to
 * @c boost::container::flat_set if @c std::flat_set is not available.
 */


#ifndef CXXUTILS_FLAT_SET_H
#define CXXUTILS_FLAT_SET_H

#include <version>

#ifdef __cpp_lib_flat_set

// Have std::flat_set.  Put it in the CxxUtils namespace.
#include <flat_set>
namespace CxxUtils {
  using std::flat_set;
}

#else

// We don't have std::flat_set.
// Use boost::container::flat_set instead.
#include <functional>
#include <boost/container/flat_set.hpp>
namespace CxxUtils {
  // void for the third argument matches the defaults in
  // boost/container/container_fwd.hpp
  template <class KEY, class COMPARE = std::less<KEY>, class KEYCONTAINER = void>
  using flat_set = boost::container::flat_set<KEY, COMPARE, KEYCONTAINER>;
}

#endif


#endif // not CXXUTILS_FLAT_SET_H
