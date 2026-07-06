/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#define BOOST_TEST_MODULE FloatCompressor_test
#include <boost/test/unit_test.hpp>

#include "CxxUtils/FloatCompressor.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace {

constexpr unsigned int NMANTISSA = 23U;

std::uint32_t
floatToBits(float value) {
  std::uint32_t bits{};
  std::memcpy(&bits, &value, sizeof(bits));
  return bits;
}

float
bitsToFloat(std::uint32_t bits) {
  float value{};
  std::memcpy(&value, &bits, sizeof(value));
  return value;
}

unsigned int
clampedMantissaBits(unsigned int mantissaBits) {
  if (mantissaBits < 5U) {
    return 5U;
  }
  if (mantissaBits > NMANTISSA) {
    return NMANTISSA;
  }
  return mantissaBits;
}

float
referenceReduceFloatPrecision(float value, unsigned int mantissaBits) {
  mantissaBits = clampedMantissaBits(mantissaBits);

  if (mantissaBits == NMANTISSA) {
    return value;
  }

  if (!std::isfinite(value)) {
    return value;
  }

  std::uint32_t mantissaBitmask{};

  for (unsigned int i = 0; i < (NMANTISSA - mantissaBits); ++i) {
    mantissaBitmask |= (std::uint32_t{1} << i);
  }

  mantissaBitmask = ~mantissaBitmask;

  const std::uint32_t rounding =
      std::uint32_t{1} << (32U - (1U + 8U + mantissaBits) - 1U);

  std::uint32_t vmax = std::uint32_t{0x7f7} << 20U;
  vmax |= std::uint32_t{0x000fffff} xor rounding;

  std::uint32_t bits = floatToBits(value);

  if ((bits & std::uint32_t{0x7fffffff}) < vmax) {
    bits += rounding;
  }

  bits &= mantissaBitmask;

  return bitsToFloat(bits);
}

void
checkCompression(float value, unsigned int mantissaBits) {
  const CxxUtils::FloatCompressor compressor{mantissaBits};

  const float compressed = compressor.reduceFloatPrecision(value);
  const float expected = referenceReduceFloatPrecision(value, mantissaBits);

  BOOST_TEST_CONTEXT("value = " << value << ", mantissaBits = " << mantissaBits) {
    BOOST_TEST(floatToBits(compressed) == floatToBits(expected));
  }
}

} // namespace

BOOST_AUTO_TEST_CASE(default_compressor_matches_reference) {
  const CxxUtils::FloatCompressor compressor;

  const std::vector<float> values{
      0.0F,
      -0.0F,
      1.0F,
      -1.0F,
      1.2345678F,
      -1.2345678F,
      12345.678F,
      -12345.678F,
      1.0e-10F,
      -1.0e-10F,
      1.0e10F,
      -1.0e10F
  };

  for (const float value : values) {
    const float compressed = compressor.reduceFloatPrecision(value);
    const float expected = referenceReduceFloatPrecision(value, 7U);

    BOOST_TEST_CONTEXT("value = " << value) {
      BOOST_TEST(floatToBits(compressed) == floatToBits(expected));
    }
  }
}

BOOST_AUTO_TEST_CASE(different_mantissa_precisions_match_reference) {
  const std::vector<unsigned int> mantissaBitCounts{
      0U,
      1U,
      4U,
      5U,
      7U,
      10U,
      16U,
      20U,
      23U,
      24U,
      100U
  };

  const std::vector<float> values{
      0.0F,
      -0.0F,
      0.1F,
      -0.1F,
      0.5F,
      -0.5F,
      1.0F,
      -1.0F,
      1.5F,
      -1.5F,
      3.1415927F,
      -3.1415927F,
      1000.125F,
      -1000.125F,
      1.0e-20F,
      -1.0e-20F,
      1.0e20F,
      -1.0e20F
  };

  for (const unsigned int mantissaBits : mantissaBitCounts) {
    for (const float value : values) {
      checkCompression(value, mantissaBits);
    }
  }
}

BOOST_AUTO_TEST_CASE(non_finite_values_are_preserved) {
  const CxxUtils::FloatCompressor compressor{7U};

  const float positiveInfinity = std::numeric_limits<float>::infinity();
  const float negativeInfinity = -std::numeric_limits<float>::infinity();
  const float quietNaN = std::numeric_limits<float>::quiet_NaN();

  BOOST_TEST(floatToBits(compressor.reduceFloatPrecision(positiveInfinity)) ==
             floatToBits(positiveInfinity));

  BOOST_TEST(floatToBits(compressor.reduceFloatPrecision(negativeInfinity)) ==
             floatToBits(negativeInfinity));

  const float compressedNaN = compressor.reduceFloatPrecision(quietNaN);
  BOOST_TEST(std::isnan(compressedNaN));
}

BOOST_AUTO_TEST_CASE(full_precision_is_noop) {
  const CxxUtils::FloatCompressor compressor{23U};

  const std::vector<float> values{
      0.0F,
      -0.0F,
      0.1F,
      -0.1F,
      1.2345678F,
      -1.2345678F,
      1.0e20F,
      -1.0e20F,
      std::numeric_limits<float>::infinity(),
      -std::numeric_limits<float>::infinity()
  };

  for (const float value : values) {
    BOOST_TEST(floatToBits(compressor.reduceFloatPrecision(value)) ==
               floatToBits(value));
  }
}

BOOST_AUTO_TEST_CASE(requested_precision_is_clamped_to_valid_range) {
  const std::vector<float> values{
      0.1F,
      -0.1F,
      1.2345678F,
      -1.2345678F,
      12345.678F,
      -12345.678F
  };

  const CxxUtils::FloatCompressor tooLow{1U};
  const CxxUtils::FloatCompressor minimum{5U};

  const CxxUtils::FloatCompressor tooHigh{100U};
  const CxxUtils::FloatCompressor full{23U};

  for (const float value : values) {
    BOOST_TEST_CONTEXT("value = " << value) {
      BOOST_TEST(floatToBits(tooLow.reduceFloatPrecision(value)) ==
                 floatToBits(minimum.reduceFloatPrecision(value)));

      BOOST_TEST(floatToBits(tooHigh.reduceFloatPrecision(value)) ==
                 floatToBits(full.reduceFloatPrecision(value)));
    }
  }
}

BOOST_AUTO_TEST_CASE(compression_is_idempotent) {
  const std::vector<unsigned int> mantissaBitCounts{
      5U,
      7U,
      10U,
      16U,
      20U,
      23U
  };

  const std::vector<float> values{
      0.1F,
      -0.1F,
      1.2345678F,
      -1.2345678F,
      12345.678F,
      -12345.678F,
      1.0e-10F,
      -1.0e-10F
  };

  for (const unsigned int mantissaBits : mantissaBitCounts) {
    const CxxUtils::FloatCompressor compressor{mantissaBits};

    for (const float value : values) {
      const float once = compressor.reduceFloatPrecision(value);
      const float twice = compressor.reduceFloatPrecision(once);

      BOOST_TEST_CONTEXT("value = " << value << ", mantissaBits = " << mantissaBits) {
        BOOST_TEST(floatToBits(twice) == floatToBits(once));
      }
    }
  }
}