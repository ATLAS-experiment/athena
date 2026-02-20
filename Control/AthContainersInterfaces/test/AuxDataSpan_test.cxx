/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersInterfaces/test/AuxDataSpan_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Regression test for AuxDataSpan.
 */


#undef NDEBUG
#include "AthContainersInterfaces/AuxDataSpan.h"
#include "TestTools/expect_exception.h"
#include <iostream>
#include <cassert>


void test1()
{
  std::cout << "test1\n";
  int ii[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  SG::AuxDataSpanBase sb1;
  SG::AuxDataSpanBase sb2 (ii, std::size(ii));

  SG::detail::AuxDataSpan<int> s1 (sb1);
  assert (s1.size() == 0);
  assert (s1.empty());

  SG::detail::AuxDataSpan<int> s2 (sb2);
  assert (s2.size() == 10);
  assert (!s2.empty());

  SG::detail::AuxDataConstSpan<int> cs1 (sb1);
  assert (cs1.size() == 0);
  assert (cs1.empty());

  SG::detail::AuxDataConstSpan<int> cs2 (sb2);
  assert (cs2.size() == 10);
  assert (!cs2.empty());

  const SG::detail::AuxDataSpan<int>& s2_c = s2;
  assert (s2.data() == ii);
  assert (s2_c.data() == ii);
  assert (cs2.data() == ii);

  assert (s2[2] == 2);
  s2[2] = 22;
  assert (s2_c[2] == 22);
  assert (cs2[2] == 22);

  assert (s2.at(3) == 3);
  s2.at(3) = 33;
  assert (s2_c.at(3) == 33);
  assert (cs2.at(3) == 33);

  EXPECT_EXCEPTION( std::out_of_range, s2.at(100) );
  EXPECT_EXCEPTION( std::out_of_range, s2_c.at(100) );
  EXPECT_EXCEPTION( std::out_of_range, cs2.at(100) );

  assert (s2.front() == 0);
  assert (s2_c.front() == 0);
  assert (cs2.front() == 0);

  assert (s2.back() == 9);
  assert (s2_c.back() == 9);
  assert (cs2.back() == 9);

  sb2.size = 5;
  assert (s2.size() == 5);
  assert (cs2.size() == 5);
}

//coverity[root_function]
int main()
{
  std::cout << "AthContainersInterfaces/AuxDataSpan_test\n";
  test1();
}
