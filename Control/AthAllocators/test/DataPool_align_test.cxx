/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include <cassert>
#include <print>

#include "AthAllocators/ArenaBlockAlignDetail.h"
#include "AthAllocators/DataPool.h"

using namespace std;

// Basic tests
void test() {

  std::println ("test");

  // we should always be fine for this alignment
  using testStruct = SG::ArenaBlockAlignDetail::padForAlign;
  size_t struct_alignment = alignof(testStruct);

  auto g = std::make_unique<testStruct>();
  std::uintptr_t stdPtr = (uintptr_t)(g.get());
  std::println ("Std ptr properly aligned {}", (stdPtr % struct_alignment == 0));
  assert(stdPtr % struct_alignment == 0);

  auto df = std::make_unique<DataPool<testStruct>>(10);
  testStruct* f = df->nextElementPtr();
  std::uintptr_t poolPtr = (uintptr_t)f;
  std::println ("Pool ptr properly aligned {}", (poolPtr % struct_alignment == 0));
  assert(poolPtr % struct_alignment == 0);
}

int main() {
  test();
  return 0;
}
