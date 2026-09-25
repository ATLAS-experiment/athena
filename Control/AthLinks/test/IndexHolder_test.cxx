/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthLinks/test/IndexHolder_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2014
 * @brief Regression tests for IndexHolder.
 */

#undef NDEBUG
#include "AthLinks/tools/IndexHolder.h"
#include <print>
#include <cassert>


void test1()
{
  std::println ("test1");

  SG::IndexHolder<std::string> h;
  assert (!h.isValid());

  h = SG::IndexHolder<std::string> ("asd");
  assert (h.isValid());
  assert (static_cast<const std::string&>(h) == "asd");

  h.reset();
  assert (!h.isValid());
}


int main()
{
  test1();
  return 0;
}
