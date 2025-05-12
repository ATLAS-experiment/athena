/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   MultiComponentStateAssembler.cxx
 * @date   Monday 20th December 2004
 * @author Atkinson,Anthony Morley, Christos Anastopoulos
 *
 * Implementation code for MultiComponentStateAssembler
 */

#include "TrkGaussianSumFilterUtils/MultiComponentStateAssembler.h"
#include "TrkParameters/ComponentParameters.h"
//
#include <limits>

namespace {

using namespace Trk::MultiComponentStateAssembler;

/** @bried Method to check the validity of of the cached state */
inline bool
isStateValid(const Cache& cache)
{
  return !cache.multiComponentState.empty();
}

/** @bried Method to assemble state with correct weightings */
Trk::MultiComponentState
doStateAssembly(Cache&& cache, const double newWeight)
{
  if (!isStateValid(cache)) {
    return {};
  }
  const size_t cacheSize = cache.multiComponentState.size();
  if (cache.validWeightSum <= 0.) {
    if (!cache.multiComponentState.empty()) {
      const double fixedWeights = 1. / static_cast<double>(cacheSize);
      for (auto& component : cache.multiComponentState) {
        component.weight = fixedWeights;
      }
    }
    Trk::MultiComponentState assembledState =
      std::move(cache.multiComponentState);
    // Reset the cache before leaving
    return assembledState;
  }

  Trk::MultiComponentState assembledState =
    std::move(cache.multiComponentState);
  const double scalingFactor =
    cache.validWeightSum > 0. ? newWeight / cache.validWeightSum : 1.;
  for (auto& component : assembledState) {
    component.weight *= scalingFactor;
  }
  // Reset the cache before leaving
  return assembledState;
}

/** @bried Method to Check component entries before full assembly */
bool
prepareStateForAssembly(Cache& cache)
{
  // Protect against empty state
  if (!isStateValid(cache)) {
    return false;
  }

  // Check for minimum fraction of valid states
  double const den = cache.validWeightSum + cache.invalidWeightSum;
  double const validWeightFraction = den > 0 ? cache.validWeightSum / den : 0;
  if (cache.invalidWeightSum > 0. &&
      validWeightFraction < Trk::MultiComponentStateAssembler::Cache::minimumValidFraction) {
    return false;
  }
  // Sort Multi-Component State by weights
  std::sort(
      cache.multiComponentState.begin(), cache.multiComponentState.end(),
      [](const Trk::ComponentParameters& x, const Trk::ComponentParameters& y) {
        return x.weight > y.weight;
      });

  double const totalWeight(cache.validWeightSum + cache.invalidWeightSum);
  if (totalWeight != 0.) {

    // ordered in descending order
    // return the 1st element where (element<value)
    const double minimumWeight =
      std::max(Trk::MultiComponentStateAssembler::Cache::minimumFractionalWeight * totalWeight,
               std::numeric_limits<double>::min());

    const Trk::ComponentParameters dummySmallestWeight = {nullptr, minimumWeight};

    auto lower_than = std::upper_bound(
        cache.multiComponentState.begin(), cache.multiComponentState.end(),
        dummySmallestWeight,
        [](const Trk::ComponentParameters& x,
           const Trk::ComponentParameters& y) { return x.weight > y.weight; });

    // reverse iterate , so as to delete removing the last
    auto lower_than_reverse = std::make_reverse_iterator(lower_than);
    for (auto itr = cache.multiComponentState.rbegin();
         itr != lower_than_reverse;
         ++itr) {
      cache.multiComponentState.erase(itr.base() - 1);
    }
  }
  // Now recheck to make sure the state is now still valid
  if (!isStateValid(cache)) {
    return false;
  }

  return true;
}

} // end anonymous namespace

Trk::MultiComponentState
Trk::MultiComponentStateAssembler::assembledState(
  MultiComponentStateAssembler::Cache&& cache)
{
  if (!prepareStateForAssembly(cache)) {
    return {};
  }
  double totalWeight = cache.validWeightSum;
  if (cache.invalidWeightSum > 0. || cache.validWeightSum <= 0.) {
    totalWeight = cache.validWeightSum + cache.invalidWeightSum;
  }

  return doStateAssembly(std::move(cache), totalWeight);
}

