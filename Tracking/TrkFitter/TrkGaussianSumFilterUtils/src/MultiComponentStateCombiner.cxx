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
#include "TrkGaussianSumFilterUtils/MultiComponentStateModeCalculator.h"
//
#include "CxxUtils/phihelper.h"
#include "CxxUtils/inline_hints.h"
#include "TrkParameters/TrackParameters.h"

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

