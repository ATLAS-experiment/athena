// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/inplace_vector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2026
 * @brief Wrapper for C++26 @c std::inplace_vector.
 *
 * C++26 adds @c std::inplace_vector, where the vector is contained
 * within the instance (with a fixed upper bound) rather than being
 * dynamically allocated.  This wraps @c std::inplace_vector as
 * @c CxxUtils::inplace_vector, falling back to @c boost::container::small_vector
 * if @c std::inplace_vector is not available.
 */


#ifndef CXXUTILS_INPLACE_VECTOR_H
#define CXXUTILS_INPLACE_VECTOR_H

#include <version>

#ifdef __cpp_lib_inplace_vector

// Have std::inplace_vector.  Put it in the CxxUtils namespace.
#include <inplace_vector>
namespace CxxUtils {
  using std::inplace_vector;
}

#else

// We don't have std::inplace_vector.
// Use boost::container::small_vector instead.
#include <boost/container/small_vector.hpp>
namespace CxxUtils {
  template <class T, size_t N>
  using inplace_vector = boost::container::small_vector<T, N>;
}

#endif


#endif // not CXXUTILS_INPLACE_VECTOR_H
