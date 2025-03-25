/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkGaussianSumFilterUtils/KLGaussianMixtureReduction.h"
#include "TrkGaussianSumFilterUtils/AlignedDynArray.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
//
#include "TrkGaussianSumFilterUtils/GSFFindIndexOfMinimum.h"
//
#include "CxxUtils/restrict.h"
#include "CxxUtils/vec.h"
//
#include <cmath>
#include <memory>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

/**
 * @file  KLGaussianMixtureReduction.cxx
 * @author Anthony Morley , Christos Anastopoulos
 * @date 26th November 2019
 *
 * Implementation of KLGaussianMixtureReduction
 */
namespace KLReductionFMV {
//clang FMV needs a namespace :/
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
#endif
}  // namespace KLReductionFMV

namespace {
//internal implementation methods

//We want to be using up to a 256 ISA, these cover also a narrower one
constexpr size_t STRIDEForKL = vAlgs::strideOfNumSIMDVec<256,float>(4);
constexpr size_t ALIGNMENTForKL = vAlgs::alignmentForArray<256>();
using namespace GSFUtils;

/**
 * Based on
 * https://www.sciencedirect.com/science/article/pii/089812218990103X
 * equation (16)
 * or
 * https://arxiv.org/pdf/2001.00727.pdf
 * equation (10)
 * but not accounting for weights
 * covI * invCovJ + covJ * invCovI + (mean1-mean2) (invcov+invcov) (mean1-mean2)
 * We use doubles for the intermediate calculations
 * but we store the final distance in a float
 */
inline float
symmetricKL(const Component1D& ATH_RESTRICT componentI,
            const Component1D& ATH_RESTRICT componentJ)
{
  const double meanDifference = componentI.mean - componentJ.mean;
  const double inverCovSum = componentI.invCov + componentJ.invCov;
  const double term1 = componentI.invCov * componentJ.cov;
  const double term2 = componentJ.invCov * componentI.cov;
  const double term3 = meanDifference * inverCovSum * meanDifference;
  return term1 + term2 + term3;
}
/**
 * Moment-preserving merge of two 1D components
 * for example see
 * Runnalls, Andrew R.(2007)
 * Kullback-Leibler approach to Gaussian mixture reduction
 * equations (2),(3),(4)
 */
inline void
combine(GSFUtils::Component1D& ATH_RESTRICT updated,
        GSFUtils::Component1D& ATH_RESTRICT removed)
{

  const double sumWeight = updated.weight + removed.weight;

  const double invSumWeight = 1. / sumWeight;
  const double weightI_IJ = updated.weight * invSumWeight;
  const double weightJ_IJ = removed.weight * invSumWeight;
  const double meanDiff = (updated.mean - removed.mean);

  const double sumMean = weightI_IJ * updated.mean + weightJ_IJ * removed.mean;

  const double sumVariance = weightI_IJ * updated.cov +
                             weightJ_IJ * removed.cov +
                             weightI_IJ * weightJ_IJ * meanDiff * meanDiff;
  updated.mean = sumMean;
  updated.cov = sumVariance;
  updated.invCov = 1. / sumVariance;
  updated.weight = sumWeight;
}

inline int
numDistances(const int n, float* distancesIn)
{
  const int npadded = vAlgs::numPadded<STRIDEForKL>(n);
  // Make sure the padded elements are set to max
  std::fill(
    distancesIn + n, distancesIn + npadded, std::numeric_limits<float>::max());
  return npadded;
}

/*
 * This is a O(N^3) time N^2 space algorithm.
 * It can be seen as tailored implementation for
 * our problem of the basic algorithm
 * for Hierarchical Clustering.
 *
 * We rely on the fast findMinimunIndex above
 * and a triangular array representation to
 * reduce the actual time for our problem
 * (max N=72)
 *
 * We opt for fixed size arrays of max N(=72) elements,
 * but dynamic arrays of ~ N*(N-1)/2 elements.
 *
 * Existing alternatives in the literature:
 * - 1. O(N^3) worst-case time , O(n^2) best
 * - 2. O(N^2 log(N)) worst-time
 * See :
 * "Modern hierarchical, agglomerative clustering algorithms"
 *  https://arxiv.org/abs/1109.2378
 *  or
 * "Efficient algorithms for agglomerative hierarchical clustering methods"
 *
 * What we found in the past:
 * - We seem to hit the O(N^3)/worst case of Alg 1
 * quite often.
 * - Alg 2 had significant overhead
 * from the required data-structures.
 * We could revisit these in the future.
 */

/*
 * Pairwise distances implementation:
 * Assuming N total elements 0... N-1,
 * the pairwise distance matrix
 * can be represented in a triangular array: <br>
 * [ (1,0) ] <br>
 * [ (2,0), (2,1) ] <br>
 * [ (3,0), (3,1), (3,2)] <br>
 * [ (4,0), (4,1), (4,2) , (4,3) <br>
 * [.............................] <br>
 * [(N-1,0),(N-1,1),(N-1,2),(N-1,3) ... (N-1,N-2)]<br>
 * With size 1+2+3+ .... (N-1) = N*(N-1)/2
 *
 * The lexicographical storage allocation function is
 * Loc(i,j) = i*(i-1)/2 + j <br>
 * e.g : <br>
 * (1,0) => 1 *(1-1)/2 + 0 => 0 <br>
 * (2,0) => 2 *(2-1)/2 + 0 => 1 <br>
 * (2,1) => 2 *(2-1)/2 + 1 => 2 <br>
 * (3,0) => 3 * (3-1)/2 +0 => 3 <br>
 * Leading to <br>
 * [(1,0),(2,0),(2,1),(3,0),(3,1),(3,2).... (N-1,N-2)]
 *
 * The N-1 Rows  map to the value K of the 1st element in the pair
 * 1,2,3,..,N-1. <br>
 * Each Row has size K  and starts at array positions K*(K-1)/2 <br>
 * e.g <br>
 * The row for element 1 starts at array position 0. <br>
 * The row for element 2 starts at array position 1. <br>
 * The row for element N-1  starts at array positon (N-1)*(N-2)/2 <br>
 *
 * The N-1 Columns map to the value K of the second  element in the pair <br>
 * K= 0,1,2 .., N-2 <br>
 * The array positions follows (i-1)*i/2+K <br>
 * where i : K+1 .... N-1 [for(i=K+1;i<N;++i) <br>
 * e.g <br>
 * 0 appears as 2nd element in the pair at array positions [0,1,3,6...] <br>
 * 1 appears as 2nd element in the pair at array positions [2,4,7...] <br>
 * 2 appears as 2nd element in the pair at array positions [5,8,12....] <br>
 * N-2 appears as 2nd element once at position [N(N-1)/2-1] <br>
 */

/**
 * @brief precalculate the offsets for the column
 */
constexpr std::array<int, GSFConstants::maxComponentsAfterConvolution>
    offset = []() {
      constexpr int n = GSFConstants::maxComponentsAfterConvolution;
      std::array<int, n> tmp = {};
      for (int i = 0; i < n; ++i) {
        tmp[i] = (i - 1) * i / 2;
      }
      return tmp;
    }();

/**
 * @brief Helper struct to map position in
 * triangular array to matrix(I, J) indices
 */
struct triangularToIJ
{
  int8_t I = -1;
  int8_t J = -1;
};
/**
 * @brief Helper method to convert
 * from index in triangular array
 * to matrix(I,J)
 */
inline triangularToIJ
convert(int idx)
{
  if (idx<0){
    throw std::out_of_range("KLGaussianMixtureReduction.cxx::convert : idx is negative");
  }
  // We prefer to preMap the maximum 2556 elements.
  // Alternatively one can use the following
  // if pre-mapping becomes an issue
  // (see https://hal.archives-ouvertes.fr/hal-02047514/document)
  //  int8_t i = std::floor((std::sqrt(1 + 8 * idx) + 1) / 2);
  //  int8_t j = idx - (i - 1) * i / 2;
  static const std::vector<triangularToIJ> preMap = []() {
    constexpr int n = GSFConstants::maxComponentsAfterConvolution;
    constexpr size_t nn = n * (n - 1) / 2;
    std::vector<triangularToIJ> indexMap(nn);
    for (int8_t i = 1; i < n; ++i) {
      const int indexConst = offset[i];
      for (int8_t j = 0; j < i; ++j) {
        indexMap[indexConst + j] = { i, j };
      }
    }
    return indexMap;
  }();
  return preMap[idx];
}

/**
 * @brief Calculate the distances for all component pairs
 * @c componentsIn is the array of componets
 * @distancesIn is the array of distances to fill
 * @n is the number of components
 */
inline void
calculateAllDistances(const Component1D* componentsIn,
                      float* distancesIn,
                      const int n)
{
  const Component1D* components =
    std::assume_aligned<GSFConstants::alignment>(componentsIn);
  float* distances =
    std::assume_aligned<GSFConstants::alignment>(distancesIn);
  for (int i = 1; i < n; ++i) {
    const int indexConst = offset[i];
    const Component1D componentI = components[i];
    for (int j = 0; j < i; ++j) {
      const Component1D componentJ = components[j];
      distances[indexConst + j] = symmetricKL(componentI, componentJ);
    }
  }
}

/**
 * @brief update the distances wrt to the merged
 * components
 * @c componentsIn are the input components
 * @c distancesIn is the array of distances
 * @c minFrom is the index of the element we merged from (remove)
 * @c minTo is the index of the element we merged to (keep)
 * @c n is the components before the removal
 */
inline int
updateDistances(
  Component1D* ATH_RESTRICT componentsIn,
  std::array<int8_t, GSFConstants::maxComponentsAfterConvolution>& mergingIndex,
  float* ATH_RESTRICT distancesIn,
  int minFrom,
  int minTo,
  int n)
{
  float* distances =
    std::assume_aligned<GSFConstants::alignment>(distancesIn);
  Component1D* components =
    std::assume_aligned<GSFConstants::alignment>(componentsIn);
  // We swap the last elements with the ones indexed by minFrom.
  // After this the remaining components we care about
  // are n-1 which we return
  const int last = (n - 1);
  const int indexOffsetJ = offset[minFrom];
  const int indexOffsetLast = offset[last];
  // we do no need to swap the last with itself
  if (minFrom != last) {
    // Rows in distance matrix
    for (int i = 0; i < minFrom; ++i) {
      std::swap(distances[indexOffsetJ + i], distances[indexOffsetLast + i]);
    }
    // Columns in distance matrix
    for (int i = minFrom + 1; i < last; ++i) {
      const int index = offset[i] + minFrom;
      std::swap(distances[index], distances[indexOffsetLast + i]);
    }
    // swap the components
    std::swap(components[minFrom], components[last]);
    std::swap(mergingIndex[minFrom], mergingIndex[last]);
  }
  // In case minTo was indexing the last now it should
  // be indexing minFrom due to the above swapping
  if (minTo == last) {
    minTo = minFrom;
  }
  const int indexConst = offset[minTo];
  // This is the component that has been updated
  const Component1D componentJ = components[minTo];
  // Rows in distance matrix
  for (int i = 0; i < minTo; ++i) {
    const Component1D componentI = components[i];
    const int index = indexConst + i;
    distances[index] = symmetricKL(componentI, componentJ);
  }
  // Columns in distance matrix
  for (int i = minTo + 1; i < last; ++i) {
    const Component1D componentI = components[i];
    const int index = offset[i] + minTo;
    distances[index] = symmetricKL(componentI, componentJ);
  }
  return last;
}

/**
 * Return which components need to be merged.
 */
MergeArray
findMergesImpl(const Component1DArray& componentsIn,
               const int n,
               const int8_t reducedSize)
{
  // copy the array for internal use
  Component1DArray copyComponents(componentsIn);
  Component1D* components = std::assume_aligned<GSFConstants::alignment>(
    copyComponents.components.data());
  // Based on the inputSize n allocate enough space for the pairwise distances
  int nn = n * (n - 1) / 2;
  int nnpadded = vAlgs::numPadded<STRIDEForKL>(nn);
  AlignedDynArray<float, ALIGNMENTForKL> distances(
    nnpadded, std::numeric_limits<float>::max());
  // initial distance calculation
  calculateAllDistances(components, distances.buffer(), n);
  // As we merge keep track where things moved
  std::array<int8_t, GSFConstants::maxComponentsAfterConvolution>
    mergingIndex{};
  std::iota(mergingIndex.begin(), mergingIndex.end(), 0);
  // Result to be returned
  MergeArray result{};
  int numberOfComponentsLeft = n;
  // merge loop
  while (numberOfComponentsLeft > reducedSize) {
    // find pair with minimum distance
    const int minIndex = KLReductionFMV::vIdxOfMin(distances.buffer(), nnpadded);
    const triangularToIJ conversion = convert(minIndex);
    int8_t minTo = conversion.I;
    int8_t minFrom = conversion.J;
    // This is the convention we had so retained.
    if (mergingIndex[minTo] < mergingIndex[minFrom]) {
      std::swap(minTo, minFrom);
    }
    // prepare what to return
    const int8_t miniToreturn = mergingIndex[minTo];
    const int8_t minjToreturn = mergingIndex[minFrom];
    result.merges[result.numMerges] = { miniToreturn, minjToreturn };
    ++result.numMerges;
    // Combine
    combine(components[minTo], components[minFrom]);
    // update distances
    numberOfComponentsLeft = updateDistances(components,
                                             mergingIndex,
                                             distances.buffer(),
                                             minFrom,
                                             minTo,
                                             numberOfComponentsLeft);

    // number of remaining distances padded
    nn = offset[numberOfComponentsLeft];
    nnpadded = numDistances(nn, distances.buffer());
  } // end of merge while
  return result;
}

} // anonymous namespace with implementation

namespace GSFUtils {
MergeArray
findMerges(const Component1DArray& componentsIn, const int8_t reducedSize)
{
  const int n = componentsIn.numComponents;
  if (n < 0 || n > GSFConstants::maxComponentsAfterConvolution ||
      reducedSize > n) {
    throw std::runtime_error("findMerges :Invalid InputSize or reducedSize");
  }
  return findMergesImpl(componentsIn, n, reducedSize);
}
} // end namespace GSFUtils
