// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/xmalloc.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2025
 * @brief Trapping version of malloc.
 */


#ifndef CXXUTILS_XMALLOC_H
#define CXXUTILS_XMALLOC_H


#include <cstdlib>


namespace CxxUtils {


/**
 * @brief Trapping version of malloc.
 * @param size Number of bytes to allocate.
 *
 * Calls malloc.  Throws a std::bad_alloc exception on failure.
 *
 * If you're writing new code, you probably don't want to use this!
 * Use make_unique/new or STL containers instead.
 * This is intended for compatiblilty with existing code requiring malloc/free.
 */
void* xmalloc (size_t size);


} // namespace CxxUtils


#endif // not CXXUTILS_XMALLOC_H
