/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthAllocators/test/exceptions_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2016
 * @brief Regression tests for exceptions.
 */


#undef NDEBUG
#include "AthAllocators/exceptions.h"
#include <print>


void test1()
{
  std::println ("test1");

  std::println ("{}", SG::ExcDifferentArenas().what());
  std::println ("{}", SG::ExcProtection(EINVAL).what());
  std::println ("{}", SG::ExcProtected().what());
}


int main()
{
  test1();
  return 0;
}
