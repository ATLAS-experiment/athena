/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/vec_fb_float_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2020
 * @brief Unit test for vec_fb for float types.
 */


#define WANT_VECTOR_FALLBACK 1
#include "vec_test_common.h"
#include "CxxUtils/vec.h"
#include <print>

void
test1()
{
  std::println ("test1 vec_fb for float");
  testFloat1<CxxUtils::vec_fb>();
}

int
main()
{
  std::println ("CxxUtils/vec_test");
  test1();
  return 0;
}
