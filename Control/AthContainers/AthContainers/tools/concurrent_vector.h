// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/concurrent_vector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2024
 * @brief concurrent_vector definition.
 *
 * In a standard build, we take concurrent_vector from tbb.
 * In a standalone build, we fall back to std::vector.
 *
 * Split out from threading.h to reduce compile-time dependencies
 * on tbb.
 */


#ifndef ATHCONTAINERS_CONCURRENT_VECTOR_H
#define ATHCONTAINERS_CONCURRENT_VECTOR_H


#ifdef ATHCONTAINERS_NO_THREADS


#include <vector>


namespace AthContainers_detail {


template <class T>
using concurrent_vector = std::vector<T>;


} // namespace AthContainers_detail


#else  // not ATHCONTAINERS_NO_THREADS


// See ATLINFR-4996 for an explanation of this clunky
// warning suppression.
#if (__cplusplus > 201703L) && defined(__ROOTCLING__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-volatile"
#endif  // >C++17 with ROOT
#include "tbb/concurrent_vector.h"
#if (__cplusplus > 201703L) && defined(__ROOTCLING__)
#pragma clang diagnostic pop
#endif  // >C++17 with ROOT


namespace AthContainers_detail {


using tbb::concurrent_vector;


} // namespace AthContainers_detail



#endif


#endif // not ATHCONTAINERS_CONCURRENT_VECTOR_H
