/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/tests/bitmask_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2018
 * @brief Unit tests for bitmask.
 */


#undef NDEBUG
#include "CxxUtils/bitmask.h"
#include <cassert>
#include <print>


enum class Bits { None=0, One=1, Two=2, Three=4, Four=8,
                  ATH_BITMASK };

enum class Bits2 { None=0, Five=16, Six=32, Seven=64,
                   ATH_BITMASK };

void test1()
{
  std::println ("test1");
  Bits b = Bits::None;
  b |= Bits::Two;
  assert (static_cast<unsigned>(b) == 2);
  b ^= (Bits::One|Bits::Two|Bits::Three);
  assert (static_cast<unsigned>(b) == 5);
  b &= ~Bits::One;
  assert (static_cast<unsigned>(b) == 4);

  assert (static_cast<unsigned> (b & Bits::Three) == 4);
  assert (static_cast<unsigned> (b & Bits::Two) == 0);
  assert (static_cast<unsigned> (b | Bits::One) == 5);
  assert (static_cast<unsigned> (b ^ Bits::One) == 5);
  assert (static_cast<unsigned> (b ^ Bits::Three) == 0);

  assert (!CxxUtils::test (b, Bits::One));
  assert (CxxUtils::test (b, Bits::Three));
  CxxUtils::set (b, Bits::One);
  assert (static_cast<unsigned>(b) == 5);
  assert (CxxUtils::test (b, Bits::One));
  assert (CxxUtils::test (b, Bits::Three));
  CxxUtils::reset (b, Bits::Three);
  assert (static_cast<unsigned>(b) == 1);
  assert (CxxUtils::test (b, Bits::One));
  assert (!CxxUtils::test (b, Bits::Three));
}


void test2()
{
  std::println ("test2");
  assert ((Bits::One | Bits2::Five) == 17);
  assert ((Bits::One & Bits2::None) == 0);
  assert ((Bits::One ^ Bits2::Six) == 33);
  assert ((Bits::None | 1 | Bits2::Five) == 17);
  assert ((42 & Bits::Four & Bits2::None) == 0);
  assert ((Bits::One ^ 2 ^ Bits2::Six) == 35);
}


int main()
{
  std::println ("bitmask_test");
  test1();
  test2();
  return 0;
}
