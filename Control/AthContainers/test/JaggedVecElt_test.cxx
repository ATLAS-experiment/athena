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
  assert (e1.begin() == 0);
  assert (e1.end() == 0);
  assert (e1.size() == 0);

  SG::JaggedVecElt<int> e2 (3, 5);
  assert (e2.begin() == 3);
  assert (e2.end() == 5);
  assert (e2.size() == 2);

  SG::JaggedVecElt<int> e3 (3, 5);

  assert (e2 == e3);
  assert (e2 != e1);

  SG::JaggedVecEltBase::Shift sh (3);
  sh (e2);
  assert (e2.begin() == 6);
  assert (e2.end() == 8);
  assert (e2.size() == 2);
}


int main()
{
  std::cout << "AthContainers/JaggedVecElt_test\n";
  test1();
  return 0;
}


