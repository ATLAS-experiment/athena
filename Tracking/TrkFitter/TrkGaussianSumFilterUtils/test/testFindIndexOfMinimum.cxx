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
template <typename T>
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
  GSFUtils::AlignedDynArray<T, GSFConstants::alignment> distances;
};
static const InitArray<float> initArrayF;
static const InitArray<double> initArrayD;

static void findIdxOfMinimumSTL() {
  const float* arrayF = std::assume_aligned<GSFConstants::alignment>(
      initArrayF.distances.buffer());
  int minIndex = std::distance(arrayF, std::min_element(arrayF, arrayF + N));
  std::cout << "STL Index of Minimum : " << minIndex << " with value "
            << initArrayF.distances[minIndex] << '\n';
  const double* arrayD = std::assume_aligned<GSFConstants::alignment>(
      initArrayD.distances.buffer());
  minIndex = std::distance(arrayD, std::min_element(arrayD, arrayD + N));
  std::cout << "STL Index of Minimum : " << minIndex << " with value "
            << initArrayD.distances[minIndex] << '\n';
}

static void findVecMinThenIdx() {

  int minIndex = GSFFMVDetail::vIdxOfMin(initArrayF.distances.buffer(), N);
  std::cout << "vIdxOfMin Index of Minimum : " << minIndex << " with value "
            << initArrayF.distances[minIndex] << '\n';
  minIndex = GSFFMVDetail::vIdxOfMin(initArrayD.distances.buffer(), N);
  std::cout << "vIdxOfMin Index of Minimum : " << minIndex << " with value "
            << initArrayD.distances[minIndex] << '\n';

}

int main() {
  findIdxOfMinimumSTL();
  findVecMinThenIdx();
  return 0;
}
