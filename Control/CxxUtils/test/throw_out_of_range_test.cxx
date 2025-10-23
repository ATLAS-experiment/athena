/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/throw_out_of_range_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2025
 * @brief Unit test for throw_out_of_range.
 */


#undef NDEBUG
#include "CxxUtils/throw_out_of_range.h"
#include <stdexcept>
#include <iostream>
#include <cassert>



void test1()
{
  void* obj = reinterpret_cast<void*>(0x1234);

  std::string what;

  try {
    CxxUtils::throw_out_of_range ("test1a", 10, 5, obj);
  }
  catch (const std::out_of_range& e) {
    what = e.what();
  }
  assert (what == "CxxUtils::throw_out_of_range test1a requested index 10 >= 5 for object at 0x1234");

  try {
    CxxUtils::throw_out_of_range (std::string("test1b"), 10, 5, obj);
  }
  catch (const std::out_of_range& e) {
    what = e.what();
  }
  assert (what == "CxxUtils::throw_out_of_range test1b requested index 10 >= 5 for object at 0x1234");
}


int main()
{
  std::cout << "CxxUtils/throw_out_of_range_test\n";
  test1();
  return 0;
}
