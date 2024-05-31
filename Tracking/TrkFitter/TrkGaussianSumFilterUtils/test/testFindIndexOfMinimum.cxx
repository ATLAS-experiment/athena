/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkGaussianSumFilterUtils/GSFFindIndexOfMinimum.h"
//
#include "TrkGaussianSumFilterUtils/AlignedDynArray.h"
//
#include <algorithm>
#include <iostream>
#include <random>

// create global data for the test
constexpr size_t N = 4096;
struct InitArray {
 public:
  InitArray() : distances(N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.001, 5.0);
    for (size_t i = 0; i < N; ++i) {
      distances[i] = dis(gen);
      // At the end of the "vectorized loops"
      //  36 will the 1st element of the second SIMD vec
      //  1024 will the 1st element of the first SIMD vec
      //  17 will the 2nd eleament of the fourth SIMD vec
      //  40 will the 1st eleament of the third SIMD vec
      if (i == 17 || i == 40 || i == 36 || i == 1024) {
        distances[i] = 0.0;
      }
    }
  }
  GSFUtils::AlignedDynArray<float, GSFConstants::alignment> distances;
};
static const InitArray initArray;

static void findIdxOfMinimumC() {
  int32_t minIndex = findIdxOfMinimum::impl<findIdxOfMinimum::C>(
      initArray.distances.buffer(), N);
  std::cout << "C Index of Minimum : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}

static void findIdxOfMinimumSTL() {
  int32_t minIndex = findIdxOfMinimum::impl<findIdxOfMinimum::STL>(
      initArray.distances.buffer(), N);
  std::cout << "STL Index of Minimum : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}

static void findVecAlwaysTrackIdx() {
  int32_t minIndex =
      findIdxOfMinimum::impl<findIdxOfMinimum::VecAlwaysTrackIdx>(
          initArray.distances.buffer(), N);
  std::cout << "VecAlwaysTrackIdx Index of Minimum : " << minIndex
            << " with value " << initArray.distances[minIndex] << '\n';
}

static void findVecUpdateIdxOnNewMin() {
  int32_t minIndex =
      findIdxOfMinimum::impl<findIdxOfMinimum::VecUpdateIdxOnNewMin>(
          initArray.distances.buffer(), N);
  std::cout << "VecUpdateIdxOnNewMin Index of Minimum : " << minIndex
            << " with value " << initArray.distances[minIndex] << '\n';
}

static void findVecMinThenIdx() {
  int32_t minIndex = findIdxOfMinimum::impl<findIdxOfMinimum::VecMinThenIdx>(
      initArray.distances.buffer(), N);
  std::cout << "VecMinThenIdx Index of Minimum : " << minIndex << " with value "
            << initArray.distances[minIndex] << '\n';
}
int main() {
  findIdxOfMinimumC();
  findIdxOfMinimumSTL();
  findVecAlwaysTrackIdx();
  findVecUpdateIdxOnNewMin();
  findVecMinThenIdx();
  return 0;
}
