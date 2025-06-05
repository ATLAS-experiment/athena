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
 * (benchmarks can be moved when we have the
 * right externals)
 */

#ifndef GSFFindIndexOfMimimum_H
#define GSFFindIndexOfMimimum_H
#include "CxxUtils/features.h"
#include "CxxUtils/inline_hints.h"
#include "CxxUtils/restrict.h"
#include "CxxUtils/vec.h"
//
#include <algorithm>
#include <memory>
#include <numeric>
#include <climits>

namespace vAlgs{
/// In the following
/// ISA_WIDTH is the ISA width in bits e.g 128 for SSE
/// 256 for AVX2 etc
/// For the cases of interest/tested doing 4 simd vectors
/// at a time seemed best.

///@brief Alignment needed for  arrays of elements
///when using an ISA of specific width
///e.g 32 for AVX2 16 for SSE4 etc
template <size_t ISA_WIDTH>
constexpr size_t alignmentForArray(){
  return ISA_WIDTH / CHAR_BIT;
}

/// @brief returns the STRIDE in units of elements
/// covered by NumSIMDVec simd
/// vectors of type T for the specific ISA
/// For example for a 256 bit, a SIMD vector is 8 floats
/// So 4 simd vectors correspond to 32 elements.
template <size_t ISA_WIDTH, typename T>
constexpr size_t strideOfNumSIMDVec(size_t NumSIMDVec){
  return NumSIMDVec * (ISA_WIDTH / (sizeof(T) * CHAR_BIT));
}

/// @brief Given a number n returns a
/// new n >= n that is padded to
/// the required STRIDE
template <size_t STRIDE>
constexpr int numPadded(const int n) {
  // This always return a padded number dividable
  // with STRIDE , eg if STRIDE = 16
  // e.g ((33+15)&~15) = 48
  constexpr size_t STRIDEMINUS1 = STRIDE - 1;
  return ((n + STRIDEMINUS1) & ~STRIDEMINUS1);
}

/// @brief Find the minimum element
/// in the array of distances
/// processing four simd vectors at a time
template <size_t ISA_WIDTH, typename T>
ATH_ALWAYS_INLINE
T vFindMinimum(const T* distancesIn, int n) {

  using namespace CxxUtils;
  static_assert(std::is_floating_point_v<T>, "T not a floating point type");
  constexpr size_t VEC_WIDTH = ISA_WIDTH / (sizeof(T) * CHAR_BIT);
  const T* array = std::assume_aligned<alignmentForArray<ISA_WIDTH>()>(distancesIn);
  using vec_t = vec<T, VEC_WIDTH>;
  vec_t minValues1;
  vec_t minValues2;
  vec_t minValues3;
  vec_t minValues4;
  vload(minValues1, array);
  vload(minValues2, array + VEC_WIDTH);
  vload(minValues3, array + VEC_WIDTH * 2);
  vload(minValues4, array + VEC_WIDTH * 3);
  vec_t values1;
  vec_t values2;
  vec_t values3;
  vec_t values4;
  for (int i = 4 * VEC_WIDTH; i < n; i += 4 * VEC_WIDTH) {
    // 1
    vload(values1, array + i);
    vmin(minValues1, values1, minValues1);
    // 2
    vload(values2, array + i + VEC_WIDTH);
    vmin(minValues2, values2, minValues2);
    // 3
    vload(values3, array + i + 2 * VEC_WIDTH);
    vmin(minValues3, values3, minValues3);
    // 4
    vload(values4, array + i + 3 * VEC_WIDTH);
    vmin(minValues4, values4, minValues4);
  }
  // Compare //1 with //2
  vmin(minValues1, minValues1, minValues2);
  // compare //3 with //4
  vmin(minValues3, minValues3, minValues4);
  // Compare //1 with //3
  vmin(minValues1, minValues1, minValues3);
  // Do the final calculation scalar way
  T finalMinValues[VEC_WIDTH];
  vstore(finalMinValues, minValues1);

  // Do the final calculation scalar way
  return std::reduce(std::begin(finalMinValues), std::end(finalMinValues),
                     finalMinValues[0],
                     [](T a, T b) { return a < b ? a : b; });
}

/// @brief Find the index of an element
/// in the array of distances
/// processing four simd vectors at a time.
/// Returns -1 if the element is not found.
template <size_t ISA_WIDTH, typename T>
ATH_ALWAYS_INLINE
int vIdxOfValue(const T value,
                const T* distancesIn, int n) {
  using namespace CxxUtils;

  static_assert(std::is_floating_point_v<T>, "T not a floating point type");
  constexpr int VEC_WIDTH = ISA_WIDTH / (sizeof(T) * CHAR_BIT);
  const T* array = std::assume_aligned<alignmentForArray<ISA_WIDTH>()>(distancesIn);
  using vec_t = vec<T, VEC_WIDTH>;
  using vec_mask = vec_mask_type_t<vec_t>;
  vec_t values1;
  vec_t values2;
  vec_t values3;
  vec_t values4;
  vec_t target;
  vbroadcast(target, value);
  for (int i = 0; i < n; i += 4 * VEC_WIDTH) {
    // 1
    vload(values1, array + i);
    vec_mask eq1 = values1 == target;
    // 2
    vload(values2, array + i + VEC_WIDTH);
    vec_mask eq2 = values2 == target;
    // 3
    vload(values3, array + i + VEC_WIDTH * 2);
    vec_mask eq3 = values3 == target;
    // 4
    vload(values4, array + i + VEC_WIDTH * 3);
    vec_mask eq4 = values4 == target;

    vec_mask eq12 = eq1 || eq2;
    vec_mask eq34 = eq3 || eq4;
    vec_mask eqAny = eq12 || eq34;
    if (vany(eqAny)) {
      for (int idx = i; idx < i + 4 * VEC_WIDTH; ++idx) {
        if (distancesIn[idx] == value) {
          return idx;
        }
      }
    }
  }
  return -1;
}

/// @brief Find the index of the minimum
/// in the array of distances.
/// Inputs are a ptr to the data
/// and the number of elements to
/// examine. The input needs to be properly
/// padded (see numpadded method)
template <int ISA_WIDTH, typename T>
ATH_ALWAYS_INLINE
int vIdxOfMin(const T* distancesIn, int n) {
  using namespace CxxUtils;
  const T* array = std::assume_aligned<vAlgs::alignmentForArray<ISA_WIDTH>()>(distancesIn);
  static_assert(std::is_floating_point_v<T>, "T not a floating point type");
  //We process elements in blocks.
  //When we find the minimum we also
  //keep track in which block it was.
  //The blocksize of 512 seemed to be a good enough
  //compromise in tests.
  constexpr int blockSize = 512;
  // case for n less than blockSize
  if (n <= blockSize) {
    T min = vFindMinimum<ISA_WIDTH>(array, n);
    return vIdxOfValue<ISA_WIDTH>(min, array, n);
  }
  int idx = 0;
  T min = array[0];
  // We might have a remainder, elements after an integral
  // number of blockes that we need to handle
  const int remainder = n & (blockSize - 1);
  // Process elements up to the remainder in blocks
  // For example for blockSize 512 and 1056 elements
  // (if we opt for padding/multiple of 32)
  // The loop will run two times and then we need
  // to handle the 32 remaining elements.
  for (int i = 0; i < (n - remainder); i += blockSize) {
    T mintmp = vFindMinimum<ISA_WIDTH>(array + i, blockSize);
    if (mintmp < min) {
      min = mintmp;
      idx = i;
    }
  }
  //Process the remaining elements if any
  if (remainder != 0) {
    int index = n - remainder;
    T mintmp = vFindMinimum<ISA_WIDTH>(array + index, remainder);
    if (mintmp < min) {
      min = mintmp;
      return index + vIdxOfValue<ISA_WIDTH>(min, array + index, remainder);
    }
  }
  // Return the idx of the minimum just looping over a single block
  return idx + vIdxOfValue<ISA_WIDTH>(min, array + idx, blockSize);
}

} // namespace vAlgs

#endif
