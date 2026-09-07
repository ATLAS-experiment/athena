// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/atomic_fetch_minmax.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2017
 * @brief Atomic min/max functions.
 *
 * These add atomic operations for finding the minimum or maximum.
 */


#ifndef CXXUTILS_ATOMIC_FETCH_MINMAX_H
#define CXXUTILS_ATOMIC_FETCH_MINMAX_H


#include "CxxUtils/stall.h"
#include <atomic>


namespace CxxUtils {


#if __cpp_lib_atomic_min_max


// As of C++26, these are available in stdlib.


using std::atomic_fetch_min;
using std::atomic_fetch_max;


#else


/**
 * @brief Atomically calculate maximum.
 * @param a Pointer to the atomic value.
 * @param v The other value.
 * @param memorder Memory ordering.
 *
 * Computes max(*a, v) and stores it in *a.  Returns the original value of *a.
 */
template <class T>
inline
T atomic_fetch_max (std::atomic<T>* a, T v,
                    std::memory_order memorder = std::memory_order_seq_cst)
{
  T orig = a->load (memorder);
  while (v > orig && !a->compare_exchange_strong (orig, v, memorder)) {
    CxxUtils::stall();
  }
  return orig;
}


/**
 * @brief Atomically calculate minimum.
 * @param a Pointer to the atomic value.
 * @param v The other value.
 * @param memorder Memory ordering.
 *
 * Computes min(*a, v) and stores it in *a.  Returns the original value of *a.
 */
template <class T>
inline
T atomic_fetch_min (std::atomic<T>* a, T v,
                    std::memory_order memorder = std::memory_order_seq_cst)
{
  T orig = a->load (memorder);
  while (v < orig && !a->compare_exchange_strong (orig, v, memorder)) {
    CxxUtils::stall();
  }
  return orig;
}


#endif


} // namespace CxxUtils


#endif // not CXXUTILS_ATOMIC_FETCH_MINMAX_H
