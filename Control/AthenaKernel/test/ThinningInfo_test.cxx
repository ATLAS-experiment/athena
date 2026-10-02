/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/test/ThinningInfo_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2020
 * @brief Regression tests for ThinningInfo.
 */

#undef NDEBUG

#include "AthenaKernel/ThinningInfo.h"
#include <cassert>
#include <print>
#include <stdexcept>


void test1()
{
  std::println ("test1");

  SG::ThinningInfo ti;
  ti.m_vetoed.insert (2);
  assert (ti.vetoed(2));
  assert (!ti.vetoed(1));

  ti.m_compression[10].insert(5);
  assert (ti.compression(5) == 10);
  assert (ti.compression(6) == 0);
}


int main()
{
  std::println ("AthenaKernel/ThinningInfo_test");
  test1();
  return 0;
}
