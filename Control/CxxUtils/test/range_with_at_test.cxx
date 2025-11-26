// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/range_with_at_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Test range_with_at.
 */


#undef NDEBUG
#include "CxxUtils/range_with_at.h"
#include "TestTools/expect_exception.h"
#include <iostream>
#include <cassert>
#include <ranges>
#include <vector>
#include <span>
#include <stdexcept>


struct Square
{
  int operator() (int x) const { return x*x; }
};


void test1()
{
  std::cout << "test1\n";

  std::vector<int> v { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  using Xform = std::ranges::transform_view<std::span<int>, Square>;
  using Range = CxxUtils::range_with_at<Xform>;
  Range r (std::span (v.begin(), v.end()), Square());
  
  assert (r[4] == 16);
  assert (r.at(4) == 16);
  EXPECT_EXCEPTION (std::out_of_range, r.at(20));

  const Range& cr = r;
  assert (cr.at(4) == 16);
  EXPECT_EXCEPTION (std::out_of_range, cr.at(20));
}

//coverity[root_function]
int main()
{
  std::cout << "CxxUtils/range_with_at_test\n";
  test1();
  return 0;
}
