/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  GSFFindIndexOfMimimum
 * @author Christos Anastopoulos
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
 * In ATLAS currently we need to solve it for float.
 * Furthermore, after discussion with Scott Snyder
 * we opted for using the gnu vector types from "CxxUtils/vec.h".
 * And we target x86_64-v2.
 * In this aimplementations a vec<float,4> vec<int,4>
 * is a 4 wide register. And we do operation explicitly
 * 4 elements a time.
 *
 * For completeness and future comparisons
 * we collect
 * - A "C" implementation.
 * - A "STL" implementation.
 * - A "Vec"  implementation always tracking the index.
 * - A "Vec" implementation that updates the index when an new minimum is
 * found. This can be faster when the inputs are not ordered.
 * - A "Vec" implementation that first finds the minimum and then
 *   finds the index. This will be the faster on average.
 *
 * We provide a convenient entry method
 * to select in compile time an implementation
 *
 * The original playground with benchmarking code
 * can be found at:
 * https://github.com/AnChristos/FindIdxOfMinimum
 * (benchmark can be moved when we have the
 * right externals)
 */

#ifndef GSFFindIndexOfMimimum_H
#define GSFFindIndexOfMimimum_H
#include "CxxUtils/inline_hints.h"
#include "CxxUtils/restrict.h"
#include "CxxUtils/vec.h"
#include "GaudiKernel/Kernel.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
//
#include <algorithm>
#include <memory>
#include <numeric>
namespace findIdxOfMinDetail {

// index of minimum scalar
ATH_ALWAYS_INLINE
int32_t scalarC(const float* distancesIn, int n) {
  const float* array =
      std::assume_aligned<GSFConstants::alignment>(distancesIn);
  float minvalue = array[0];
  int minIndex = 0;
  for (int i = 0; i < n; ++i) {
    const float value = array[i];
    if (value < minvalue) {
      minvalue = value;
      minIndex = i;
    }
  }
  return minIndex;
}

// index of minimum STL
ATH_ALWAYS_INLINE
int32_t scalarSTL(const float* distancesIn, int n) {
  const float* array =
      std::assume_aligned<GSFConstants::alignment>(distancesIn);
  return std::distance(array, std::min_element(array, array + n));
}

// index of minimum vec with blend
ATH_ALWAYS_INLINE
int32_t vecAlwaysTrackIdx(const float* distancesIn, int n) {
  using namespace CxxUtils;
  const float* array =
      std::assume_aligned<GSFConstants::alignment>(distancesIn);
  const vec<int, 4> increment = {16, 16, 16, 16};

  vec<int, 4> indices1 = {0, 1, 2, 3};
  vec<int, 4> indices2 = {4, 5, 6, 7};
  vec<int, 4> indices3 = {8, 9, 10, 11};
  vec<int, 4> indices4 = {12, 13, 14, 15};

  vec<int, 4> minIndices1 = indices1;
  vec<int, 4> minIndices2 = indices2;
  vec<int, 4> minIndices3 = indices3;
  vec<int, 4> minIndices4 = indices4;

  vec<float, 4> minValues1;
  vec<float, 4> minValues2;
  vec<float, 4> minValues3;
  vec<float, 4> minValues4;
  vload(minValues1, array);
  vload(minValues2, array + 4);
  vload(minValues3, array + 8);
  vload(minValues4, array + 12);

  vec<float, 4> values1;
  vec<float, 4> values2;
  vec<float, 4> values3;
  vec<float, 4> values4;
  for (int i = 16; i < n; i += 16) {
    // 1
    vload(values1, array + i);  // 0-3
    indices1 = indices1 + increment;
    vec<int, 4> lt1 = values1 < minValues1;
    vselect(minIndices1, indices1, minIndices1, lt1);
    vmin(minValues1, values1, minValues1);
    // 2
    vload(values2, array + i + 4);  // 4-7
    indices2 = indices2 + increment;
    vec<int, 4> lt2 = values2 < minValues2;
    vselect(minIndices2, indices2, minIndices2, lt2);
    vmin(minValues2, values2, minValues2);
    // 3
    vload(values3, array + i + 8);  // 8-11
    indices3 = indices3 + increment;
    vec<int, 4> lt3 = values3 < minValues3;
    vselect(minIndices3, indices3, minIndices3, lt3);
    vmin(minValues3, values3, minValues3);
    // 4
    vload(values4, array + i + 12);  // 12-15
    indices4 = indices4 + increment;
    vec<int, 4> lt4 = values4 < minValues4;
    vselect(minIndices4, indices4, minIndices4, lt4);
    vmin(minValues4, values4, minValues4);
  }

  float minValues[16];
  int minIndices[16];
  vstore(minValues, minValues1);
  vstore(minValues + 4, minValues2);
  vstore(minValues + 8, minValues3);
  vstore(minValues + 12, minValues4);
  vstore(minIndices, minIndices1);
  vstore(minIndices + 4, minIndices2);
  vstore(minIndices + 8, minIndices3);
  vstore(minIndices + 12, minIndices4);

  float minValue = minValues[0];
  int32_t minIndex = minIndices[0];
  for (size_t i = 1; i < 16; ++i) {
    const float value = minValues[i];
    const int32_t index = minIndices[i];
    if (value < minValue) {
      minValue = value;
      minIndex = index;
    } else if (value == minValue && index < minIndex) {
      // we want to return the smallest index
      // in case of 2 same values
      minIndex = index;
    }
  }
  return minIndex;
}

// index of minimum vec quite fast for the "unordered"/"random" case
ATH_ALWAYS_INLINE
int32_t vecUpdateIdxOnNewMin(const float* distancesIn, int n) {
  using namespace CxxUtils;
  const float* array =
      std::assume_aligned<GSFConstants::alignment>(distancesIn);

  int32_t idx = 0;
  float min = distancesIn[0];
  vec<float, 4> minValues;
  vbroadcast(minValues, min);
  vec<float, 4> values1;
  vec<float, 4> values2;
  vec<float, 4> values3;
  vec<float, 4> values4;
  for (int i = 0; i < n; i += 16) {
    // 1
    vload(values1, array + i);  // 0-3
    // 2
    vload(values2, array + i + 4);  // 4-7
    // 3
    vload(values3, array + i + 8);  // 8-11
    // 4
    vload(values4, array + i + 12);  // 12-15

    // compare 1 with 2 result is 1
    vmin(values1, values1, values2);
    // compare 3 with 4 result is 3
    vmin(values3, values3, values4);
    // compare 1 with 3 result is 1
    vmin(values1, values1, values3);
    //see if the new minimum is less than existing
    vec<int, 4> newMinimumMask = values1 < minValues;
    if (vany(newMinimumMask)) {
      idx = i;
      float minCandidates[4];
      vstore(minCandidates, values1);
      for (int j = 0; j < 4; ++j) {
        if (minCandidates[j] < min) {
          min = minCandidates[j];
        }
      }
      vbroadcast(minValues, min);
    }
  }
  //Do the final calculation scalar way
  for (int i = idx; i < idx + 16; ++i) {
    if (distancesIn[i] == min) {
      return i;
    }
  }
  return 0;
}

template <typename T = float, int STRIDE = 16, int VEC_WIDTH = 4>
ATH_ALWAYS_INLINE
float vecFindMinimum(const float* distancesIn, int n) {
  using namespace CxxUtils;
  const float* array =
      std::assume_aligned<GSFConstants::alignment>(distancesIn);

  constexpr int vectorCount = STRIDE / VEC_WIDTH;

  vec<T, VEC_WIDTH> minValues[vectorCount];

  for (int i = 0; i < vectorCount; i++) {
    vload(minValues[i], array + (VEC_WIDTH * i));
  }

  constexpr int totalStride = VEC_WIDTH * vectorCount;
  vec<T, VEC_WIDTH> values[vectorCount];
  for (int i = totalStride; i < n; i += totalStride) {
    GAUDI_LOOP_UNROLL(4)
    for (int j = 0; j < vectorCount; j++) {
      vload(values[j], array + i + (VEC_WIDTH * j));
      vmin(minValues[j], values[j], minValues[j]);
    }
  }

  T finalMinValues[VEC_WIDTH];
  vstore(finalMinValues, std::reduce(std::begin(minValues), 
                                     std::end(minValues), 
                                     minValues[0], 
                                     [](auto a, auto b){ return a < b ? a : b; }));

  // Do the final calculation scalar way
  return std::reduce(std::begin(finalMinValues), 
                     std::end(finalMinValues), 
                     finalMinValues[0], 
                     [](auto a, auto b){ return a < b ? a : b; });
}

template <typename T = float, int STRIDE = 16, int VEC_WIDTH = 4>
ATH_ALWAYS_INLINE
int32_t vecIdxOfValue(const T value, const T* distancesIn, int n) {
  using namespace CxxUtils;
  const float* array =
      std::assume_aligned<GSFConstants::alignment>(distancesIn);

  constexpr int vectorCount = STRIDE / VEC_WIDTH;

  vec<T, VEC_WIDTH> values[vectorCount];
  vec<T, VEC_WIDTH> target;
  vbroadcast(target, value);
  vec<int, VEC_WIDTH> eqs[vectorCount];

  for (int i = 0; i < n; i += STRIDE) {
    GAUDI_LOOP_UNROLL(4)
    for (int j = 0; j < vectorCount; j++) {
      vload(values[j], array + i + (VEC_WIDTH * j));
      eqs[j] = values[j] == target;
    }
    
    // See if we have the value in any
    // of the vectors
    // If yes then use scalar code to locate it
    if (vany(std::reduce(std::begin(eqs),
                         std::end(eqs),
                         eqs[0],
                         [](auto a, auto b){ return a || b; }))) {
      for (int idx = i; idx < i + STRIDE; ++idx) {
        if (distancesIn[idx] == value) {
          return idx;
        }
      }
    }
  }
  return -1;
}

ATH_ALWAYS_INLINE
int32_t vecMinThenIdx(const float* distancesIn, int n) {
  using namespace CxxUtils;
  const float* array =
      std::assume_aligned<GSFConstants::alignment>(distancesIn);
  // Finding of minimum needs to loop over all elements
  // But we can run the finding of index only inside a block
  constexpr int blockSizePower2 = 8;
  constexpr int blockSize = 2 << blockSizePower2;
  // case for n less than blockSize
  if (n <= blockSize) {
    float min = vecFindMinimum(array, n);
    return vecIdxOfValue(min, array, n);
  }
  int32_t idx = 0;
  float min = array[0];
  // We might have a remainder that we need to handle
  const int remainder = n & (blockSize - 1);
  for (int32_t i = 0; i < (n - remainder); i += blockSize) {
    float mintmp = vecFindMinimum(array + i, blockSize);
    if (mintmp < min) {
      min = mintmp;
      idx = i;
    }
  }
  if (remainder != 0) {
    int index = n - remainder;
    float mintmp = vecFindMinimum(array + index, remainder);
    // if the minimum is in this part
    if (mintmp < min) {
      min = mintmp;
      return index + vecIdxOfValue(min, array + index, remainder);
    }
  }
  //default return
  return idx + vecIdxOfValue(min, array + idx, blockSize);
}

}  // namespace findIdxOfMinDetail

namespace findIdxOfMinimum {
enum Impl {
  VecUpdateIdxOnNewMin = 0,
  VecAlwaysTrackIdx = 1,
  VecMinThenIdx = 2,
  C = 3,
  STL = 4
};

template <enum Impl I>
ATH_ALWAYS_INLINE int32_t impl(const float* distancesIn, int n) {
  static_assert(I == VecUpdateIdxOnNewMin || I == VecAlwaysTrackIdx ||
                    I == VecMinThenIdx || I == C || I == STL,
                "Not a valid implementation chosen");

  if constexpr (I == VecUpdateIdxOnNewMin) {
    return findIdxOfMinDetail::vecUpdateIdxOnNewMin(distancesIn, n);
  } else if constexpr (I == VecAlwaysTrackIdx) {
    return findIdxOfMinDetail::vecAlwaysTrackIdx(distancesIn, n);
  } else if constexpr (I == VecMinThenIdx) {
    return findIdxOfMinDetail::vecMinThenIdx(distancesIn, n);
  } else if constexpr (I == C) {
    return findIdxOfMinDetail::scalarC(distancesIn, n);
  } else if constexpr (I == STL) {
    return findIdxOfMinDetail::scalarSTL(distancesIn, n);
  }
  else {
    return 0; // Avoid cppchcheck warning.
  }
}
}  // namespace findIdxOfMinimum

#endif
