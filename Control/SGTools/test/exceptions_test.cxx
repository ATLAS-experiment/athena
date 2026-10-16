/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file SGTools/test/exceptions_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2014
 * @brief Regression tests for exceptions.
 */

#undef NDEBUG
#include "SGTools/exceptions.h"
#include <print>


void test1()
{
  std::println ("test1");

  std::println ("{}", SG::ExcBadDataProxyCast (123, typeid(int)).what());
  std::println ("{}", SG::ExcProxyCollision (123, "abc", 456, "def").what());
  std::println ("{}", SG::ExcSgkeyCollision ("asd", 123, "def", 456, 789).what());
}


int main()
{
  test1();
  return 0;
}
