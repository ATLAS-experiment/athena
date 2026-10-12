/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/IsMostDerivedFlag_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2013
 * @brief Regression tests for IsMostDerivedFlag.
 */


#undef NDEBUG
#include "AthContainers/tools/IsMostDerivedFlag.h"
#include <print>
#include <cassert>


void test1()
{
  std::println ("test1");
  SG::IsMostDerivedFlag f1;
  assert (f1 == false);
  f1 = true;
  assert (f1 == true);
  f1 = false;
  assert (f1 == false);
  SG::IsMostDerivedFlag f2 (true);
  assert (f2 == true);
  SG::IsMostDerivedFlag f3 (f2);
  assert (f3 == false);
  SG::IsMostDerivedFlag f4 (std::move (f2));
  assert (f4 == false);
}


int main()
{
  test1();
  return 0;
}
