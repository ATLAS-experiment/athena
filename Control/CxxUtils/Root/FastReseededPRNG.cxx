/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

----------------------------------------------------------------------
  Xoroshiro code copyright notice:
Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>.
----------------------------------------------------------------------

This provides a uniform random bit generator using xoroshiro128**
seeded using 128 bit XXH3 to hash the provided seeds.

Compared to other URBGs this is very cheap to construct, which is useful
when the PRNG is reseeded every event and only used to generate a few random
numbers.
*/

#include "CxxUtils/FastReseededPRNG.h"

#define XXH_INLINE_ALL
#include "xxhash.h"

FastReseededPRNG::result_type FastReseededPRNG::operator()() {
  return next();
}

// Set seed
void FastReseededPRNG::set_seed(const std::uint64_t* start, std::size_t len) {
  const auto hash = XXH3_128bits(start, sizeof(std::uint64_t) * len);
  m_seed_arr[0] = hash.high64;
  m_seed_arr[1] = hash.low64;
}

// Reference xoroshiro128** implementation, with modifications to move to C++:
namespace {
std::uint64_t rotl(const std::uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}
}  // namespace

std::uint64_t FastReseededPRNG::next() {
  const std::uint64_t s0 = m_seed_arr[0];
  std::uint64_t s1 = m_seed_arr[1];
  const std::uint64_t result = rotl(s0 * 5, 7) * 9;

  s1 ^= s0;
  m_seed_arr[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);  // a, b
  m_seed_arr[1] = rotl(s1, 37);                    // c

  return result;
}
