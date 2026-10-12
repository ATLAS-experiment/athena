/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/PackedLink_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Regression tests for PackedLink.
 */


#undef NDEBUG

#include "AthContainers/PackedLinkImpl.h"
#include <print>
#include <cassert>


void test1()
{
  std::println ("test1");
  SG::PackedLinkBase pl1;
  assert (pl1.index() == 0);
  assert (pl1.collection() == 0);

  SG::PackedLinkBase pl2 (10, 1000);
  assert (pl2.index() == 1000);
  assert (pl2.collection() == 10);

  assert (pl1 == pl1);
  assert (pl2 == pl2);
  assert (pl1 != pl2);

  pl1.setIndex (1000);
  assert (pl1.index() == 1000);
  assert (pl1.collection() == 0);

  pl1.setCollection (10);
  assert (pl1.index() == 1000);
  assert (pl1.collection() == 10);
  assert (pl1 == pl2);
}


int main()
{
  std::println ("AthContainers/PackedLink_test");
  test1();
  return 0;
}


