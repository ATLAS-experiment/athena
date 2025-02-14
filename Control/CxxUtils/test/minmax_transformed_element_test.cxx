/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/minmax_transformed_element_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2025
 * @brief Unit tests for min/max_transformed_element.
 */

#undef NDEBUG
#include "CxxUtils/minmax_transformed_element.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <cassert>


struct distfrom
{
  distfrom (double x) : m_x(x) {}
  double operator() (double y) const { return std::abs(y - m_x); }
  double m_x;
};

void test1()
{
  std::cout << "test1\n";
  std::vector<double> v { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  {
    auto [fmin, itmin] = CxxUtils::min_transformed_element (v, distfrom(4.25));
    assert (fmin == 0.25);
    assert (*itmin == 4);
  }

  {
    auto [fmax, itmax] = CxxUtils::max_transformed_element (v, distfrom(2.5));
    assert (fmax == 6.5);
    assert (*itmax == 9);
  }
}


int main()
{
  std::cout << "minmax_transformed_element_test\n";
  test1();
  return 0;
}
