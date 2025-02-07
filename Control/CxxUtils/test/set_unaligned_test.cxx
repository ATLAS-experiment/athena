/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/get_unaligned_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2025
 * @brief Regression tests for set_unaligned functions.
 */

#undef NDEBUG
#include "CxxUtils/set_unaligned.h"
#include "CxxUtils/restrict.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <stdint.h>


void compare (const uint8_t* arr, const std::vector<uint8_t>& exp)
{
  if (!std::equal (exp.begin(), exp.end(), arr)) {
    std::cout << "Comparison failure\n";
    std::cout << "  expected:";
    for (size_t i = 0; i < exp.size(); i++) std::cout << " " << static_cast<unsigned>(exp[i]);
    std::cout << "\n";
    std::cout << "  observed:";
    for (size_t i = 0; i < exp.size(); i++) std::cout << " " << static_cast<unsigned>(arr[i]);
    std::cout << "\n";
    std::cout.flush();
    std::abort();
  }
}


void test1()
{
  std::cout << "test1\n";
  uint8_t arr[11] = { 0 };
  uint8_t* p = arr+1;
  CxxUtils::set_unaligned<uint8_t> (p, 1);
  compare (arr, { 0, 1, 0});
  assert (p == arr+2);
  p = arr+1;
  CxxUtils::set_unaligned<int8_t> (p, -2);
  compare (arr, { 0, 0xfe, 0});
  assert (p == arr+2);

  p = arr+1;
  CxxUtils::set_unaligned16 (p, 0x0201);
  compare (arr, { 0, 1, 2, 0 });
  assert (p == arr+3);
  p = arr+1;
  CxxUtils::set_unaligned<uint16_t> (p, 0x0302);
  compare (arr, { 0, 2, 3, 0 });
  assert (p == arr+3);
  p = arr+1;
  CxxUtils::set_unaligned<int16_t> (p, -32760);
  compare (arr, { 0, 8, 128, 0 });
  assert (p == arr+3);

  CxxUtils::set_unaligned32 (p, 0x6050403);
  compare (arr, { 0, 8, 128, 3, 4, 5, 6, 0 });
  assert (p == arr+7);
  p = arr+3;
  CxxUtils::set_unaligned<uint32_t> (p, 0x1020304);
  compare (arr, { 0, 8, 128, 4, 3, 2, 1, 0 });
  assert (p == arr+7);
  p = arr+3;
  CxxUtils::set_unaligned<int32_t> (p, -2);
  compare (arr, { 0, 8, 128, 0xfe, 0xff, 0xff, 0xff, 0 });
  assert (p == arr+7);

  uint8_t* ATH_RESTRICT q = arr+1;
  CxxUtils::set_unaligned64 (q, 0x807060504030201);
  compare (arr, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 0 });
  assert (q == arr+9);
  q = arr+1;
  CxxUtils::set_unaligned<uint64_t> (q, 0x203040506070809);
  compare (arr, { 0, 9, 8, 7, 6, 5, 4, 3, 2, 0 });
  assert (q == arr+9);
  q = arr+1;
  CxxUtils::set_unaligned<int64_t> (q, -2);
  compare (arr, { 0, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0 });
  assert (q == arr+9);

  uint8_t arr_f[11] = { 0 };
  uint8_t* pf = arr_f+1;
  CxxUtils::set_unaligned_float (pf, 3.125);
  compare (arr_f, { 0, 0, 0, 0x48, 0x40, 0 });
  assert (pf == arr_f+5);
  pf = arr_f+1;
  CxxUtils::set_unaligned<float> (pf, 6.25);
  compare (arr_f, { 0, 0, 0, 0xc8, 0x40, 0 });
  assert (pf == arr_f+5);

  uint8_t arr_d[11] = { 0 };
  uint8_t* pd = arr_d+1;
  CxxUtils::set_unaligned_double (pd, 3.125);
  compare (arr_d, { 0, 0, 0, 0, 0, 0, 0, 0x09, 0x40 });
  assert (pd == arr_d+9);
  pd = arr_d+1;
  CxxUtils::set_unaligned<double> (pd, 6.25);
  compare (arr_d, { 0, 0, 0, 0, 0, 0, 0, 0x19, 0x40 });
  assert (pd == arr_d+9);
}


int main()
{
  std::cout << "CxxUtils/set_unaligned_test\n";
  test1();
  return 0;
}
