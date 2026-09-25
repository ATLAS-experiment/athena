/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthLinks/test/exceptions_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2014
 * @brief Regression tests for exceptions.
 */


#undef NDEBUG
#include "AthLinks/exceptions.h"
#include <print>


void test1()
{
  std::println ("test1");

  std::println ("{}", SG::ExcPointerNotInSG((char*)0x1234).what());
  std::println ("{}", SG::ExcCLIDMismatch(123, 456).what());
  std::println ("{}", SG::ExcInvalidLink (123, "key", 765).what());
  std::println ("{}", SG::ExcBadForwardLink (123, 345, "Foo").what());
  std::println ("{}", SG::ExcElementNotFound ("test").what());
  std::println ("{}", SG::ExcInvalidIndex ("test").what());
  std::println ("{}", SG::ExcIndexNotFound ("test").what());
  std::println ("{}", SG::ExcIncomparableEL().what());
  std::println ("{}", SG::ExcBadToTransient().what());
  std::println ("{}", SG::ExcConstStorable (123, "key", 765).what());
  std::println ("{}", SG::ExcBadThinning (123, "key", 765).what());
}


int main()
{
  test1();
  return 0;
}
