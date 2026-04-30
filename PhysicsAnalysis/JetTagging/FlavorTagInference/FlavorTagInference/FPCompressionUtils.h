/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  Header-only utilities for reduced-precision float compression.

  Bit-truncation of the float32 representation, parameterised by exponent-
  and mantissa-bit budgets (E, M):

    - truncateToFloat(v, E, M) : RNE-truncated float32, low (31-E-M) bits
      zero, exponent range clamped to what E bits can address.

  Special case: (E=8, M=7) reproduces bfloat16 exactly (no exponent
  clamping since E==8 covers float32's full range).

  All functions use round-to-nearest-even.
*/

#ifndef FLAVORTAGINFERENCE_FPCOMPRESSIONUTILS_H
#define FLAVORTAGINFERENCE_FPCOMPRESSIONUTILS_H

#include <bit>
#include <cmath>
#include <cstdint>

namespace FlavorTagInference {
namespace FPCompressionUtils {

  // RNE-truncated float32 with (exp_bits, mantissa_bits) precision budget.
  // Low (31-E-M) bits are zeroed. If exp_bits < 8, values outside the
  // representable exponent range are saturated (over) or flushed to zero
  // (under); NaN/Inf pass through.
  inline float truncateToFloat(float val, int exp_bits, int mantissa_bits) {
    uint32_t bits = std::bit_cast<uint32_t>(val);

    // Exponent clamping (only meaningful if E < 8; float32 uses E=8).
    if (exp_bits < 8) {
      const int max_exp = (1 << (exp_bits - 1)) - 1;  // e.g. E=5 -> 15
      const int min_exp = -(max_exp - 1);
      const int exp_raw = static_cast<int>((bits >> 23) & 0xFFu);
      if (exp_raw != 0xFF) {  // pass NaN/Inf through
        const int actual_exp = exp_raw - 127;
        if (actual_exp > max_exp) {
          const uint32_t sign = bits & 0x80000000u;
          const uint32_t sat_exp = static_cast<uint32_t>(max_exp + 127) << 23;
          const uint32_t sat_man = (1u << 23) - 1u;
          bits = sign | sat_exp | sat_man;
        } else if (actual_exp < min_exp) {
          bits &= 0x80000000u;
        }
      }
    }

    // RNE-round: zero low k bits with round-to-nearest-even bias.
    const int k = 31 - exp_bits - mantissa_bits;
    if (k > 0 && k < 32) {
      const uint32_t mask = ~((1u << k) - 1u);
      const uint32_t round_bias = (1u << (k - 1)) - 1u + ((bits >> k) & 1u);
      bits = (bits + round_bias) & mask;
    }
    return std::bit_cast<float>(bits);
  }

} // namespace FPCompressionUtils
} // namespace FlavorTagInference

#endif // FLAVORTAGINFERENCE_FPCOMPRESSIONUTILS_H
