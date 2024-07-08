/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  GSFFindIndexOfMimimum
 * @author Christos Anastopoulos
 * @author Lucy Lewitt
 *
 *
 * @brief Finding the index of a minimum
 * is an importand operation for the
 * the KL mixture reduction.
 * We rely on having an as fast as
 * possible implementation
 *
 * The issues are described in ATLASRECTS-5244
 * Some timing improvements in the overall
 * GSF refitting algorithm time can be found at :
 * https://gitlab.cern.ch/atlas/athena/-/merge_requests/67962
 * At large a slow implmentation can increase
 * significantly the time for the GSF refititng
 * algorithm.
 *
 * There is literature in the internet
 * namely in blogs by Wojciech Mula
 * and Sergey Slotin
 * They solve the problem for
 * integers using intrinsics and various
 * AVX levels.
 *
 * In ATLAS currently we need to solve it for float types.
 *
 * Furthermore, after discussion with Scott Snyder
 * we opted for using the gnu vector types from "CxxUtils/vec.h".
 *
 * Additionally, gcc for
 * combination that are too wide for the current architecture
 * synthesizes the instructions using a narrower mode.
 * But can result in quite poor assembly in cases.
 * So we opt to pass the ISA width in bits and
 * always use the right sized vector types.
 *
 * The original playground with benchmarking code
 * can be found at:
 * https://github.com/AnChristos/FindIdxOfMinimum
 * (benchmark can be moved when we have the
 * right externals)
 */

#ifndef GSFFindIndexOfMimimum_H
#define GSFFindIndexOfMimimum_H
#include "CxxUtils/features.h"
#include "CxxUtils/inline_hints.h"
#include "CxxUtils/restrict.h"
#include "CxxUtils/vec.h"
#include "GaudiKernel/Kernel.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
//
#include <algorithm>
#include <memory>
#include <numeric>
#include <climits>

namespace vAlgs{

// Functionalit similar to std::reduce for array of vector types.
// The reduction result is stored in array[0].
template <typename T, int N, int SIZE>
void vreduce(CxxUtils::vec<T, N> array[SIZE], const auto& lambda) {

  static_assert(SIZE != 0, "SIZE can not be 0");
  static_assert((SIZE & (SIZE - 1)) == 0 || SIZE == 1,
                "SIZE not 1 or a power of 2");
  if constexpr (SIZE == 1) {
    return;
  }
  for (int i = 0; i < SIZE / 2; i++) {
    lambda(array[i], array[i + (SIZE / 2)]);
  }
  if constexpr (SIZE / 2 > 0) {
    vreduce<T, N, SIZE / 2>(array, lambda);
  } else {
    return;
  }
}

/// ISA_WIDTH is the ISA width in bits e.g 128 for SSE
/// 256 for AVX2
///
/// STRIDE is how many elements (in units of elements) we want to cover
/// in each iteration
///
/// T is the element type
///
/// The input array is assumed to be at least
/// ISA_WIDTH/CHAR_BIT aligned e.g 16 for SSE4, 32 for AVX2
/// The array size is n . n needs to be dividable with STIDE.
/// Aka the array needs to be properly padded.
///
/// Based on the ISA and the element type
/// We choose the best size for the SIMD types
///
/// And then we use an array of as  many SIMD types needed
/// As to cover the stride.

template <int ISA_WIDTH, int STRIDE, typename T>
ATH_ALWAYS_INLINE T
vFindMinimum(const T* distancesIn, int n) {
  using namespace CxxUtils;
  //We want to have vectors that fit on the specified
  //ISA
  //For large STRIDES we use an array of such
  //vectors
  constexpr int VEC_WIDTH = ISA_WIDTH / (sizeof(T) * CHAR_BIT);
  constexpr int ALIGNMENT = ISA_WIDTH / CHAR_BIT;
  constexpr int VECTOR_COUNT = STRIDE / VEC_WIDTH;
  static_assert((ISA_WIDTH & (ISA_WIDTH - 1)) == 0,
                "ISA_WIDTH not a power of 2");
  static_assert((STRIDE & (STRIDE - 1)) == 0,
                "STRIDE not a power of 2");
  static_assert((ALIGNMENT & (ALIGNMENT - 1)) == 0,
                "ALIGNMENT not a power of 2");
  static_assert(VECTOR_COUNT > 0,
                "STRIDE smaller that the selected ISA SIMD width");
  static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "T not a floating or integral type");

  const T* array = std::assume_aligned<ALIGNMENT>(distancesIn);

  using vec_T = vec<T, VEC_WIDTH>;
  vec_T minValues[VECTOR_COUNT];
  // Limit unrolling to 4 for now as too much unrolling can
  // also cause problems.
  // When VECTOR_COUNT is less than 4 the relevant loops
  // are removed
  GAUDI_LOOP_UNROLL(4)
  for (int i = 0; i < VECTOR_COUNT; i++) {
    vload(minValues[i], array + (VEC_WIDTH * i));
  }
  vec_T values[VECTOR_COUNT];
  for (int i = STRIDE; i < n; i += STRIDE) {
    GAUDI_LOOP_UNROLL(4)
    for (int j = 0; j < VECTOR_COUNT; ++j) {
      vload(values[j], array + i + (VEC_WIDTH * j));
      vmin(minValues[j], values[j], minValues[j]);
    }
  }

  vreduce<T, VEC_WIDTH, VECTOR_COUNT>(
      minValues,
      [](vec<T, VEC_WIDTH>& a, vec<T, VEC_WIDTH>& b) { a = a < b ? a : b; });

  T finalMinValues[VEC_WIDTH];
  vstore(finalMinValues, minValues[0]);

  // Do the final calculation scalar way
  return std::reduce(std::begin(finalMinValues), std::end(finalMinValues),
                     finalMinValues[0],
                     [](float a, float b) { return a < b ? a : b; });
}

template <int ISA_WIDTH, int STRIDE, typename T>
ATH_ALWAYS_INLINE
int vIdxOfValue(const T value,
                const T* distancesIn, int n) {
  using namespace CxxUtils;
  //We want to have vectors that fit on the specified
  //ISA
  //For large STRIDES we use an array of such
  //vectors
  constexpr int VEC_WIDTH = ISA_WIDTH / (sizeof(T) * CHAR_BIT);
  constexpr int ALIGNMENT = ISA_WIDTH / CHAR_BIT;
  constexpr int VECTOR_COUNT = STRIDE / VEC_WIDTH;
  static_assert((ISA_WIDTH & (ISA_WIDTH - 1)) == 0,
                "ISA_WIDTH not a power of 2");
  static_assert((STRIDE & (STRIDE - 1)) == 0,
                "STRIDE not a power of 2");
  static_assert((ALIGNMENT & (ALIGNMENT - 1)) == 0,
                "ALIGNMENT not a power of 2");
  static_assert(VECTOR_COUNT > 0,
                "STRIDE smaller that the selected ISA SIMD width");
  static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "T not a floating or integral type");

  const T* array = std::assume_aligned<ALIGNMENT>(distancesIn);

  using vec_T = vec<T, VEC_WIDTH>;
  vec_T values[VECTOR_COUNT];
  vec_T target;
  vbroadcast(target, value);
  using vec_mask = vec_mask_type_t<vec<T, VEC_WIDTH>>;
  vec_mask eqs[VECTOR_COUNT];

  for (int i = 0; i < n; i += STRIDE) {
    GAUDI_LOOP_UNROLL(4)
    for (int j = 0; j < VECTOR_COUNT; j++) {
      vload(values[j], array + i + (VEC_WIDTH * j));
      eqs[j] = values[j] == target;
    }

    vreduce<vec_type_t<vec_mask>, VEC_WIDTH, VECTOR_COUNT>(
        eqs, [](vec_mask& a, vec_mask& b) { a = a || b; });

    // See if we have the value in any
    // of the vectors
    // If yes then use scalar code to locate it
    if (vany(eqs[0])) {
      for (int idx = i; idx < i + STRIDE; ++idx) {
        if (distancesIn[idx] == value) {
          return idx;
        }
      }
    }
  }
  return -1;
}

template <int ISA_WIDTH, int STRIDE, typename T>
ATH_ALWAYS_INLINE
int vIdxOfMin(const T* distancesIn, int n) {
  using namespace CxxUtils;
  constexpr int ALIGNMENT = ISA_WIDTH / CHAR_BIT;
  static_assert((ISA_WIDTH & (ISA_WIDTH - 1)) == 0,
                "ISA_WIDTH not a power of 2");
  static_assert((STRIDE & (STRIDE - 1)) == 0,
                "STRIDE not a power of 2");
  static_assert((ALIGNMENT & (ALIGNMENT - 1)) == 0,
                "ALIGNMENT not a power of 2");
  static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "T not a floating or integral type");

  const T* array = std::assume_aligned<ALIGNMENT>(distancesIn);
  // Finding of minimum needs to loop over all elements
  // But we can run the finding of index only inside a block
  constexpr int blockSize = 512;
  // case for n less than blockSize
  if (n <= blockSize) {
    T min = vFindMinimum<ISA_WIDTH, STRIDE>(array, n);
    return vIdxOfValue<ISA_WIDTH, STRIDE>(min, array, n);
  }
  int idx = 0;
  T min = array[0];
  // We might have a remainder that we need to handle
  const int remainder = n & (blockSize - 1);
  for (int i = 0; i < (n - remainder); i += blockSize) {
    T mintmp = vFindMinimum<ISA_WIDTH, STRIDE>(array + i, blockSize);
    if (mintmp < min) {
      min = mintmp;
      idx = i;
    }
  }
  if (remainder != 0) {
    int index = n - remainder;
    T mintmp = vFindMinimum<ISA_WIDTH, STRIDE>(array + index, remainder);
    // if the minimum is in this part
    if (mintmp < min) {
      min = mintmp;
      return index + vIdxOfValue<ISA_WIDTH, STRIDE>(min, array + index, remainder);
    }
  }
  //default return
  return idx + vIdxOfValue<ISA_WIDTH, STRIDE>(min, array + idx, blockSize);
}

} // namespace vAlgs

namespace GSFFMVDetail {
// Multiversioning is an impl detail here
// if we compile with different ISA. Things  can be changed/moved
// but for now assume we mainly run on AVX2 machines and
// cover as default SSE4 machines.
#if HAVE_FUNCTION_MULTIVERSIONING
[[gnu::target("avx2")]]
int vIdxOfMin(const float* distancesIn, int n) {
  return vAlgs::vIdxOfMin<256, 16>(distancesIn, n);
}
[[gnu::target("default")]]
#endif
int vIdxOfMin(const float* distancesIn, int n) {
  return vAlgs::vIdxOfMin<128, 16>(distancesIn, n);
}

#if HAVE_FUNCTION_MULTIVERSIONING
[[gnu::target("avx2")]]
int vIdxOfMin(const double* distancesIn, int n) {
  return vAlgs::vIdxOfMin<256, 16>(distancesIn, n);
}
[[gnu::target("default")]]
#endif
int vIdxOfMin(const double* distancesIn, int n) {
  return vAlgs::vIdxOfMin<128, 16>(distancesIn, n);
}

}  // namespace GSFFMVDetail

#endif
