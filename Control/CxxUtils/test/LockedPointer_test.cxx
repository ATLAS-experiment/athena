/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/LockedPointer_test.cxx
 * @author scott snyder
 * @date Sep 2021
 * @brief Regression tests for LockedPointer.
 */


#undef NDEBUG
#include "CxxUtils/LockedPointer.h"
#include <mutex>
#include <print>
#include <cassert>


void test1a (CxxUtils::LockedPointer<int> p)
{
  assert (*p == 42);
}

void test1()
{
  std::println ("test1");

  std::recursive_mutex m;
  int x = 42;

  {
    std::unique_lock l (m);
    CxxUtils::LockedPointer<int> p (x, std::move(l));
    assert (*p == 42);
    assert (*p.get() == 42);
    test1a (std::move (p));
  }
}


int main()
{
  std::println ("CxxUtils/LockedPointer_test");
  test1();
  return 0;
}
