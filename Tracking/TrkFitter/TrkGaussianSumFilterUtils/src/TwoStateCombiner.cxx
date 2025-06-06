/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   TwoStateCombiner.cxx
 * @author Atkinson,Anthony Morley, Christos Anastopoulos
 *
 * Implementation code for TwoStateCombiner
 */

#include "TrkGaussianSumFilterUtils/TwoStateCombiner.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateCombiner.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateAssembler.h"
#include "TrkGaussianSumFilterUtils/KLGaussianMixtureReduction.h"
//
#include "CxxUtils/inline_hints.h"

namespace {
 /*
 * Utilities used for the forward / backward
 * smoother combination.
 */
void
combineWithWeight(Trk::ComponentParameters& mergeTo,
const Trk::ComponentParameters& addThis)
{
  const Trk::TrackParameters* firstTrackParameters = mergeTo.params.get();
  const AmgVector(5)& firstParameters = firstTrackParameters->parameters();
  double const firstWeight = mergeTo.weight;

  const Trk::TrackParameters* secondTrackParameters = addThis.params.get();
  const AmgVector(5)& secondParameters = secondTrackParameters->parameters();
  double const secondWeight = addThis.weight;

  // copy over the first
  AmgVector(5) finalParameters(firstParameters);
  double finalWeight = firstWeight;
  Trk::MultiComponentStateCombiner::combineParametersWithWeight(
    finalParameters, finalWeight, secondParameters, secondWeight);

  const AmgSymMatrix(5)* firstMeasuredCov = firstTrackParameters->covariance();
  const AmgSymMatrix(5)* secondMeasuredCov = secondTrackParameters->covariance();
  // Check to see if first track parameters are measured or not
  if (firstMeasuredCov && secondMeasuredCov) {
    AmgSymMatrix(5) finalMeasuredCov(*firstMeasuredCov);
    Trk::MultiComponentStateCombiner::combineCovWithWeight(
        firstParameters, finalMeasuredCov, firstWeight, secondParameters,
        *secondMeasuredCov, secondWeight);
    mergeTo.params->updateParameters(finalParameters, finalMeasuredCov);
    mergeTo.weight = finalWeight;
  } else {
    mergeTo.params->updateParameters(finalParameters);
    mergeTo.weight = finalWeight;
  }
}

/// Method for merging components  and assembling a final state
Trk::MultiComponentState mergeFullDistArray(
    Trk::MultiComponentStateAssembler::Cache& cache,
    Trk::MultiComponentState&& statesToMerge,
    const unsigned int maximumNumberOfComponents) {

  const int n = statesToMerge.size();
  GSFUtils::Component1DArray componentsArray(n);
  for (int i = 0; i < n; ++i) {
    const AmgSymMatrix(5)* measuredCov = statesToMerge[i].params->covariance();
    const AmgVector(5)& parameters = statesToMerge[i].params->parameters();
    // Fill in infomation
    const double cov = measuredCov ? (*measuredCov)(Trk::qOverP, Trk::qOverP) : -1.;
    componentsArray[i].mean = parameters[Trk::qOverP];
    componentsArray[i].cov = cov;
    componentsArray[i].invCov = cov > 0 ? 1. / cov : 1e10;
    componentsArray[i].weight = statesToMerge[i].weight;
  }

  // Gather the merges
  const GSFUtils::MergeArray merges = findMerges(std::move(componentsArray), maximumNumberOfComponents);

  // Do the full 5D calculations of the merge
  const int numMerges = merges.size();
  for (int i = 0; i < numMerges; ++i) {
    const int mini = merges[i].To;
    const int minj = merges[i].From;
    combineWithWeight(statesToMerge[mini], statesToMerge[minj]);
    statesToMerge[minj].params.reset();
    statesToMerge[minj].weight = 0.;
  }
  // Assemble the final result
  for (auto& state : statesToMerge) {
    // Avoid merge ones
    if (!state.params) {
      continue;
    }
    cache.multiComponentState.push_back({std::move(state.params), state.weight});
    cache.validWeightSum += state.weight;
  }
  Trk::MultiComponentState mergedState =
      Trk::MultiComponentStateAssembler::assembledState(std::move(cache));
  // Clear the state vector
  return mergedState;
}

////
Trk::MultiComponentState
merge(Trk::MultiComponentState&& statesToMerge,
      const unsigned int maximumNumberOfComponents) {

  Trk::MultiComponentStateAssembler::Cache cache;
  //In case there is nothing to merge
  if (statesToMerge.size() <= maximumNumberOfComponents) {
    Trk::MultiComponentStateAssembler::addMultiState(cache, std::move(statesToMerge));
    return Trk::MultiComponentStateAssembler::assembledState(std::move(cache));
  }

  // Scan all components for covariance matrices. If one or more component
  // is missing an error matrix, component reduction is impossible.
  bool componentWithoutMeasurement = false;
  Trk::MultiComponentState::const_iterator component = statesToMerge.cbegin();
  for (; component != statesToMerge.cend(); ++component) {
    if (!component->params->covariance()) {
      componentWithoutMeasurement = true;
      break;
    }
  }
  if (componentWithoutMeasurement) {
    // Sort to select the one with the largest weight
    std::sort(
        statesToMerge.begin(), statesToMerge.end(),
        [](const Trk::ComponentParameters& x,
           const Trk::ComponentParameters& y) { return x.weight > y.weight; });
    //And return it
    Trk::ComponentParameters dummyCompParams = {std::move(statesToMerge.begin()->params), 1.};
    Trk::MultiComponentState returnMultiState;
    returnMultiState.push_back(std::move(dummyCompParams));
    return returnMultiState;
  }
  //Do the full merging of states
  return mergeFullDistArray(cache, std::move(statesToMerge),maximumNumberOfComponents);
}
}  // end anonymous namespace


// The following does heave use of Eigen
// for covariance. Avoid out-of-line calls
// to Eigen
ATH_FLATTEN
Trk::MultiComponentState
Trk::TwoStateCombiner::combine(
  const Trk::MultiComponentState& forwardsMultiState,
  const Trk::MultiComponentState& smootherMultiState,
  unsigned int maximumNumberOfComponents)
{

  auto combinedMultiState = std::make_unique<Trk::MultiComponentState>();

  // Loop over all components in forwards multi-state
  for (const auto& forwardsComponent : forwardsMultiState) {
    const AmgSymMatrix(5)* forwardMeasuredCov = forwardsComponent.params->covariance();

    // Loop over all components in the smoother multi-state
    for (const auto& smootherComponent : smootherMultiState) {
      const AmgSymMatrix(5)* smootherMeasuredCov = smootherComponent.params->covariance();

      // If not covariances return here.
      if (!smootherMeasuredCov && !forwardMeasuredCov) {
        return {};
      }

      if (!forwardMeasuredCov) {
        Trk::ComponentParameters smootherComponentOnly = {
            smootherComponent.params->uniqueClone(), smootherComponent.weight};
        combinedMultiState->push_back(std::move(smootherComponentOnly));
        continue;
      }

      if (!smootherMeasuredCov) {
        Trk::ComponentParameters forwardComponentOnly = {
            forwardsComponent.params->uniqueClone(), forwardsComponent.weight};
        combinedMultiState->push_back(std::move(forwardComponentOnly));
        continue;
      }

      const AmgVector(5) smootherParams = smootherComponent.params->parameters();
      const AmgVector(5) forwardParams = forwardsComponent.params->parameters();
      const AmgSymMatrix(5) summedCovariance = *forwardMeasuredCov + *smootherMeasuredCov;
      const AmgSymMatrix(5) invertedSummedCovariance = summedCovariance.inverse();
      const AmgSymMatrix(5) K = *forwardMeasuredCov * invertedSummedCovariance;
      const AmgVector(5) newParameters = forwardParams + K * (smootherParams - forwardParams);
      AmgSymMatrix(5) covarianceOfNewParameters = AmgSymMatrix(5)(K * *smootherMeasuredCov);

      std::unique_ptr<Trk::TrackParameters> combinedTrackParameters =
          (forwardsComponent.params)
              ->associatedSurface()
              .createUniqueTrackParameters(
                  newParameters[Trk::loc1],
                  newParameters[Trk::loc2],
                  newParameters[Trk::phi],
                  newParameters[Trk::theta],
                  newParameters[Trk::qOverP],
                  std::move(covarianceOfNewParameters));

      // Determine the scaling factor for the new weighting. Determined from the
      // PDF of the many-dimensional gaussian
      const AmgVector(5) parametersDiff = forwardParams - smootherParams;
      double const exponent = parametersDiff.transpose() *
                              invertedSummedCovariance * parametersDiff;
      double const weightScalingFactor = exp(-0.5 * exponent);
      double const combinedWeight = smootherComponent.weight *
                                    forwardsComponent.weight *
                                    weightScalingFactor;
      Trk::ComponentParameters combinedComponent = {
          std::move(combinedTrackParameters), combinedWeight};
      combinedMultiState->push_back(std::move(combinedComponent));
    }
  } //end of double loop

  // Component reduction on the combined state
  Trk::MultiComponentState mergedState =
      merge(std::move(*combinedMultiState), maximumNumberOfComponents);
  // Before return the weights of the states need to be renormalised to one.
  Trk::MultiComponentStateHelpers::renormaliseState(mergedState);

  return mergedState;
}
