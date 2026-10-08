/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

----------------------------------------------------------------------
Based on Xoroshiro code with copyright notice:
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
#ifndef PILEUPMT_FASTRESEEDEDPRNG_H
#define PILEUPMT_FASTRESEEDEDPRNG_H
#include <array>
#include <cstdint>
#include <limits>

class FastReseededPRNG {
 public:
  using result_type = std::uint64_t;
  template <typename... Int>
  FastReseededPRNG(Int... seed) {
    const std::array<std::uint64_t, sizeof...(seed)> seed_array{
        static_cast<std::uint64_t>(seed)...};
    set_seed(seed_array.data(), seed_array.size());
  }
  FastReseededPRNG() = delete;  // No default constructor. Always need a seed.

  static constexpr result_type min() { return 0; }
  static constexpr result_type max() {
    return std::numeric_limits<std::uint64_t>::max();
  }

  result_type operator()();

 private:
  std::array<std::uint64_t, 2> m_seed_arr;
  void set_seed(const std::uint64_t* start, std::size_t len);
  std::uint64_t next();
};

#endif  // PILEUPMT_FASTRESEEDEDPRNG_H
