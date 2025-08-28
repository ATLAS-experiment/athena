/*
  Copyright (C) 2002-2015, 2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/tests/byteswap_list.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2025
 * @brief Unit test for byteswap.
 */

#undef NDEBUG


#include "CxxUtils/byteswap.h"
#include <cassert>
#include <iostream>
#include <cstdint>


void test1()
{
  std::cout << "test1\n";

  static const uint8_t u8 = 0x01;
  assert (CxxUtils::byteswap(u8) == 0x01);

  static const uint16_t u16 = 0x0102;
  assert (CxxUtils::byteswap(u16) == 0x0201);

  static const uint32_t u32 = 0x01020304;
  assert (CxxUtils::byteswap(u32) == 0x04030201);

  static const uint64_t u64 = 0x0102030405060708;
  assert (CxxUtils::byteswap(u64) == 0x0807060504030201);
}


int main()
{
  std::cout << "CxxUtils/byteswap_test\n";
  test1();
  return 0;
}
