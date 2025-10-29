/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file   GsfMeasurementUpdator.cxx
 * @date   Friday 25th February 2005
 * @author Tom Athkinson, Anthony Morley, Christos Anastopoulos
 * @brief  Implementation code for Gsf Measurement Update
 */

// Suppress a bogus warning.
#if __GNUC__ >= 14 && defined(__aarch64__)
# pragma GCC diagnostic ignored "-Warray-bounds"
#endif

#include "EventPrimitives/EventPrimitivesCovarianceHelpers.h"
//
#include "TrkGaussianSumFilterUtils/GsfMeasurementUpdator.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateAssembler.h"
//
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkMeasurementBase/MeasurementBase.h"

#include "CxxUtils/inline_hints.h"

#include <memory>

namespace {

// constants
const Trk::ProjectionMatricesSet s_reMatrices(5);
constexpr double s_thetaGainDampingValue = 0.1;
const AmgVector(5) s_cov0Vec {250., 250., 0.25, 0.25, 0.000001};
const AmgMatrix(5, 5) s_unitMatrix(AmgMatrix(5, 5)::Identity());
const AmgVector(2) s_thetaMin(0.0, -M_PI);
enum RangeCheckDef
{
  absoluteCheck = 0,
  differentialCheck = 1
};

struct componentsCache
{
  struct element
  {
    double determinantR;
    double chi2;
  };
  std::array<element, GSFConstants::maxNumberofStateComponents> elements{};
  size_t numElements = 0;
};

/** Get the Measurememnt Co-ordinate for the 1D case */
int measurementCoord_1D(int paramKey) {
  for (int i = 0; i < 5; ++i) {
    if (paramKey & (1 << i)) {
      return i;
    }
  }

  return 0;
}

/** Absolute phi values should be in [-pi, pi]
    absolute theta values should be in [0, +pi]
    phi differences should also be in [-pi, pi] - else go other way round,
    theta differences should be smaller than pi but can be negative
    => other constraint than absolute theta.
*/
inline bool
thetaPhiWithinRange_5D(const AmgVector(5) & V, const RangeCheckDef rcd)
{
  return ((std::abs(V(Trk::phi)) <= M_PI) &&
          (V(Trk::theta) >= s_thetaMin(rcd)) && (V(Trk::theta) <= M_PI));
}

/**
 * Test if theta angle is inside boundaries. No differential-check option.
 */
inline bool
thetaWithinRange_5D(const AmgVector(5) & V)
{
  return (V(Trk::theta) >= 0.0 && (V(Trk::theta) <= M_PI));
}

bool
correctThetaPhiRange_5D(AmgVector(5) & V,
                        AmgSymMatrix(5) & M,
                        const RangeCheckDef rcd)
{

  // correct theta coordinate
  if (V(Trk::theta) < s_thetaMin((int)rcd) || V(Trk::theta) > M_PI) {
    // absolute theta: repair if between -pi and +2pi.
    // differential theta: repair if between -pi and +pi
    if (V(Trk::theta) < -M_PI) {
      return false;
    }
    if (V(Trk::theta) > (rcd == differentialCheck ? M_PI : 2 * M_PI)) {
      return false;
    }
    if (V(Trk::theta) > M_PI) {
      V(Trk::theta) = 2 * M_PI - V(Trk::theta);
      V(Trk::phi) += (V(Trk::phi) > 0.0) ? -M_PI : M_PI;
    } else if (V(Trk::theta) < 0.0) {
      V(Trk::theta) = -V(Trk::theta);
      V(Trk::phi) += (V(Trk::phi) > 0.0) ? -M_PI : M_PI;
    }
    // correct also cov matrix:
    M.fillSymmetric(0, 3, -M(0, 3)); // cov(polar,locX)
    M.fillSymmetric(1, 3, -M(1, 3)); // cov(polar,locY)
    M.fillSymmetric(2, 3, -M(2, 3)); // cov(polar,azimuthal)
    M.fillSymmetric(3, 4, -M(3, 4)); // cov(polar,qOverP)
  }

  // correct phi coordinate if necessary
  if ((V(Trk::phi) > M_PI)) {
    V(Trk::phi) = fmod(V(Trk::phi) + M_PI, 2 * M_PI) - M_PI;
  } else if ((V(Trk::phi) < -M_PI)) {
    V(Trk::phi) = fmod(V(Trk::phi) - M_PI, 2 * M_PI) + M_PI;
  }

  return true;
}

/**
 * Avoid multiplications with sparse H matrices by cutting 2D rows&columns
 * out of the full cov matrix.
 */
template<int DIM>
AmgSymMatrix(DIM) projection_T(const AmgSymMatrix(5) & M, int key)
{
  if (key == 3 || key == 7 || key == 15) { // shortcuts for the most common use
                                           // cases
    return M.topLeftCorner<DIM, DIM>();
  }

  Eigen::Matrix<int, DIM, 1, 0, DIM, 1> iv;
  iv.setZero();
  for (int i = 0, k = 0; i < 5; ++i) {
    iv[k++] = key & (1 << i) ? i : 0;
  }

  AmgSymMatrix(DIM) covSubMatrix;
  covSubMatrix.setZero();
  for (int i = 0; i < DIM; ++i) {
    for (int j = 0; j < DIM; ++j) {
      covSubMatrix(i, j) = M(iv(i), iv(j));
    }
  }

  return covSubMatrix;
}

/**
 * calculations for  Kalman filter step
 * formalism.
 * We have specific method for 1D and 5D.
 * The more usual/special cases.
 * And also a templated method on DIM
 * (for the 2D,3D,4D cases)
 */
bool
calculateFilterStep_1D(Trk::TrackParameters& TP,
                       const AmgSymMatrix(5) & trkCov,
                       double measPar,
                       double measCov,
                       int paramKey,
                       Trk::FitQualityOnSurface& fitQoS)
{
  const int mk = measurementCoord_1D(paramKey);
  // get the parameters from the
  const AmgVector(5)& trkPar = TP.parameters();
  // use measuring coordinate (variable "mk") instead of reduction matrix
  const double r = measPar - trkPar(mk);
  double R = measCov + trkCov(mk, mk);
  if (R == 0.0) {
    return false;
  }
  R = 1. / R;
  // --- compute Kalman gain matrix
  AmgVector(5) K = trkCov.col(mk) * R;
  // --- compute local filtered state, here = TP+K*r = TP + TCov * H.T * R * r
  AmgVector(5) newPar = trkPar + K * r;

  if (!thetaWithinRange_5D(newPar)) {

    if (mk != Trk::theta &&
        (std::abs(R * r) > 1.0 ||
         trkCov(Trk::theta, Trk::theta) > 0.1 * s_cov0Vec(Trk::theta))) {

      AmgVector(5) dampedCov = trkCov.col(mk);

      dampedCov(Trk::theta) = dampedCov(Trk::theta) * s_thetaGainDampingValue;

      newPar = trkPar + dampedCov * R * r;

      K(Trk::theta, 0) = K(Trk::theta, 0) * s_thetaGainDampingValue;
    }
  }
  // --- compute covariance matrix of local filteres state
  AmgMatrix(5, 5) KtimesH;
  KtimesH.setZero();
  KtimesH.col(mk) = K;
  const AmgMatrix(5, 5) M = s_unitMatrix - KtimesH;
  AmgSymMatrix(5) newCov =
    trkCov.similarity(M) + K * measCov * K.transpose();

  if (!thetaPhiWithinRange_5D(newPar, absoluteCheck) &&
      !correctThetaPhiRange_5D(newPar, newCov, absoluteCheck)) {
    return false;
  }

  const double predictedResidual = (measPar - newPar(mk));
  const AmgSymMatrix(5)& updatedCov = newCov;

  double chiSquared = measCov - updatedCov(mk, mk);
  if (chiSquared != 0.0) {
    // get chi2 = r.T() * R^-1 * r
    chiSquared = predictedResidual * predictedResidual / chiSquared;
  }
  fitQoS = Trk::FitQualityOnSurface(chiSquared, 1);
  // In place update of parameters
  TP.updateParameters(newPar, newCov);
  return true;
}

bool
calculateFilterStep_5D(Trk::TrackParameters& TP,
                       const AmgSymMatrix(5) & trkCov,
                       const AmgVector(5) & measPar,
                       const AmgSymMatrix(5) & measCov,
                       Trk::FitQualityOnSurface& fQ)
{
  // get the parameter vector
  const AmgVector(5)& trkPar = TP.parameters();
  // Kalman gain K, residual r, combined covariance R
  const AmgVector(5) r = measPar - trkPar;
  // for full safety in Eigen see
  // http://eigen.tuxfamily.org/dox/classEigen_1_1FullPivLU.html
  const AmgSymMatrix(5) R = (measCov + trkCov).inverse();
  const AmgMatrix(5, 5) K = trkCov * R;
  const AmgMatrix(5, 5) M = s_unitMatrix - K;
  // --- compute local filtered state
  const AmgVector(5) newPar = trkPar + K * r;
  // --- compute filtered covariance matrix
  const AmgSymMatrix(5) newCov =
    trkCov.similarity(M) + measCov.similarity(K);
  const double chiSquared = Amg::chi2(R, r);
  // create the FQSonSurface
  fQ = Trk::FitQualityOnSurface(chiSquared, 5);
  TP.updateParameters(newPar, newCov);
  return true;
}

template<int DIM>
bool
calculateFilterStep_T(Trk::TrackParameters& TP,
                      const AmgSymMatrix(5) & trkCov,
                      const AmgVector(DIM) & measPar,
                      const AmgSymMatrix(DIM) & measCov,
                      int paramKey,
                      Trk::FitQualityOnSurface& fQ)
{

  // get the parameter vector
  const AmgVector(5)& trkPar = TP.parameters();
  // reduction matrix
  const AmgMatrix(DIM, 5) H =
    s_reMatrices.expansionMatrix(paramKey).topLeftCorner<DIM, 5>();
  // the projected parameters from the TrackParameters
  AmgVector(DIM) projTrkPar;
  if (paramKey == 3 || paramKey == 7 || paramKey == 15) {
    projTrkPar = trkPar.head<DIM>();
  } else {
    projTrkPar = H * trkPar;
  }

  // reduction matrix H, Kalman gain K, residual r, combined covariance R
  // residual after reduction
  const AmgVector(DIM) r = measPar - projTrkPar;
  // combined covariance after reduction
  const AmgSymMatrix(DIM) R =
    (measCov + projection_T<DIM>(trkCov, paramKey)).inverse();
  // Kalman gain matrix
  const AmgMatrix(5, DIM) K = trkCov * H.transpose() * R;
  const AmgMatrix(5, 5) M = s_unitMatrix - K * H;
  // --- compute local filtered state
  const AmgVector(5) newPar = trkPar + K * r;
  // --- compute filtered covariance matrix
  // C = M * trkCov * M.T() + K * covRio * K.T()
  const AmgSymMatrix(5) newCov =
    trkCov.similarity(M) + K * measCov * K.transpose();
  const double chiSquared = Amg::chi2(R, r);
  // create the FQSonSurface
  fQ = Trk::FitQualityOnSurface(chiSquared, DIM);
  // In place update of parameters
  TP.updateParameters(newPar, newCov);
  return true;
}

/*
 * Method implementing the actual kalman filter step
 * using the methods above.
 * Also does heavy use of Eigen
 * so we use flatten
 */
ATH_FLATTEN
bool
filterStep(Trk::TrackParameters& trackParameters,
           Trk::FitQualityOnSurface& fitQos,
           const Trk::LocalParameters& measurement,
           const Amg::MatrixX& measCovariance)
{

  const AmgSymMatrix(5)* trkCov = trackParameters.covariance();
  if (!trkCov) {
    return false;
  }

  const int nLocCoord = measCovariance.cols();
  if (!(measurement.dimension() == nLocCoord)) {
    return false;
  }

  switch (nLocCoord) {
    case 1: {
      return calculateFilterStep_1D(trackParameters,
                                    *trkCov,
                                    measurement(0),
                                    measCovariance(0, 0),
                                    measurement.parameterKey(),
                                    fitQos);
    }
    case 2: {
      return calculateFilterStep_T<2>(trackParameters,
                                      *trkCov,
                                      measurement.head<2>(),
                                      measCovariance.topLeftCorner<2, 2>(),
                                      measurement.parameterKey(),
                                      fitQos);
    }
    case 3: {
      return calculateFilterStep_T<3>(trackParameters,
                                      *trkCov,
                                      measurement.head<3>(0),
                                      measCovariance.topLeftCorner<3, 3>(),
                                      measurement.parameterKey(),
                                      fitQos);
    }
    case 4: {
      return calculateFilterStep_T<4>(trackParameters,
                                      *trkCov,
                                      measurement.head<4>(0),
                                      measCovariance.topLeftCorner<4, 4>(),
                                      measurement.parameterKey(),
                                      fitQos);
    }
    case 5: {
      return calculateFilterStep_5D(trackParameters,
                                    *trkCov,
                                    measurement.head<5>(),
                                    measCovariance.topLeftCorner<5, 5>(),
                                    fitQos);
    }
    default: {
      return false;
    }
  }
}

/*
 * Methods needed to calculate the
 * fit quality
 */
bool
makeChi2_1D(Trk::FitQualityOnSurface& updatedFitQoS,
            const AmgVector(5) & trkPar,
            const AmgSymMatrix(5) & trkCov,
            double valRio,
            double rioCov,
            int paramKey)
{

  const int mk = measurementCoord_1D(paramKey);
  const double r = valRio - trkPar(mk);
  //  if (mk==3) catchPiPi;
  double chiSquared = rioCov + trkCov(mk, mk);
  if (chiSquared == 0.0) {
    return false;
  }
  chiSquared = r * r / chiSquared;
  updatedFitQoS = Trk::FitQualityOnSurface(chiSquared, 1);
  return true;
}

template<int DIM>
bool
makeChi2_T(Trk::FitQualityOnSurface& updatedFitQoS,
           const AmgVector(5) & trkPar,
           const AmgSymMatrix(5) & trkCov,
           const AmgVector(DIM) & measPar,
           const AmgSymMatrix(DIM) & covPar,
           int paramKey)

{

  const AmgMatrix(DIM, 5) H =
    s_reMatrices.expansionMatrix(paramKey).topLeftCorner<DIM, 5>();
  const AmgVector(DIM) r = measPar - H * trkPar;
  // get the projected matrix
  AmgSymMatrix(DIM) R = projection_T<DIM>(trkCov, paramKey);
  R += covPar;
  // calcualte the chi2 value
  const double chiSquared = Amg::chi2(R.inverse(), r);
  updatedFitQoS = Trk::FitQualityOnSurface(chiSquared, DIM);
  return true;
}

bool
stateFitQuality(Trk::FitQualityOnSurface& updatedFitQoS,
                const Trk::TrackParameters& trkPar,
                const Trk::LocalParameters& position,
                const Amg::MatrixX& covariance)
{
  if (!trkPar.covariance()) {
    return false;
  }
  // For the LocalPos. version, need to get # meas. coord. from covariance
  // matrix.
  const int nLocCoord = covariance.cols();
  switch (nLocCoord) {
    case 1: {
      return makeChi2_1D(updatedFitQoS,
                         trkPar.parameters(),
                         (*trkPar.covariance()),
                         position[Trk::locX],
                         covariance(0, 0),
                         1);
    }
    case 2: {
      return makeChi2_T<2>(updatedFitQoS,
                           trkPar.parameters(),
                           (*trkPar.covariance()),
                           position,
                           covariance.topLeftCorner<2, 2>(),
                           3);
    }
    default: {
      return false;
    }
  }
}

/*
 * Methods to adjust the weights of the
 * multicomponent state (Gaussian sum)
 * given a measurement.
 */
template<int DIM>
std::pair<double, double>
calculateWeight_T(const Trk::TrackParameters* componentTrackParameters,
                  const AmgSymMatrix(5) * predictedCov,
                  const AmgVector(DIM) & measPar,
                  const AmgSymMatrix(DIM) & measCov,
                  int paramKey)
{
  // Define the expansion matrix
  const AmgMatrix(DIM, 5) H =
    s_reMatrices.expansionMatrix(paramKey).topLeftCorner<DIM, 5>();

  // Calculate the residual
  const AmgVector(DIM) r = measPar - H * componentTrackParameters->parameters();
  // Residual covariance. Posterior weights is calculated used predicted state
  // and measurement. Therefore add covariances
  AmgSymMatrix(DIM) const R(measCov + predictedCov->similarity(H));
  // compute determinant of residual
  const double det = R.determinant();
  if (det == 0) {
    return { 0, 0 };
  }
  // Compute Chi2
  return {det, (1. / (double)DIM) * Amg::chi2(R.inverse(), r)};
}

std::pair<double, double>
calculateWeight_1D(const Trk::TrackParameters* componentTrackParameters,
                   const AmgSymMatrix(5) * predictedCov,
                   const double measPar,
                   const double measCov,
                   int paramKey)
{
  const int mk = measurementCoord_1D(paramKey);
  // Calculate the residual
  const double r = measPar - (componentTrackParameters->parameters())(mk);
  // Residual covariance. Posterior weights is calculated used predicted state
  // and measurement. Therefore add covariances
  const double R = measCov + (*predictedCov)(mk, mk);
  if (R == 0) {
    return { 0, 0 };
  }
  // Compute Chi2
  return { R, r * r / R };
}

std::pair<double, double>
calculateWeight_2D_3(const Trk::TrackParameters* componentTrackParameters,
                     const AmgSymMatrix(5) * predictedCov,
                     const AmgVector(2) & measPar,
                     const AmgSymMatrix(2) & measCov)
{
  // Calculate the residual
  const AmgVector(2) r =
    measPar - componentTrackParameters->parameters().head<2>();
  // Residual covariance. Posterior weights is calculated used predicted state
  // and measurement. Therefore add covariances
  const AmgSymMatrix(2) R(measCov + predictedCov->topLeftCorner<2, 2>());
  // compute determinant of residual
  const double det = R.determinant();
  if (det == 0) {
    return { 0, 0 };
  }
  // Compute Chi2
  return {det, 0.5 * Amg::chi2(R.inverse(), r)};
}

Trk::MultiComponentState
weights(Trk::MultiComponentState&& predictedState,
        const Trk::MeasurementBase& measurement)
{

  const size_t predictedStateSize = predictedState.size();
  if (predictedStateSize == 0) {
    return {};
  }
  if (predictedStateSize > GSFConstants::maxNumberofStateComponents) {
    throw std::runtime_error("PosteriorWeightsCalculator :Invalid predictedState size");
  }
  const Trk::LocalParameters& measurementLocalParameters =
    measurement.localParameters();
  const int nLocCoord = measurement.localParameters().dimension();
  if (nLocCoord < 1 || nLocCoord > 5) {
    return {};
  }

  // Move  to output and update
  Trk::MultiComponentState returnMultiComponentState = std::move(predictedState);

  // Calculate chi2 and determinant of each component.
  componentsCache determinantRandChi2{};
  double minimumChi2(10.e10); // Initalise high
  // Loop over all components
  for (const auto& component : returnMultiComponentState) {

    const Trk::TrackParameters* componentTrackParameters =
      component.params.get();
    if (!componentTrackParameters) {
      continue;
    }
    const AmgSymMatrix(5)* predictedCov =
      componentTrackParameters->covariance();
    if (!predictedCov) {
      continue;
    }

    std::pair<double, double> result(0, 0);
    switch (nLocCoord) {
      case 1: {
        result = calculateWeight_1D(componentTrackParameters,
                                    predictedCov,
                                    measurementLocalParameters(0),
                                    measurement.localCovariance()(0, 0),
                                    measurementLocalParameters.parameterKey());
      } break;
      case 2: {
        if (measurementLocalParameters.parameterKey() == 3) {
          result = calculateWeight_2D_3(
            componentTrackParameters,
            predictedCov,
            measurementLocalParameters.head<2>(),
            measurement.localCovariance().topLeftCorner<2, 2>());
        } else {
          result = calculateWeight_T<2>(
            componentTrackParameters,
            predictedCov,
            measurementLocalParameters.head<2>(),
            measurement.localCovariance().topLeftCorner<2, 2>(),
            measurementLocalParameters.parameterKey());
        }
      } break;
      case 3: {
        result =
          calculateWeight_T<3>(componentTrackParameters,
                               predictedCov,
                               measurementLocalParameters.head<3>(),
                               measurement.localCovariance().topLeftCorner<3, 3>(),
                               measurementLocalParameters.parameterKey());
      } break;
      case 4: {
        result =
          calculateWeight_T<4>(componentTrackParameters,
                               predictedCov,
                               measurementLocalParameters.head<4>(),
                               measurement.localCovariance().topLeftCorner<4, 4>(),
                               measurementLocalParameters.parameterKey());
      } break;
      case 5: {
        result =
          calculateWeight_T<5>(componentTrackParameters,
                               predictedCov,
                               measurementLocalParameters.head<5>(),
                               measurement.localCovariance().topLeftCorner<5, 5>(),
                               measurementLocalParameters.parameterKey());
      } break;
      default: {
      }
    }

    if (result.first == 0) {
      continue;
    }
    // Cache R and Chi2
    determinantRandChi2.elements[determinantRandChi2.numElements] = {
      result.first, result.second
    };
    ++determinantRandChi2.numElements;
    if (result.second < minimumChi2) {
      minimumChi2 = result.second;
    }
  } // end loop over components

  // If something went wrong in the loop return empty
  if (determinantRandChi2.numElements != predictedStateSize) {
    return {};
  }

  // Calculate posterior weights.
  size_t index(0);
  double sumWeights(0.);
  std::array<double, GSFConstants::maxNumberofStateComponents> fallBackWeights{};
  auto componentItr = returnMultiComponentState.begin();
  for (; componentItr != returnMultiComponentState.end();
       ++componentItr, ++index) {
    // Extract common factor to avoid numerical problems during exponentiation
    const double chi2 = determinantRandChi2.elements[index].chi2 - minimumChi2;
    const double priorWeight = componentItr->weight;
    fallBackWeights[index] = priorWeight;
    double updatedWeight(0.);
    // Determinant can not be below 1e-19. Rather ugly but protect
    // against 0 determinants Normally occur when the component is a poor fit
    if (determinantRandChi2.elements[index].determinantR > 1e-20) {
      updatedWeight =
        priorWeight *
        sqrt(1. / determinantRandChi2.elements[index].determinantR) *
        exp(-0.5 * chi2);
    } else {
      updatedWeight = 1e-10;
    }
    componentItr->weight = updatedWeight;
    sumWeights += updatedWeight;
  }
  if (sumWeights > 0.) {
    const double invertSumWeights = 1. / sumWeights;
    // Renormalise the state to total weight = 1
    for (auto& returnComponent : returnMultiComponentState) {
      returnComponent.weight *= invertSumWeights;
    }
  } else {
    // If the sum weights is less than 0 revert them back
    size_t fallbackIndex(0);
    for (auto& returnComponent : returnMultiComponentState) {
      returnComponent.weight = fallBackWeights[fallbackIndex];
      ++fallbackIndex;
    }
  }
  return returnMultiComponentState;
}

/*
 * Methods to handle cases with invalid
 * components
 */
bool
invalidComponent(const Trk::TrackParameters* trackParameters)
{
  const auto* measuredCov = trackParameters->covariance();
  if (!measuredCov) {
    return true;
  }

  for (int i = 0; i < 5; ++i) {
    if ((*measuredCov)(i, i) <= 0.) {
      return true;
    }
  }

  return false;
}

Trk::MultiComponentState
rebuildState(Trk::MultiComponentState&& stateBeforeUpdate)
{
  Trk::MultiComponentState stateWithInsertedErrors =
    std::move(stateBeforeUpdate);
  // We need to loop checking for invalid componets i.e negative covariance
  // diagonal elements and update them with a large covariance matrix
  for (const Trk::ComponentParameters& component : stateWithInsertedErrors) {
    const Trk::TrackParameters* trackParameters = component.params.get();
    const bool rebuildCov = invalidComponent(trackParameters);
    if (rebuildCov) {
      AmgSymMatrix(5) bigNewCovarianceMatrix = AmgSymMatrix(5)::Zero();
      const double covarianceScaler = 1.;
      bigNewCovarianceMatrix(0, 0) = 250. * covarianceScaler;
      bigNewCovarianceMatrix(1, 1) = 250. * covarianceScaler;
      bigNewCovarianceMatrix(2, 2) = 0.25;
      bigNewCovarianceMatrix(3, 3) = 0.25;
      bigNewCovarianceMatrix(4, 4) = 0.001 * 0.001;
      component.params->updateParameters(trackParameters->parameters(),
                                        bigNewCovarianceMatrix);
    }
  }
  return stateWithInsertedErrors;
}

/*
 * Methods that bring all
 * weights adjustement, filter step,
 * and FitQuality together,
 * so as to update the current
 * multi-component state given a measurement
 */
Trk::MultiComponentState
calculateFilterStep(Trk::MultiComponentState&& stateBeforeUpdate,
                    const Trk::MeasurementBase& measurement,
                    Trk::FitQualityOnSurface& fitQoS)
{
  // state Assembler cache
  Trk::MultiComponentStateAssembler::Cache cache;
  if (stateBeforeUpdate.empty()) {
    return {};
  }

  // Calculate the weight of each component after the measurement
  Trk::MultiComponentState stateWithNewWeights =
    weights(std::move(stateBeforeUpdate), measurement);

  if (stateWithNewWeights.empty()) {
    return {};
  }

  double chiSquared = 0;
  int degreesOfFreedom = 0;
  for (Trk::ComponentParameters& component : stateWithNewWeights) {
    if (stateWithNewWeights.size() > 1 &&
        std::abs(component.params->parameters()[Trk::qOverP]) > 0.033333) {
      continue;
    }
    Trk::FitQualityOnSurface componentFitQuality;
    /// Update the component in place
    bool const updateSuccess = filterStep(*(component.params),
                                    componentFitQuality,
                                    measurement.localParameters(),
                                    measurement.localCovariance());
    if (!updateSuccess) {
      continue;
    }

    if (invalidComponent(component.params.get())) {
      continue;
    }

    if (componentFitQuality.chiSquared() <= 0.) {
      continue;
    }

    chiSquared += component.weight * componentFitQuality.chiSquared();

    // The same measurement is included in each update
    // so we can update the degree of freedom only
    if (degreesOfFreedom == 0.0) {
      degreesOfFreedom = componentFitQuality.numberDoF();
    }

    // Add component to state being prepared for assembly
    Trk::MultiComponentStateAssembler::addComponent(cache,
                                                    std::move(component));
  }

  Trk::MultiComponentState assembledUpdatedState =
    Trk::MultiComponentStateAssembler::assembledState(std::move(cache));

  if (assembledUpdatedState.empty()) {
    return {};
  }

  fitQoS.setChiSquared(chiSquared);
  fitQoS.setNumberDoF(degreesOfFreedom);
  // Renormalise state
  Trk::MultiComponentStateHelpers::renormaliseState(assembledUpdatedState);
  return assembledUpdatedState;
}

} // end of anonymous namespace

// The external/interface methods
// implemented in terms of the internal ones.
Trk::MultiComponentState
Trk::GsfMeasurementUpdator::update(Trk::MultiComponentState&& stateBeforeUpdate,
                                   const Trk::MeasurementBase& measurement,
                                   FitQualityOnSurface& fitQoS)
{

  // Check all components have associated error matrices
  bool rebuildStateWithErrors = false;
  // Perform initial check of state awaiting update. If all states have
  // associated error matricies then no need to perform the rebuild
  for (const Trk::ComponentParameters& component : stateBeforeUpdate) {
    rebuildStateWithErrors =
      rebuildStateWithErrors || invalidComponent(component.params.get());
  }

  if (rebuildStateWithErrors) {
    Trk::MultiComponentState stateWithInsertedErrors =
      rebuildState(std::move(stateBeforeUpdate));
    // Perform the measurement update with the modified state
    Trk::MultiComponentState updatedState = calculateFilterStep(
      std::move(stateWithInsertedErrors), measurement, fitQoS);
    if (updatedState.empty()) {
      return {};
    }
    return updatedState;
  }

  // Perform the measurement update
  Trk::MultiComponentState updatedState = calculateFilterStep(
    std::move(stateBeforeUpdate), measurement, fitQoS);

  if (updatedState.empty()) {
    return {};
  }
  return updatedState;
}

Trk::FitQualityOnSurface
Trk::GsfMeasurementUpdator::fitQuality(const MultiComponentState& updatedState,
                                       const MeasurementBase& measurement)
{
  double chi2 = 0;
  Trk::FitQualityOnSurface componentFitQuality;
  for (const Trk::ComponentParameters& component: updatedState) {
    stateFitQuality(componentFitQuality,
                    *component.params.get(),
                    measurement.localParameters(),
                    measurement.localCovariance());

    chi2 += component.weight * componentFitQuality.chiSquared();
  }

  //The same measurement is included
  return { chi2, componentFitQuality.numberDoF() };
}

