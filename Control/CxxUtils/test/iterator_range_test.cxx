/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/iterator_range_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2025
 * @brief Regression tests for iterator_range.
 */


#undef NDEBUG


#include "CxxUtils/iterator_range.h"
#include <cassert>
#include <iostream>
#include <ranges>


void test1()
{
  int a[3] = {1, 2, 3};
  CxxUtils::iterator_range<const int*> r (a, a+3);
  assert (std::ranges::size(r) == 3);
  int sum = 0;
  for (int x : r) sum += x;
  assert (sum == 6);
}

int main()
{
  std::cout << "CxxUtils/iterator_range_test\n";
  test1();
  return 0;
}
