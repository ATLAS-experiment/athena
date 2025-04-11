/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file xmalloc_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2025
 * @brief Regression tests for xmalloc.
 */


#undef NDEBUG
#include "CxxUtils/xmalloc.h"
#include "TestTools/expect_exception.h"
#include <new>
#include <iostream>


void test1()
{
  void* p = CxxUtils::xmalloc (1000);
  free (p);
  EXPECT_EXCEPTION( std::bad_alloc, p = CxxUtils::xmalloc (static_cast<size_t> (-1)) );
}


int main()
{
  std::cout << "CxxUtils/xmalloc_test\n";
  test1();
  return 0;
}
