//Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
/**
 * @file CxxUtils/test/close_to_zero_test.cxx
 * @author sroe
 * @date Oct 2025
 * @brief tests for close_to_zero function
 */
 
#undef NDEBUG

#include "CxxUtils/close_to_zero.h"
#include <cassert>
#include <iostream>
#include <limits>  //for epsilon

using CxxUtils::close_to_zero;

int main(){
  constexpr double dbleps =  std::numeric_limits<double>::epsilon();
  constexpr float feps =  std::numeric_limits<float>::epsilon();
  //test for double
  assert(close_to_zero(0.));
  //user defined tolerance of 0.5
  assert(close_to_zero(0.25, 0.5));
  assert(not close_to_zero(2. * dbleps));
  assert(not close_to_zero(1., 0.5));
  //test for float
  assert(close_to_zero(0.f));
  //user defined tolerance of 0.3
  assert(close_to_zero(0.1f, 0.3f));
  assert(not close_to_zero(2.f * feps));
  assert(not close_to_zero(1.f, 0.5f));
  //test for int
  assert(close_to_zero(0));
  assert(not close_to_zero(1));
  //won't compile
  //std::cout<<close_to_zero("hello");
  return 0;
}