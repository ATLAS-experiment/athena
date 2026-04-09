/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  Header-only utilities for converting IEEE 754 single-precision floats
  to reduced-precision formats encoded as uint16_t:

  - floatToFP16: IEEE 754 half-precision (1+5+10 bits, range +-65504)
  - floatToBF16: bfloat16 (1+8+7 bits, same range as float32)

  Both use round-to-nearest-even for maximum accuracy.
*/

#ifndef FLAVORTAGINFERENCE_FP16UTILS_H
#define FLAVORTAGINFERENCE_FP16UTILS_H

#include <cmath>
#include <cstdint>
#include <cstring>

namespace FlavorTagInference {
namespace FP16Utils {

  // bfloat16: truncate lower 16 mantissa bits from float32.
  // Same exponent range as float32, so no overflow/underflow handling
  // needed.  NaN and Inf propagate correctly.
  inline uint16_t floatToBF16(float val) {
    uint32_t bits;
    std::memcpy(&bits, &val, sizeof(bits));
    // Round-to-nearest-even: add 0x7FFF (half an ulp) plus the
    // rounding bit so ties go to even.
    bits += 0x7FFFu + ((bits >> 16) & 1u);
    return static_cast<uint16_t>(bits >> 16);
  }

  inline uint16_t floatToFP16(float val) {
    if (std::isnan(val)) return 0x7E00u;
    if (std::isinf(val)) return val > 0 ? 0x7C00u : 0xFC00u;

    uint32_t bits;
    std::memcpy(&bits, &val, sizeof(bits));

    uint32_t sign = (bits >> 16) & 0x8000u;
    int32_t exponent = ((bits >> 23) & 0xFF) - 127;
    uint32_t mantissa = bits & 0x007FFFFFu;

    if (exponent > 15) {
      // Overflow: clamp to fp16 max (65504)
      return static_cast<uint16_t>(sign | 0x7BFFu);
    }
    if (exponent > -15) {
      // Normal number: round to nearest even
      uint32_t fp16Exp = static_cast<uint32_t>(exponent + 15) << 10;
      uint32_t fp16Man = mantissa >> 13;
      uint32_t round_bit = (mantissa >> 12) & 1u;
      uint32_t sticky = mantissa & 0xFFFu;
      if (round_bit && (sticky || (fp16Man & 1u))) {
        fp16Man++;
        if (fp16Man > 0x3FFu) {
          fp16Man = 0;
          fp16Exp += 0x0400u;
          if (fp16Exp > 0x7C00u) {
            return static_cast<uint16_t>(sign | 0x7BFFu);
          }
        }
      }
      return static_cast<uint16_t>(sign | fp16Exp | fp16Man);
    }
    if (exponent >= -24) {
      // Subnormal: fold exponent into mantissa.
      // fp16Man = mantissa_with_implicit * 2^(exponent+1)
      //         = mantissa_with_implicit >> (-1 - exponent) = mantissa >> shift
      mantissa |= 0x00800000u;
      int shift = -1 - exponent;  // 14..23
      uint32_t fp16Man = mantissa >> shift;
      uint32_t round_bit = (mantissa >> (shift - 1)) & 1u;
      uint32_t sticky = mantissa & ((1u << (shift - 1)) - 1u);
      if (round_bit && (sticky || (fp16Man & 1u))) {
        fp16Man++;
      }
      return static_cast<uint16_t>(sign | fp16Man);
    }
    // Too small: zero
    return static_cast<uint16_t>(sign);
  }

} // namespace FP16Utils
} // namespace FlavorTagInference

#endif // FLAVORTAGINFERENCE_FP16UTILS_H
