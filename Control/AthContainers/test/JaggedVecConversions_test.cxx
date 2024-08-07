/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/JaggedVecConversions_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2024
 * @brief Regression tests for JaggedVec converter classes.
 */


#undef NDEBUG
#include "AthContainers/tools/JaggedVecConversions.h"
#include "TestTools/expect_exception.h"
#include <vector>
#include <iostream>
#include <cassert>


void test_JaggedVecConstConverter()
{
  std::cout << "test_JaggedVecConstConverter\n";

  std::vector<int> v1 { 1, 2, 3, 4, 5 };
  SG::AuxDataSpanBase sp1 { v1.data(), v1.size() };
  SG::detail::JaggedVecConstConverter<int> c1 (sp1);
  SG::JaggedVecEltBase e1 (2, 4);
  auto r1 = c1 (e1);
  assert (r1.size() == 2);
  assert (r1[1] == 4);

  SG::JaggedVecEltBase e2 (2, 10);
  EXPECT_EXCEPTION( std::out_of_range, c1 (e2) );
  SG::JaggedVecEltBase e3 (4, 2);
  EXPECT_EXCEPTION( std::out_of_range, c1 (e3) );
}


int main()
{
  std::cout << "AthContainers/JaggedVecConversions_test\n";
  test_JaggedVecConstConverter();
  return 0;
}
