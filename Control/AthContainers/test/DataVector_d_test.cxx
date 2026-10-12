/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/DataVector_d_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Regression tests for DataVector
 *
 * The @c DataVector regression tests are split into several pieces,
 * in order to reduce the memory required for compilation.
 */

#undef NDEBUG


#include "DataVector_test.icc"


void test2_d()
{
  std::println ("test2_d");
  do_test2<M, R> ();
}


int main()
{
  try {
    test2_d();
  }
  catch (const std::runtime_error& e) {
    std::println (std::cerr, "{}", e.what());
    return 1;
  }
  return 0;
}
