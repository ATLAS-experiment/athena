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


//Multiversion for the test
#if HAVE_FUNCTION_MULTIVERSIONING
[[gnu::target("default")]]
#endif
int vIdxOfMin(const float* distancesIn, int n) {
  return vAlgs::vIdxOfMin<128>(distancesIn, n);
}
#if HAVE_FUNCTION_MULTIVERSIONING
[[gnu::target("avx2")]]
int vIdxOfMin(const float* distancesIn, int n) {
  return vAlgs::vIdxOfMin<256>(distancesIn, n);
}
[[gnu::target("default")]]
#endif
int vIdxOfMin(const double* distancesIn, int n) {
  return vAlgs::vIdxOfMin<128>(distancesIn, n);
}
#if HAVE_FUNCTION_MULTIVERSIONING
[[gnu::target("avx2")]]
int vIdxOfMin(const double* distancesIn, int n) {
  return vAlgs::vIdxOfMin<256>(distancesIn, n);
}
#endif
//constants
constexpr size_t STRIDE = vAlgs::strideOfNumSIMDVec<256,float>(4);
constexpr size_t ALIGNMENT = vAlgs::alignmentForArray<256>();
constexpr size_t N = 128*STRIDE;

// create global data for the test
template <typename T>
struct InitArray {
 public:
  InitArray() : distances(N) {
    std::mt19937 gen(0);
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
  GSFUtils::AlignedDynArray<T, ALIGNMENT> distances;
};
static const InitArray<float> initArrayF;
static const InitArray<double> initArrayD;

//Test using STL
static void findIdxOfMinimumSTL() {
  const float* arrayF =
      std::assume_aligned<ALIGNMENT>(
          initArrayF.distances.buffer());
  int minIndex = std::distance(arrayF, std::min_element(arrayF, arrayF + N));
  std::cout << "STL Index of Minimum : " << minIndex << " with value "
            << initArrayF.distances[minIndex] << '\n';
  const double* arrayD =
      std::assume_aligned<ALIGNMENT>(
          initArrayD.distances.buffer());
  minIndex = std::distance(arrayD, std::min_element(arrayD, arrayD + N));
  std::cout << "STL Index of Minimum : " << minIndex << " with value "
            << initArrayD.distances[minIndex] << '\n';
}

//Test using Vec code
static void findVecMinThenIdx() {
  int minIndex = vIdxOfMin(initArrayF.distances.buffer(), N);
  std::cout << "vIdxOfMin Index of Minimum : " << minIndex << " with value "
            << initArrayF.distances[minIndex] << '\n';
  minIndex = vIdxOfMin(initArrayD.distances.buffer(), N);
  std::cout << "vIdxOfMin Index of Minimum : " << minIndex << " with value "
            << initArrayD.distances[minIndex] << '\n';

}

int main() {
  findIdxOfMinimumSTL();
  findVecMinThenIdx();
  return 0;
}
