/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  KLGaussianMixtureReduction.h
 * @author Anthony Morley , Christos Anastopoulos
 * @date 26th November 2019
 *
 *
 * @brief Utilities to facilitate the calculation of the
 * KL divergence/distance between components of the mixture
 * and the merging of similar componets together.
 */

#ifndef KLGaussianMixReductionUtils_H
#define KLGaussianMixReductionUtils_H

#include "TrkGaussianSumFilterUtils/AlignedDynArray.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
//
#include <vector>
#include <cstdint>

namespace GSFUtils {

/**
 * @brief struct representing 1D component
 */
struct Component1D
{
  double mean = 0.;
  double cov = 0.;
  double invCov = 1e10;
  double weight = 0.;
};
using Component1DArray = AlignedDynArray<Component1D, GSFConstants::alignment>;
/**
 * @brief struct representing an array or the merges.
 * The merge is from the element in positon 'From'
 * to the element in position 'To'
 */
struct Merge {
  int To = 0;
  int From = 0;
};
using MergeArray = std::vector<Merge>;

/**
 * @brief Find the order in which the components need to
 * be merged. Returns an MergeArray  with the merges
 * (To,From).
 * The index of the merged From is always smaller than
 * the To (RHS is smaller than LHS)
 *
 * @c Component1DArray : Array of simplified 1D components
 * used to calculate the merge order using q/p.
 *
 * @c reducedSize  The size we want to reduce the mixture to.
 * Needs to be smaller than the numComponents of the componentsIn
 * array
 */
MergeArray
findMerges(Component1DArray&& componentsIn, const int reducedSize);

} // namespace KLGaussianMixtureReduction

#endif
