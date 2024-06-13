// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/range_with_at_conv.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Test range_with_cont.
 */


#undef NDEBUG
#include "CxxUtils/range_with_conv.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <list>
#include <span>
#include <memory_resource>


void test1()
{
  std::cout << "test1\n";
  std::vector<int> v { 0, 1, 2, 3, 4 };
  using Rtest = CxxUtils::range_with_conv<std::span<int> >;
  Rtest r (v.begin(), v.size());
  std::vector<int> v2 = r;
  assert (v2 == v);
  assert (r.asVector() == v);
  std::list<int> l2 = r;
  assert (l2 == (std::list<int> { 0, 1, 2, 3, 4 }) );
  std::vector<int, std::pmr::polymorphic_allocator<int> > v3 = r;
  assert (v3.size() == v.size());
  for (size_t i = 0; i < v.size(); i++)
    assert (v3[i] == v[i]);
}


int main()
{
  std::cout << "CxxUtils/range_with_conv_test\n";
  test1();
  return 0;
}
