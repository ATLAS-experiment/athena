/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/StringParse.h"
#include <iostream>
#include <cmath>

// Make sure that asserts will work in opt builds
#undef NDEBUG
#include <cassert>


int main() {
  StringParse s("1234 foo 5.678   1234argh 5.678bleurgh");
  assert(s.size() == 5);

  assert(s.piece<std::string>(1) == "1234");
  assert(s.piece<int>(1) == 1234);
  assert(s.piece<int>(1) == 1234);

  assert(s.piece<std::string>(2) == "foo");
  assert(s.piece<std::string>(2) == "foo");

  assert(s.piece<std::string>(3) == "5.678");
  assert(s.piece<short>(3) == -1);
  assert(std::abs(s.piece<double>(3) - 5.678) < 1e-6);
  assert(std::abs(s.piece<double>(3) - 5.678) < 1e-6);

  assert(s.piece<int>(4) != 1234);
  assert(s.piece<int>(4) == -1);

  assert(std::abs(s.piece<double>(5) + 1) < 1e-6);
  assert(std::abs(s.piece<double>(5) + 1) < 1e-6);
}
