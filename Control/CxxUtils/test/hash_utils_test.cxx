/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/hash_utils_test.cxx
 * @brief Unit tests for CxxUtils hash helpers.
 */

#undef NDEBUG

#include "CxxUtils/hash_utils.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

void test_hash_combine()
{
  std::cout << "test_hash_combine\n";

  std::size_t h1 = 0;
  CxxUtils::hash_combine(h1, 42u);
  CxxUtils::hash_combine(h1, 7u);

  std::size_t h2 = 0;
  CxxUtils::hash_combine(h2, 42u);
  CxxUtils::hash_combine(h2, 7u);

  std::size_t h3 = 0;
  CxxUtils::hash_combine(h3, 7u);
  CxxUtils::hash_combine(h3, 42u);

  assert(h1 == h2);
  assert(h1 != h3);
}

void test_hash_range()
{
  std::cout << "test_hash_range\n";

  const std::vector<std::uint64_t> values {1u, 2u, 3u, 5u, 8u};

  std::size_t folded = 0;
  for (std::uint64_t v : values) {
    CxxUtils::hash_combine(folded, v);
  }

  const std::size_t fromIter = CxxUtils::hash_range(values.begin(), values.end());
  const std::size_t fromRange = CxxUtils::hash_range(values);

  assert(folded == fromIter);
  assert(fromIter == fromRange);
  assert(CxxUtils::hash_range(values.begin(), values.begin()) == 0);
}


int main()
{
  test_hash_combine();
  test_hash_range();
  std::cout << "All tests passed!\n";
  return 0;
}
