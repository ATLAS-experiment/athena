/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/crc_combine_test.cxx
 * @brief Regression tests for CRC combine functions
 */

#undef NDEBUG

#include "CxxUtils/crc_combine.h"
#include <iostream>
#include <cassert>
#include <string>


void test_crc_combine_uint32()
{
  std::cout << "test_crc_combine_uint32\n";

  // Test combining two uint32_t values
  uint32_t seed1 = 0x12345678;
  uint32_t val1 = 0x87654321;
  uint32_t result1 = CxxUtils::crc_combine(seed1, val1);

  // Test that the same inputs produce the same result (reproducibility)
  uint32_t result1_again = CxxUtils::crc_combine(seed1, val1);
  assert(result1 == result1_again);

  // Test with different values
  uint32_t seed2 = 0xdeadbeef;
  uint32_t val2 = 0xcafebabe;
  uint32_t result2 = CxxUtils::crc_combine(seed2, val2);
  assert(result2 != result1);  // Different inputs should produce different results

  // Test with zero values
  uint32_t result_zero = CxxUtils::crc_combine(0, 0);
  assert(result_zero != 0);  // CRC of zeros should still produce a value

  // Test that order matters
  uint32_t result_reversed = CxxUtils::crc_combine(val1, seed1);
  assert(result_reversed != result1);  // Swapping order should produce different result

  std::cout << "  seed1=" << std::hex << seed1 << " val1=" << val1 
            << " result=" << result1 << std::dec << "\n";
}


void test_crc_combine_string()
{
  std::cout << "test_crc_combine_string\n";

  // Test combining a seed with a string
  uint32_t seed = 0x12345678;
  std::string str1 = "hello";
  uint32_t result1 = CxxUtils::crc_combine(seed, str1);

  // Test that the same inputs produce the same result (reproducibility)
  uint32_t result1_again = CxxUtils::crc_combine(seed, str1);
  assert(result1 == result1_again);

  // Test with different strings
  std::string str2 = "world";
  uint32_t result2 = CxxUtils::crc_combine(seed, str2);
  assert(result2 != result1);  // Different strings should produce different results

  // Test with empty string
  std::string empty_str = "";
  uint32_t result_empty = CxxUtils::crc_combine(seed, empty_str);
  assert(result_empty != result1);  // Empty string with same seed should still differ

  // Test with longer strings
  std::string long_str = "This is a longer test string with multiple words";
  uint32_t result_long = CxxUtils::crc_combine(seed, long_str);
  assert(result_long != result1);

  std::cout << "  seed=" << std::hex << seed << " str=\"" << str1 << "\" result=" 
            << result1 << std::dec << "\n";
}


int main()
{
  test_crc_combine_uint32();
  test_crc_combine_string();
  std::cout << "All tests passed!\n";
  return 0;
}
