/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   MultiComponentStateCombiner.cxx
 * @date   Monday 20th December 2004
 * @author Atkinson,Anthony Morley, Christos Anastopoulos
 *
 * Implementation code for MultiComponentStateCombiner
 */

#include "TrkGaussianSumFilterUtils/MultiComponentStateCombiner.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateAssembler.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateModeCalculator.h"
#include "TrkGaussianSumFilterUtils/KLGaussianMixtureReduction.h"
//
#include "CxxUtils/phihelper.h"
#include "CxxUtils/inline_hints.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/Surface.h"

namespace {

struct MeanAndCovariance {
  AmgVector(5) mean;
  AmgSymMatrix(5) covariance;
  double sumW = 0;
};

/**
 * For formulas see
 * Runnalls, Andrew R.(2007)
 * Kullback-Leibler approach to Gaussian mixture reduction
 * equations (2),(3),(4)
 * Calculate the combined mean and the  covariance matrix
 * w_{ij} = w_i + w_j (2)
 * \mu_ij = w_{i|ij} \mu_i + w_{j|ij} \mu_j (3)
 * P_{ij} = w_{i|ij} P_i + w_{j|ij} P_j + w_{i|ij} w_{j|ij} (\mu_i − \mu_j )(\mu_i − \mu_j )^T (4)
 */
MeanAndCovariance
findMeanAndCovariance(const Trk::MultiComponentState& uncombinedState) {

  MeanAndCovariance toReturn;
  toReturn.mean.setZero();
  AmgSymMatrix(5) covariancePart1;
  covariancePart1.setZero();
  AmgSymMatrix(5) covariancePart2;
  covariancePart2.setZero();
  const double phiOfFirst = uncombinedState[0].params->parameters()[2];
  const size_t numComponents = uncombinedState.size();
  for (size_t i = 0; i < numComponents; ++i) {
    const Trk::TrackParameters* trackParameters = uncombinedState[i].params.get();
    const double weight = uncombinedState[i].weight;
    AmgVector(5) parameters = trackParameters->parameters();
    // Ensure that we don't have any problems with the cyclical nature of phi
    // Use first state as reference poin
    const double deltaPhi = phiOfFirst - parameters[2];
    if (deltaPhi > M_PI) {
      parameters[2] += 2 * M_PI;
    } else if (deltaPhi < -M_PI) {
      parameters[2] -= 2 * M_PI;
    }
    toReturn.sumW += weight;
    toReturn.mean += weight * parameters;
    // Extract local error matrix: Must make sure track parameters are
    // measured, ie have an associated error matrix.
    const AmgSymMatrix(5)* measuredCov = trackParameters->covariance();
    // Calculate the combined covariance matrix
    if (measuredCov) {
      covariancePart1 += weight * (*measuredCov);
      // Loop over all remaining components to find the second part of the
      // covariance
      for (size_t j = i + 1; j < numComponents; ++j) {
        AmgVector(5) parameterDifference =
            parameters - uncombinedState[j].params->parameters();
        const double remainingWeight = uncombinedState[j].weight;
        covariancePart2 += weight * remainingWeight * parameterDifference *
                           parameterDifference.transpose();

      }  // end loop over remaining components
    }  // end clause if errors are involved
  }  // end loop over all components
  toReturn.mean /= toReturn.sumW;
  // Ensure that phi is between -pi and pi
  toReturn.mean[2] = CxxUtils::wrapToPi(toReturn.mean[2]);
  toReturn.covariance = covariancePart1 / toReturn.sumW + covariancePart2 / (toReturn.sumW * toReturn.sumW);
  return toReturn;
}

// Actual implementation method for combining
// a multi component state.
// Imoplements the mode option
Trk::ComponentParameters combineToSingleImpl(
    const Trk::MultiComponentState& uncombinedState, const bool useMode) {

  if (uncombinedState.empty()) {
    return {};
  }
  const Trk::TrackParameters* firstParameters = uncombinedState.front().params.get();

  if (uncombinedState.size() == 1) {
    return {uncombinedState.front().params->uniqueClone(),
            uncombinedState.front().weight};
  }

  MeanAndCovariance res =  findMeanAndCovariance (uncombinedState);

  const int dimension = (uncombinedState.front()).params->parameters().rows();
  if (useMode && dimension == 5) {
    // Calculate the mode of the q/p distribution
    std::array<double, 10> modes =
        Trk::MultiComponentStateModeCalculator::calculateMode(uncombinedState);
    //  Replace res.mean with mode if qOverP mode is not 0
    if (modes[4] != 0) {
      res.mean[0] = modes[0];
      res.mean[1] = modes[1];
      res.mean[2] = modes[2];
      res.mean[3] = modes[3];
      res.mean[4] = modes[4];

      if (modes[5 + 0] > 0) {
        double currentErr = sqrt((res.covariance)(0, 0));
        currentErr = modes[5 + 0] / currentErr;
        (res.covariance)(0, 0) = modes[5 + 0] * modes[5 + 0];
        res.covariance.fillSymmetric(1, 0, (res.covariance)(1, 0) * currentErr);
        res.covariance.fillSymmetric(2, 0, (res.covariance)(2, 0) * currentErr);
        res.covariance.fillSymmetric(3, 0, (res.covariance)(3, 0) * currentErr);
        res.covariance.fillSymmetric(4, 0, (res.covariance)(4, 0) * currentErr);
      }
      if (modes[5 + 1] > 0) {
        double currentErr = sqrt((res.covariance)(1, 1));
        currentErr = modes[5 + 1] / currentErr;
        res.covariance.fillSymmetric(1, 0, (res.covariance)(1, 0) * currentErr);
        (res.covariance)(1, 1) = modes[5 + 1] * modes[5 + 1];
        res.covariance.fillSymmetric(2, 1, (res.covariance)(2, 1) * currentErr);
        res.covariance.fillSymmetric(3, 1, (res.covariance)(3, 1) * currentErr);
        res.covariance.fillSymmetric(4, 1, (res.covariance)(4, 1) * currentErr);
      }
      if (modes[5 + 2] > 0) {
        double currentErr = sqrt((res.covariance)(2, 2));
        currentErr = modes[5 + 2] / currentErr;
        res.covariance.fillSymmetric(2, 0, (res.covariance)(2, 0) * currentErr);
        res.covariance.fillSymmetric(2, 1, (res.covariance)(2, 1) * currentErr);
        (res.covariance)(2, 2) = modes[5 + 2] * modes[5 + 2];
        res.covariance.fillSymmetric(3, 2, (res.covariance)(3, 2) * currentErr);
        res.covariance.fillSymmetric(4, 2, (res.covariance)(4, 2) * currentErr);
      }
      if (modes[5 + 3] > 0) {
        double currentErr = sqrt((res.covariance)(3, 3));
        currentErr = modes[5 + 3] / currentErr;
        res.covariance.fillSymmetric(3, 0, (res.covariance)(3, 0) * currentErr);
        res.covariance.fillSymmetric(3, 1, (res.covariance)(3, 1) * currentErr);
        res.covariance.fillSymmetric(3, 2, (res.covariance)(3, 2) * currentErr);
        (res.covariance)(3, 3) = modes[5 + 3] * modes[5 + 3];
        res.covariance.fillSymmetric(4, 3, (res.covariance)(4, 3) * currentErr);
      }
      if (modes[5 + 4] > 0) {
        double currentErr = sqrt((res.covariance)(4, 4));
        currentErr = modes[5 + 4] / currentErr;
        res.covariance.fillSymmetric(4, 0, (res.covariance)(4, 0) * currentErr);
        res.covariance.fillSymmetric(4, 1, (res.covariance)(4, 1) * currentErr);
        res.covariance.fillSymmetric(4, 2, (res.covariance)(4, 2) * currentErr);
        res.covariance.fillSymmetric(4, 3, (res.covariance)(4, 3) * currentErr);
        (res.covariance)(4, 4) = modes[5 + 4] * modes[5 + 4];
      }

    }  // modes[4]!=0
  }  // useMode && dimensions==5

  std::unique_ptr<Trk::TrackParameters> combinedTrackParameters = nullptr;
  double const loc1 = res.mean[Trk::loc1];
  double const loc2 = res.mean[Trk::loc2];
  double const phi = res.mean[Trk::phi];
  double const theta = res.mean[Trk::theta];
  double const qoverp = res.mean[Trk::qOverP];
  const AmgSymMatrix(5)* firstMeasuredCov = firstParameters->covariance();
  if (firstMeasuredCov) {
    combinedTrackParameters =
        firstParameters->associatedSurface().createUniqueTrackParameters(
            loc1, loc2, phi, theta, qoverp, std::move(res.covariance));
  } else {
    combinedTrackParameters =
        firstParameters->associatedSurface().createUniqueTrackParameters(
            loc1, loc2, phi, theta, qoverp, std::nullopt);
  }

  return {std::move(combinedTrackParameters), res.sumW};
}
/*
 * Additional utilities. Mainly used for the forward / smoother
 * combination
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
    const int8_t mini = merges[i].To;
    const int8_t minj = merges[i].From;
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
    cache.multiComponentState.push_back(
        {std::move(state.params), state.weight});
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
  // Assembler Cache
  Trk::MultiComponentStateAssembler::Cache cache;
  if (statesToMerge.size() <= maximumNumberOfComponents) {
    Trk::MultiComponentStateAssembler::addMultiState(cache,
                                                     std::move(statesToMerge));
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

    Trk::ComponentParameters dummyCompParams = {
        std::move(statesToMerge.begin()->params), 1.};
    Trk::MultiComponentState returnMultiState;
    returnMultiState.push_back(std::move(dummyCompParams));
    return returnMultiState;
  }

  return mergeFullDistArray(cache, std::move(statesToMerge),
                            maximumNumberOfComponents);
}
}  // end anonymous namespace

std::unique_ptr<Trk::TrackParameters>
Trk::MultiComponentStateCombiner::combineToSingle(
const Trk::MultiComponentState& uncombinedState, const bool useMode) {
  Trk::ComponentParameters combinedComponent = combineToSingleImpl(uncombinedState, useMode);
  return std::move(combinedComponent.params);
}


// The following does heave use of Eigen
// for covariance. Avoid out-of-line calls.
ATH_FLATTEN
void
Trk::MultiComponentStateCombiner::combineParametersWithWeight(
  AmgVector(5) & firstParameters,
  double& firstWeight,
  const AmgVector(5) & secondParameters,
  const double secondWeight)
{
  double const totalWeight = firstWeight + secondWeight;
  double const invTotalWeight = 1.0/totalWeight;
  double const deltaPhi = firstParameters[2] - secondParameters[2];
  if (deltaPhi > M_PI) {
    firstParameters[2] -= 2 * M_PI;
  } else if (deltaPhi < -M_PI) {
    firstParameters[2] += 2 * M_PI;
  }
  firstParameters =
      (firstWeight * firstParameters + secondWeight * secondParameters) *
      invTotalWeight;
  // Ensure that phi is between -pi and pi
  firstParameters[2] = CxxUtils::wrapToPi(firstParameters[2]);
  firstWeight = totalWeight;
}

// The following does heave use of Eigen
// for covariance. Avoid out-of-line calls.
ATH_FLATTEN
void
Trk::MultiComponentStateCombiner::combineCovWithWeight(
  const AmgVector(5) & firstParameters,
  AmgSymMatrix(5) & firstMeasuredCov,
  const double firstWeight,
  const AmgVector(5) & secondParameters,
  const AmgSymMatrix(5) & secondMeasuredCov,
  const double secondWeight)
{
  double const invTotalWeight = 1.0/(firstWeight + secondWeight);
  AmgVector(5) parameterDifference = firstParameters - secondParameters;
  parameterDifference[2] = CxxUtils::wrapToPi(parameterDifference[2]);
  parameterDifference *= invTotalWeight;
  firstMeasuredCov = (firstWeight * firstMeasuredCov + secondWeight * secondMeasuredCov) * invTotalWeight;
  firstMeasuredCov += firstWeight * secondWeight * parameterDifference * parameterDifference.transpose();
}

// The following does heave use of Eigen
// for covariance. Avoid out-of-line calls
// to Eigen
ATH_FLATTEN
Trk::MultiComponentState
Trk::MultiComponentStateCombiner::combineWithSmoother(
  const Trk::MultiComponentState& forwardsMultiState,
  const Trk::MultiComponentState& smootherMultiState,
  unsigned int maximumNumberOfComponents)
{

  std::unique_ptr<Trk::MultiComponentState> combinedMultiState =
      std::make_unique<Trk::MultiComponentState>();

  // Loop over all components in forwards multi-state
  for (const auto& forwardsComponent : forwardsMultiState) {
    // Need to check that all components have associated weight matricies
    const AmgSymMatrix(5)* forwardMeasuredCov =
        forwardsComponent.params->covariance();
    // Loop over all components in the smoother multi-state
    for (const auto& smootherComponent : smootherMultiState) {
      // Need to check that all components have associated weight matricies
      const AmgSymMatrix(5)* smootherMeasuredCov =
          smootherComponent.params->covariance();
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

      const AmgSymMatrix(5) summedCovariance =
          *forwardMeasuredCov + *smootherMeasuredCov;
      const AmgSymMatrix(5) K =
          *forwardMeasuredCov * summedCovariance.inverse();
      const AmgVector(5) newParameters =
          forwardsComponent.params->parameters() +
          K * (smootherComponent.params->parameters() -
               forwardsComponent.params->parameters());
      const AmgVector(5) parametersDiff =
          forwardsComponent.params->parameters() -
          smootherComponent.params->parameters();

      AmgSymMatrix(5) covarianceOfNewParameters =
          AmgSymMatrix(5)(K * *smootherMeasuredCov);

      std::unique_ptr<Trk::TrackParameters> combinedTrackParameters =
          (forwardsComponent.params)
              ->associatedSurface()
              .createUniqueTrackParameters(
                  newParameters[Trk::loc1], newParameters[Trk::loc2],
                  newParameters[Trk::phi], newParameters[Trk::theta],
                  newParameters[Trk::qOverP],
                  std::move(covarianceOfNewParameters));
      const AmgSymMatrix(5) invertedSummedCovariance = summedCovariance.inverse();
      // Determine the scaling factor for the new weighting. Determined from the
      // PDF of the many-dimensional gaussian
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
  }
  // Component reduction on the combined state
  Trk::MultiComponentState mergedState =
      merge(std::move(*combinedMultiState), maximumNumberOfComponents);
  // Before return the weights of the states need to be renormalised to one.
  Trk::MultiComponentStateHelpers::renormaliseState(mergedState);

  return mergedState;
}
