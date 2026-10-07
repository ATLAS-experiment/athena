/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/vec_float_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2020
 * @brief Unit test for vec for float types.
 */


#include "vec_test_common.h"
#include "CxxUtils/vec.h"
#include <print>
void test1()
{
  using CxxUtils::vec;
  std::println ("test1 vec for float");
  testFloat1<CxxUtils::vec>();
}


int main()
{
  std::println ("CxxUtils/vec_test");
  test1();
  return 0;
}
