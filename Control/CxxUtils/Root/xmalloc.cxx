/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/Root/xmalloc.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2025
 * @brief Trapping version of malloc.
 */


#include "CxxUtils/xmalloc.h"
#include <malloc.h>
#include <new>



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
void* xmalloc (size_t size)
{
  void* p = malloc (size);
  if (!p) throw std::bad_alloc();
  return p;
}



} // namespace CxxUtils


