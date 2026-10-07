/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/nodiscard_test.cxx
 * @brief test for ATLAS_NODISCARD
 */

#include "CxxUtils/nodiscard.h"
#include <print>
ATLAS_NODISCARD
int
foo()
{
  return 1;
}

int
main()
{
  int bar = foo();
  std::println ("{}", bar);
  return 0;
}
