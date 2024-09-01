/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/JaggedVecElt_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2024
 * @brief Regression tests for JaggedVecElt.
 */


#undef NDEBUG

#include "AthContainers/JaggedVecImpl.h"
#include <iostream>
#include <cassert>


void test1()
{
  std::cout << "test1\n";

  SG::JaggedVecElt<int> e1;
  assert (e1.end() == 0);

  SG::JaggedVecElt<int> e2[2] (5, 7);
  assert (e2[0].begin(0) == 0);
  assert (e2[0].end() == 5);
  assert (e2[0].size(0) == 5);
  assert (e2[1].begin(1) == 5);
  assert (e2[1].end() == 7);
  assert (e2[1].size(1) == 2);

  SG::JaggedVecElt<int> e3 (5);

  assert (e2[0] == e3);
  assert (e2[0] != e1);

  SG::JaggedVecEltBase::Shift sh (3);
  sh (e3);
  assert (e3.end() == 8);
}


int main()
{
  std::cout << "AthContainers/JaggedVecElt_test\n";
  test1();
  return 0;
}


