/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/pputils_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Tests for pputils.
 */


#undef NDEBUG
#include "CxxUtils/pputils.h"
#include <iostream>
#include <cassert>


void check (int exp, int test=0)
{
  assert (exp == test);
}


int main()
{
  std::cout << "CxxUtils/pputils_test\n";
  check (0 CXXUTILS_PP_FIRST());
  check (1 CXXUTILS_PP_FIRST(1));
  check (1 CXXUTILS_PP_ARG1(1, 2, 3));

  check (0 CXXUTILS_PP_SECOND());
  check (0 CXXUTILS_PP_SECOND(1));
  check (2 CXXUTILS_PP_SECOND(1, 2));
  check (2 CXXUTILS_PP_ARG2(1, 2, 3));

  check (0 CXXUTILS_PP_THIRD());
  check (0 CXXUTILS_PP_THIRD(1));
  check (0 CXXUTILS_PP_THIRD(1, 2));
  check (3 CXXUTILS_PP_THIRD(1, 2, 3));
  check (3 CXXUTILS_PP_ARG3(1, 2, 3, 4));

  check (0 CXXUTILS_PP_FOURTH());
  check (0 CXXUTILS_PP_FOURTH(1));
  check (0 CXXUTILS_PP_FOURTH(1, 2, 3));
  check (4 CXXUTILS_PP_FOURTH(1, 2, 3, 4));
  check (4 CXXUTILS_PP_ARG4(1, 2, 3, 4, 5));

  check (0 CXXUTILS_PP_FIFTH());
  check (0 CXXUTILS_PP_FIFTH(1));
  check (0 CXXUTILS_PP_FIFTH(1, 2, 3, 4));
  check (5 CXXUTILS_PP_FIFTH(1, 2, 3, 4, 5));
  check (5 CXXUTILS_PP_ARG5(1, 2, 3, 4, 5, 6));

  return 0;
}
