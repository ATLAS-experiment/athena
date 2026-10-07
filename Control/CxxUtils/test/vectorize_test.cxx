/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/vectorize_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2019
 * @brief Regression test for ATH_ENABLE_TREE_VECTORIZATION.  (Parse test only.)
 */

#undef NDEBUG
#include "CxxUtils/vectorize.h"
#include <print>


ATH_ENABLE_TREE_VECTORIZATION;


int main()
{
  std::println ("CxxUtils/vectorize_test");
  return 0;
}
