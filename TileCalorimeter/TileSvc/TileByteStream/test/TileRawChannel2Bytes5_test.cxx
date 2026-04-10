/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <iostream>


static uint32_t old_deal(uint32_t u){
  uint32_t u1, u0, k, k1, k0;
  u1 = 0;
  u0 = 0;
  k = 1;
  k0 = 1;
  k1 = uint32_t{1} << 16;
  //coverity[INTEGER_OVERFLOW]
  for (int i = 0; i < 16; ++i) {
    if (u & k) u0 |= k0;
    k0 = k0 << 1;
    k  = k  << 1;

    if (u & k) u1 |= k1;

    if (i != 15) {
      k1 = k1 << 1;
      k  = k  << 1;
    }
  }
  return u1 | u0;
}


static uint32_t new_deal(uint32_t u){
  uint32_t lo = 0;
  uint32_t hi = 0;

  for (unsigned i = 0; i < 16; ++i) {
    if (u & (uint32_t{1} << (2 * i))) {
      lo |= uint32_t{1} << i;
    }
    if (u & (uint32_t{1} << (2 * i + 1))) {
      hi |= uint32_t{1} << (16 + i);
    }
  }
  return lo | hi;
}


void test1(){
  std::cout << "test1\n";

  const uint32_t testvals[] = {
    0x00000000u,
    0xffffffffu,
    0x00000001u,
    0x00000002u,
    0x00000003u,
    0x55555555u,
    0xaaaaaaaau,
    0x12345678u,
    0x87654321u,
    0x80000000u,
    0x40000000u
  };

  for (uint32_t x : testvals) {
    assert (new_deal(x) == old_deal(x));
  }
}


int main(){
  std::cout << "_deal_test\n";
  test1();
  return 0;
}