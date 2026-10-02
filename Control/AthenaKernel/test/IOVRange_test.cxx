/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/test/IOVRange_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2026
 * @brief Regression tests for IOVRange.
 */

#undef NDEBUG

#include "AthenaKernel/IOVRange.h"
#include <cassert>
#include <print>
#include <sstream>


void test1()
{
  std::println ("test1");
  IOVTime t1 (1234);
  IOVTime t2 (5678);
  IOVRange r1 (t1, t2);
  assert (r1.start() == t1);
  assert (r1.stop() == t2);
  assert (r1.isInRange (IOVTime (3333)));
  assert (!r1.isInRange (IOVTime (1000)));
  assert (!r1.isInRange (IOVTime (9999)));

  IOVTime t3 (3456);
  IOVRange r2 (t1, t3);
  assert (r1 == r1);
  assert (r1 != r2);

  std::string s1 = r1;
  assert (s1 == "{[1234] - [5678]}");
  std::ostringstream s2;
  s2 << r1;
  assert (s2.str() == s1);
  std::ostringstream s3;
  std::print (s3, "{}", r1);
  assert (s3.str() == s1);
}


int main()
{
  std::println ("AthenaKernel/IOVTime_test");
  test1();
  return 0;
}
