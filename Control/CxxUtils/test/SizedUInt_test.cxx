/*
  Copyright (C) 200223,  CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/SizedUInt.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Regression tests for SizedUInt
 */


#undef NDEBUG

#include "CxxUtils/SizedUInt.h"
#include <cassert>
#include <print>
#include <cstdint>
#include <type_traits>


void test1()
{
  std::println ("test1");
  assert ((std::is_same_v<CxxUtils::detail::SizedUInt<1>::type, uint8_t>));
  assert ((std::is_same_v<CxxUtils::detail::SizedUInt<2>::type, uint16_t>));
  assert ((std::is_same_v<CxxUtils::detail::SizedUInt<4>::type, uint32_t>));
  assert ((std::is_same_v<CxxUtils::detail::SizedUInt<8>::type, uint64_t>));
}


int main()
{
  std::println ("CxxUtils/SizedUint_test");
  test1();
  return 0;
}
