/*
  Copyright (C) 2020-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ElectronMaterialMixtureConvolution.cxx
 * @begin         July 20 2020
 * @author        Anthony Morley, Christos Anastopoulos
 * @brief         Implementation code for GSF material mixture convolution
 */

#include "TrkGaussianSumFilter/ElectronMaterialMixtureConvolution.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
#include "TrkGaussianSumFilterUtils/KLGaussianMixtureReduction.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateAssembler.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateCombiner.h"

#include "TrkParameters/ComponentParameters.h"
#include "CxxUtils/inline_hints.h"
//
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkSurfaces/PerigeeSurface.h"

#include <array>
namespace {

ATH_FLATTEN
inline void
dummyCacheElement(GsfMaterial::Combined& elem)
{
  elem.numEntries = 1;
  elem.deltaPs[0] = 0;
  elem.deltaParameters[0] = AmgVector(5)::Zero();
  elem.deltaCovariances[0] = AmgSymMatrix(5)::Zero();
}

// Avoid out-of-line Eigen calls
ATH_FLATTEN
inline void
updateCacheElement(GsfMaterial::Combined& update,
                   size_t index,
                   const AmgVector(5) & parameters,
                   const AmgSymMatrix(5) * covariance)
{
  update.deltaParameters[index] = parameters;
  if (covariance) {
    update.deltaCovariances[index] += *covariance;
  } else {
    update.deltaCovariances[index].setZero();
  }
}

bool
updateP(double& qOverP, double deltaP)
{
  double p = 1. / std::abs(qOverP);
  p += deltaP;
  if (p <= 0.) {
    return false;
  }
  qOverP = qOverP > 0. ? 1. / p : -1. / p;
  return true;
}

std::pair<const Trk::MaterialProperties*, double>
getMaterialProperties(const Trk::TrackParameters* trackParameters,
                      const Trk::Layer& layer)
{

  const Trk::MaterialProperties* materialProperties(nullptr);
  double pathCorrection(0.);

  // Check that the material properties have been defined - if not define them
  // from the layer information
  materialProperties = materialProperties
                         ? materialProperties
                         : layer.fullUpdateMaterialProperties(*trackParameters);
  // Bail out if still no material properties can be found
  if (!materialProperties) {
    return { nullptr, 0 };
  }
  // Define the path correction
  pathCorrection =
    pathCorrection > 0.
      ? pathCorrection
      : layer.surfaceRepresentation().pathCorrection(
          trackParameters->position(), trackParameters->momentum());

  // The pathlength ( in mm ) is the path correction * the thickness of the
  // material
  const double pathLength = pathCorrection * materialProperties->thickness();
  return { materialProperties, pathLength };
}

} // end of anonymous namespace

Trk::ElectronMaterialMixtureConvolution::ElectronMaterialMixtureConvolution(
  const std::string& type,
  const std::string& name,
  const IInterface* parent)
  : AthAlgTool(type, name, parent)
{
  declareInterface<IMaterialMixtureConvolution>(this);
}

Trk::ElectronMaterialMixtureConvolution::~ElectronMaterialMixtureConvolution() =
  default;

StatusCode
Trk::ElectronMaterialMixtureConvolution::initialize()
{
  if (m_maximumNumberOfComponents > GSFConstants::maxNumberofStateComponents) {
    ATH_MSG_FATAL("Requested MaximumNumberOfComponents > "
                  << GSFConstants::maxNumberofStateComponents);
    return StatusCode::FAILURE;
  }

  m_materialEffects = ElectronCombinedMaterialEffects(
    m_parameterisationFileName, m_parameterisationFileNameHighX0);
  return StatusCode::SUCCESS;
}

/* ==========================================
   Update with full material effects
   ========================================== */

Trk::MultiComponentState
Trk::ElectronMaterialMixtureConvolution::update(
  std::vector<GsfMaterial::Combined>& caches,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Layer& layer,
  Trk::PropDirection direction,
  Trk::ParticleHypothesis particleHypothesis) const
{

  Trk::MultiComponentState updatedMergedState = update(
    caches, multiComponentState, layer, direction, particleHypothesis, Normal);

  if (updatedMergedState.empty()) {
    return {};
  }
  // Renormalise state
  MultiComponentStateHelpers::renormaliseState(updatedMergedState);

  return updatedMergedState;
}

/* ==========================================
   Update with pre-update material effects
========================================== */

Trk::MultiComponentState
Trk::ElectronMaterialMixtureConvolution::preUpdate(
  std::vector<GsfMaterial::Combined>& caches,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Layer& layer,
  Trk::PropDirection direction,
  Trk::ParticleHypothesis particleHypothesis) const
{
  /* -------------------------------------
     Preliminary checks
     ------------------------------------- */
  Trk::MultiComponentState updatedMergedState = update(caches,
                                                       multiComponentState,
                                                       layer,
                                                       direction,
                                                       particleHypothesis,
                                                       Preupdate);
  if (updatedMergedState.empty()) {
    return {};
  }
  // Renormalise state
  MultiComponentStateHelpers::renormaliseState(updatedMergedState);

  return updatedMergedState;
}

/* ==========================================
   Update with post-update material effects
   ========================================== */

Trk::MultiComponentState
Trk::ElectronMaterialMixtureConvolution::postUpdate(
  std::vector<GsfMaterial::Combined>& caches,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Layer& layer,
  Trk::PropDirection direction,
  Trk::ParticleHypothesis particleHypothesis) const
{

  /* -------------------------------------
     Preliminary checks
     ------------------------------------- */

  Trk::MultiComponentState updatedMergedState = update(caches,
                                                       multiComponentState,
                                                       layer,
                                                       direction,
                                                       particleHypothesis,
                                                       Postupdate);

  if (updatedMergedState.empty()) {
    return {};
  }
  // Renormalise state
  MultiComponentStateHelpers::renormaliseState(updatedMergedState);

  return updatedMergedState;
}

Trk::MultiComponentState
Trk::ElectronMaterialMixtureConvolution::update(
  std::vector<GsfMaterial::Combined>& caches,
  const Trk::MultiComponentState& inputState,
  const Trk::Layer& layer,
  Trk::PropDirection direction,
  Trk::ParticleHypothesis,
  MaterialUpdateType updateType) const
{

  // Check the multi-component state is populated
  if (inputState.empty()) {
    return {};
  }

  double updateFactor(1.);
  // Full method does this for each component which i don't think this is needed
  if (updateType == Preupdate) {
    updateFactor =
      layer.preUpdateMaterialFactor(*inputState.front().params, direction);
  } else if (updateType == Postupdate) {
    updateFactor =
      layer.postUpdateMaterialFactor(*inputState.front().params, direction);
  }

  if (updateFactor < 0.01) {
    // Bail out as factor is too small to bother about
    return {};
  }

  caches.resize(inputState.size());

  // Fill cache and work out how many final components there should be
  size_t n(0);
  for (size_t i(0); i < inputState.size(); ++i) {
    const AmgSymMatrix(5)* measuredCov = inputState[i].params->covariance();
    // If the momentum is too dont apply material effects
    if (inputState[i].params->momentum().mag() <= 250. * Gaudi::Units::MeV) {
      dummyCacheElement(caches[i]);
      updateCacheElement(
        caches[i], 0, inputState[i].params->parameters(), measuredCov);
      caches[i].weights[0] = inputState[i].weight;
      n += caches[i].numEntries;
      continue;
    }

    // Get the material effects and store them in the cache
    std::pair<const Trk::MaterialProperties*, double> matPropPair =
        getMaterialProperties(inputState[i].params.get(), layer);

    if (!matPropPair.first) {
      dummyCacheElement(caches[i]);
      updateCacheElement(
        caches[i], 0, inputState[i].params->parameters(), measuredCov);
      caches[i].weights[0] = inputState[i].weight;
      n += caches[i].numEntries;
      continue;
    }
    // Now we can compute/apply actual material effects
    // Apply the update factor
    matPropPair.second *= updateFactor;
    m_materialEffects.compute(caches[i],
                              inputState[i],
                              *matPropPair.first,
                              matPropPair.second,
                              direction);

    // Apply material effects to input state and store results in cache
    for (size_t j(0); j < caches[i].numEntries; ++j) {
      updateCacheElement(
        caches[i], j, inputState[i].params->parameters(), measuredCov);
      // Adjust q/p of the (delta) Parameters
      // make sure update is good.
      if (!updateP(caches[i].deltaParameters[j][Trk::qOverP],
                   caches[i].deltaPs[j])) {
        ATH_MSG_ERROR("Cannot update state vector momentum!!!");
        return {};
      }
      // Store component weight
      caches[i].weights[j] *= inputState[i].weight;
      // Ensure weight of component is not too small to save us from potential
      // FPE's Value. Weights are double so the min of float should
      // be small enough and should be handled
      if (caches[i].weights[j] < std::numeric_limits<float>::min()) {
        caches[i].weights[j] = std::numeric_limits<float>::min();
      }
    }
    n += caches[i].numEntries;
  } // End of loop filling the cache

  // Fill information for  calculating which components to merge
  // In addition scan all components for covariance matrices. If one or more
  // component is missing an error matrix, component reduction is impossible.
  bool componentWithoutMeasurement = false;

  GSFUtils::Component1DArray componentsArray;
  componentsArray.numComponents = n;
  std::array<std::pair<size_t, size_t>,
             GSFConstants::maxComponentsAfterConvolution>
      indices{};

  size_t k(0);
  for (size_t i(0); i < inputState.size(); ++i) {
    for (size_t j(0); j < caches[i].numEntries; ++j) {
      const AmgSymMatrix(5)* measuredCov = inputState[i].params->covariance();
      // Fill in infomation
      const double cov =
        measuredCov ? caches[i].deltaCovariances[j](Trk::qOverP, Trk::qOverP)
                    : -1.;
      if (!measuredCov) {
        componentWithoutMeasurement = true;
      }

      componentsArray.components[k].mean =
        caches[i].deltaParameters[j][Trk::qOverP];
      componentsArray.components[k].cov = cov;
      componentsArray.components[k].invCov = cov > 0 ? 1. / cov : 1e10;
      componentsArray.components[k].weight = caches[i].weights[j];
      indices[k] = { i, j };
      ++k;
    }
  }

  // fallback if we have a component without measurement
  if (componentWithoutMeasurement) {
    auto* result = std::max_element(
      componentsArray.components.data(),
      componentsArray.components.data() + componentsArray.numComponents,
      [](const auto& a, const auto& b) { return a.weight < b.weight; });
    auto index = std::distance(componentsArray.components.data(), result);

    // Build the first TP
    const size_t stateIndex = indices[index].first;
    const size_t materialIndex = indices[index].second;

    AmgVector(5)& updatedStateVector =
      caches[stateIndex].deltaParameters[materialIndex];
    const AmgSymMatrix(5)* measuredCov =
      inputState[stateIndex].params->covariance();
    std::optional<AmgSymMatrix(5)> updatedCovariance = std::nullopt;
    if (measuredCov &&
        caches[stateIndex].deltaCovariances.size() > materialIndex) {
      updatedCovariance =
        AmgSymMatrix(5)(caches[stateIndex].deltaCovariances[materialIndex]);
    }
    std::unique_ptr<Trk::TrackParameters> updatedTrackParameters =
      inputState[stateIndex]
        .params->associatedSurface()
        .createUniqueTrackParameters(updatedStateVector[Trk::loc1],
                                     updatedStateVector[Trk::loc2],
                                     updatedStateVector[Trk::phi],
                                     updatedStateVector[Trk::theta],
                                     updatedStateVector[Trk::qOverP],
                                     std::move(updatedCovariance));

    Trk::ComponentParameters dummyCompParams = {std::move(updatedTrackParameters),
                                             1.};
    Trk::MultiComponentState returnMultiState;
    returnMultiState.push_back(std::move(dummyCompParams));
    return returnMultiState;
  }

  // Gather the merges we need
  GSFUtils::MergeArray KL;
  if (n > m_maximumNumberOfComponents) {
    KL = findMerges(componentsArray, m_maximumNumberOfComponents);
  }

  // Merge components "From" to components "To"
  MultiComponentStateAssembler::Cache assemblerCache;
  std::array<bool, GSFConstants::maxComponentsAfterConvolution> isMerged = {};
  const int returnedMerges = KL.numMerges;

  for (int i = 0; i < returnedMerges; ++i) {
    const int8_t mini = KL.merges[i].To;
    const int8_t minj = KL.merges[i].From;
    if (isMerged[minj]) {
      ATH_MSG_WARNING("Component is already merged " << static_cast<int>(minj));
      continue;
    }
    // Get the first TP
    const size_t stateIndex = indices[mini].first;
    const size_t materialIndex = indices[mini].second;
    // Copy weight and first parameters as they are needed later on
    // for updating the covariance
    const AmgVector(5) firstParameters =
      caches[stateIndex].deltaParameters[materialIndex];
    const double firstWeight = caches[stateIndex].weights[materialIndex];

    // Get the second TP
    const size_t stateIndex2 = indices[minj].first;
    const size_t materialIndex2 = indices[minj].second;

    // Some values for sanity checks
    isMerged[minj] = true;

    // Update first parameters and weight
    Trk::MultiComponentStateCombiner::combineParametersWithWeight(
      caches[stateIndex].deltaParameters[materialIndex],
      caches[stateIndex].weights[materialIndex],
      caches[stateIndex2].deltaParameters[materialIndex2],
      caches[stateIndex2].weights[materialIndex2]);

    // Update covariance
    Trk::MultiComponentStateCombiner::combineCovWithWeight(
      firstParameters,
      caches[stateIndex].deltaCovariances[materialIndex],
      firstWeight,
      caches[stateIndex2].deltaParameters[materialIndex2],
      caches[stateIndex2].deltaCovariances[materialIndex2],
      caches[stateIndex2].weights[materialIndex2]);

    // Reset 2nd parameters values just for clarity
    caches[stateIndex2].deltaParameters[materialIndex2].setZero();
    caches[stateIndex2].deltaCovariances[materialIndex2].setZero();
  }

  // Loop over remaining unmerged components
  for (size_t i(0); i < n; ++i) {
    if (isMerged[i]) {
      continue;
    }

    // Build the TP
    const size_t stateIndex = indices[i].first;
    const size_t materialIndex = indices[i].second;
    AmgVector(5)& stateVector =
      caches[stateIndex].deltaParameters[materialIndex];
    AmgSymMatrix(5)& measuredCov =
      caches[stateIndex].deltaCovariances[materialIndex];

    std::unique_ptr<Trk::TrackParameters> updatedTrackParameters =
      inputState[stateIndex]
        .params->associatedSurface()
        .createUniqueTrackParameters(stateVector[Trk::loc1],
                                     stateVector[Trk::loc2],
                                     stateVector[Trk::phi],
                                     stateVector[Trk::theta],
                                     stateVector[Trk::qOverP],
                                     measuredCov);

    const double updatedWeight = caches[stateIndex].weights[materialIndex];

    assemblerCache.multiComponentState.push_back({
      std::move(updatedTrackParameters), updatedWeight});
    assemblerCache.validWeightSum += updatedWeight;
  }

  // Check all weights
  Trk::MultiComponentState mergedState =
    MultiComponentStateAssembler::assembledState(std::move(assemblerCache));

  if (mergedState.size() > m_maximumNumberOfComponents)
    ATH_MSG_ERROR("Merging failed, target size: " << m_maximumNumberOfComponents
                                                  << " final size: "
                                                  << mergedState.size());
  return mergedState;
}

