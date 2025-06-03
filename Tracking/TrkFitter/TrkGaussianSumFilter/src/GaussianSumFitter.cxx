/*§
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GaussianSumFitter.cxx
 * @begin         Monday 7th March 2005
 * @author        Anthony Morley, Tom  Atkinson, Christos Anastopoulos
 * @brief         Implementation code for Gaussian Sum Fitter class
 */

#include "TrkGaussianSumFilter/GaussianSumFitter.h"
#include "TrkGaussianSumFilter/IMultiStateExtrapolator.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
#include "TrkGaussianSumFilterUtils/MultiComponentStateCombiner.h"
//
#include "TrkFitterUtils/TrackFitInputPreparator.h"
//
#include "TrkCaloCluster_OnTrack/CaloCluster_OnTrack.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventUtils/MeasurementBaseComparisonFunction.h"
#include "TrkEventUtils/PrepRawDataComparisonFunction.h"
#include "TrkMaterialOnTrack/EstimatedBremOnTrack.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"
//
#include <algorithm>
#include <vector>

namespace {

std::unique_ptr<Trk::FitQuality>
buildFitQuality(const Trk::GaussianSumFitter::GSFTrajectory& smoothedTrajectory)
{

  /*
   * Build fit quality by summing the chi2 and ndof for each
   * measurements
   */
  double chiSquared = 0.;
  int numberDoF = -5;
  Trk::GaussianSumFitter::GSFTrajectory::const_iterator stateOnSurface =
    smoothedTrajectory.begin();
  for (; stateOnSurface != smoothedTrajectory.end(); ++stateOnSurface) {
    if (!stateOnSurface->typeFlags.test(Trk::TrackStateOnSurface::Measurement)) {
      continue;
    }
    if (!stateOnSurface->fitQualityOnSurface) {
      continue;
    }
    chiSquared += stateOnSurface->fitQualityOnSurface.chiSquared();
    numberDoF += stateOnSurface->fitQualityOnSurface.numberDoF();
  }
  if (std::isnan(chiSquared) || chiSquared <= 0.) {
    return nullptr;
  }
  return std::make_unique<Trk::FitQuality>(chiSquared, numberDoF);
}

/*
 * Helper to return the MultiComponent TSOS
 * that we will push back to the Trajectory DataVector
 * (taking ownership)
 */
GSFTsos smootherHelper(
    Trk::MultiComponentState&& updatedState,
    std::unique_ptr<Trk::MeasurementBase>&& measurement,
    const Trk::FitQualityOnSurface& fitQuality,
    bool combineToSingle,
    bool useMode)

{
  if (combineToSingle) {
    auto combinedLastState = Trk::MultiComponentStateCombiner::combineToSingle(updatedState, useMode);
    if (combinedLastState) {
      return {fitQuality, std::move(measurement), std::move(combinedLastState),std::move(updatedState)};
    }
  }
  return {fitQuality, std::move(measurement), nullptr, std::move(updatedState)};
}

} // end of anonymous namespace

Trk::GaussianSumFitter::GaussianSumFitter(const std::string& type,
                                          const std::string& name,
                                          const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_directionToPerigee(Trk::oppositeMomentum)
  , m_trkParametersComparisonFunction{}
  , m_sortingReferencePoint{ 0, 0, 0 }
{
  declareInterface<ITrackFitter>(this);
  declareProperty("SortingReferencePoint", m_sortingReferencePoint);
}

StatusCode
Trk::GaussianSumFitter::initialize()
{

  if (m_maximumNumberOfComponents > GSFConstants::maxNumberofStateComponents) {
    ATH_MSG_FATAL("Requested MaximumNumberOfComponents > "
                  << GSFConstants::maxNumberofStateComponents);
    return StatusCode::FAILURE;
  }

  // GSF extrapolator
  ATH_CHECK(m_extrapolator.retrieve());

  // We need a to create RIO_OnTrack (calibrated/corrected)
  // measurements only if we start from PrePRawData.
  if (!m_refitOnMeasurementBase) {
    ATH_CHECK(m_rioOnTrackCreator.retrieve());
  } else {
    m_rioOnTrackCreator.disable();
  }
  // Initialise the closest track parameters search algorithm
  const Amg::Vector3D referencePosition(m_sortingReferencePoint[0],
                                  m_sortingReferencePoint[1],
                                  m_sortingReferencePoint[2]);

  m_trkParametersComparisonFunction =
    Trk::TrkParametersComparisonFunction(referencePosition);

  return StatusCode::SUCCESS;
}

/*
 * Interface Method
 * Refitting of an input track.
 * Extract the measurementBase or
 * PrepRawData and refit.
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(
  const EventContext& ctx,
  const Trk::Track& inputTrack,
  const Trk::RunOutlierRemoval outlierRemoval,
  const Trk::ParticleHypothesis particleHypothesis) const
{
  // Check that the input track has well defined parameters
  if (inputTrack.trackParameters()->empty()) {
    ATH_MSG_FATAL("No track parameters on Track!");
    return nullptr;
  }
  // Check that the input track has associated MeasurementBase objects
  if (inputTrack.trackStateOnSurfaces()->empty()) {
    ATH_MSG_FATAL("Empty MeasurementBase ");
    return nullptr;
  }
  // Retrieve the set of track parameters closest to the reference point
  const Trk::TrackParameters* parametersNearestReference =
    *(std::min_element(inputTrack.trackParameters()->begin(),
                       inputTrack.trackParameters()->end(),
                       m_trkParametersComparisonFunction));

  // If we refit using measurement base then
  // extract the measurements from the input track
  // and fit them
  if (m_refitOnMeasurementBase) {

    MeasurementSet measurementSet;

    for (const auto* tsos : *(inputTrack.trackStateOnSurfaces())) {

      if (!(tsos)) {
        ATH_MSG_WARNING("Track contains an empty MeasurementBase object ");
        continue;
      }

      const MeasurementBase* meas = tsos->measurementOnTrack();
      if (meas) {
        if (tsos->type(TrackStateOnSurface::Measurement)) {
          measurementSet.push_back(meas);
        } else if (m_reintegrateOutliers &&
                   tsos->type(TrackStateOnSurface::Outlier)) {
          measurementSet.push_back(meas);
        }
      }
    }

    return fit(ctx,
               measurementSet,
               *parametersNearestReference,
               outlierRemoval,
               particleHypothesis);
  }

  // If we refit using PrepRawData level then
  // extract the the PrepRawData  from the input track
  // and fit them
  PrepRawDataSet prepRawDataSet;

  for (const auto* meas : *(inputTrack.measurementsOnTrack())) {

    if (!meas) {
      continue;
    }
    const Trk::RIO_OnTrack* rioOnTrack = nullptr;
    if (meas->type(Trk::MeasurementBaseType::RIO_OnTrack)) {
      rioOnTrack = static_cast<const Trk::RIO_OnTrack*>(meas);
    }
    if (!rioOnTrack) {
      continue;
    }
    const PrepRawData* prepRawData = rioOnTrack->prepRawData();
    if (!prepRawData) {
      continue;
    }
    prepRawDataSet.push_back(prepRawData);
  }

  return fit(ctx,
             prepRawDataSet,
             *parametersNearestReference,
             outlierRemoval,
             particleHypothesis);
}

/*
 * Interface Method
 * Fitting of a set of PrepRawData objects
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(
  const EventContext& ctx,
  const Trk::PrepRawDataSet& prepRawDataSet,
  const Trk::TrackParameters& estimatedParametersNearOrigin,
  const Trk::RunOutlierRemoval /* Not used*/,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  // Protect against empty PrepRawDataSet object
  if (prepRawDataSet.empty()) {
    ATH_MSG_FATAL("PrepRawData set for fit is empty");
    return nullptr;
  }

  // We need a sorted PrepRawDataSet
  Trk::PrepRawDataSet sortedPrepRawDataSet = PrepRawDataSet(prepRawDataSet);
  const Trk::PrepRawDataComparisonFunction prdComparisonFunction =
    Trk::PrepRawDataComparisonFunction(
      estimatedParametersNearOrigin.position(),
      estimatedParametersNearOrigin.momentum());

  std::sort(sortedPrepRawDataSet.begin(),
            sortedPrepRawDataSet.end(),
            prdComparisonFunction);

  // Create Extrapolator cache that holds material effects cache;
  Trk::IMultiStateExtrapolator::Cache extrapolatorCache;
  // Perform GSF forward fit
  GSFTrajectory forwardTrajectory =
      forwardPRDfit(ctx, extrapolatorCache, sortedPrepRawDataSet,
             estimatedParametersNearOrigin, particleHypothesis);

  if (forwardTrajectory.empty()) {
    return nullptr;
  }
  // Perform GSF smoother operation
  GSFTrajectory smoothedTrajectory =
    smootherFit(ctx, extrapolatorCache, forwardTrajectory, particleHypothesis);
  if (smoothedTrajectory.empty()) {
    return nullptr;
  }

  // Fit quality
  std::unique_ptr<FitQuality> fitQuality = buildFitQuality(smoothedTrajectory);
  if (!fitQuality) {
    return nullptr;
  }

  // Create parameters at perigee if needed
  auto perigeeMultiStateOnSurface =
    makePerigee(ctx, extrapolatorCache, smoothedTrajectory, particleHypothesis);
  if (!perigeeMultiStateOnSurface.multiComponentState.empty()) {
    smoothedTrajectory.push_back(std::move(perigeeMultiStateOnSurface));
  } else {
    return nullptr;
  }

  // Reverse the order of the TSOS's after the smoother
  // to make be order flow from inside to out
  std::reverse(smoothedTrajectory.begin(), smoothedTrajectory.end());

  // Create Trk::Track
  Trk::TrackInfo info(Trk::TrackInfo::GaussianSumFilter, particleHypothesis);
  info.setTrackProperties(TrackInfo::BremFit);
  info.setTrackProperties(TrackInfo::BremFitSuccessful);
  return std::make_unique<Track>(
      info, convertTrajToTrack(smoothedTrajectory),
      std::move(fitQuality));
}

/*
 * Interface method
 * Fitting of a set of MeasurementBase objects
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(
  const EventContext& ctx,
  const Trk::MeasurementSet& measurementSet,
  const Trk::TrackParameters& estimatedParametersNearOrigin,
  const Trk::RunOutlierRemoval /* Not used*/,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  // Protect against empty PrepRawDataSet object
  if (measurementSet.empty()) {
    ATH_MSG_FATAL("MeasurementSet for fit is empty");
    return nullptr;
  }

  // We need to separate the possible CaloCluster on Track
  // measurement from the rest
  const Trk::CaloCluster_OnTrack* ccot(nullptr);

  Trk::MeasurementSet cleanedMeasurementSet;
  cleanedMeasurementSet.reserve(measurementSet.size());

  for (const auto* meas : measurementSet) {
    if (!meas) {
      ATH_MSG_WARNING(
        "There is an empty MeasurementBase object in the track! ");
      continue;
    }
    if (meas->type(Trk::MeasurementBaseType::CaloCluster_OnTrack)) {
      ccot = static_cast<const Trk::CaloCluster_OnTrack*>(meas);
    } else {
      cleanedMeasurementSet.push_back(meas);
    }
  }

  // We need a sorted measurement set
  Trk::MeasurementSet sortedMeasurementSet =
    MeasurementSet(cleanedMeasurementSet);

  const Trk::MeasurementBaseComparisonFunction measurementBaseComparisonFunction(
    estimatedParametersNearOrigin.position(),
    estimatedParametersNearOrigin.momentum());
  sort(sortedMeasurementSet.begin(),
       sortedMeasurementSet.end(),
       measurementBaseComparisonFunction);

  // Create Extrapolator cache that holds material effects cache;
  Trk::IMultiStateExtrapolator::Cache extrapolatorCache;

  // Perform GSF forwards fit
  GSFTrajectory forwardTrajectory =
      forwardMeasurementFit(ctx, extrapolatorCache, sortedMeasurementSet,
                      estimatedParametersNearOrigin, particleHypothesis);

  if (forwardTrajectory.empty()) {
    return nullptr;
  }

  // Perform GSF smoother operation
  GSFTrajectory smoothedTrajectory = smootherFit(
    ctx, extrapolatorCache, forwardTrajectory, particleHypothesis, ccot);
  if (smoothedTrajectory.empty()) {
    return nullptr;
  }

  // fit quality
  std::unique_ptr<FitQuality> fitQuality = buildFitQuality(smoothedTrajectory);
  if (!fitQuality) {
    return nullptr;
  }

  // Create parameters at perigee if needed
  auto perigeeMultiStateOnSurface =
    makePerigee(ctx, extrapolatorCache, smoothedTrajectory, particleHypothesis);
  if (!perigeeMultiStateOnSurface.multiComponentState.empty()) {
    smoothedTrajectory.push_back(std::move(perigeeMultiStateOnSurface));
  } else {
    return nullptr;
  }

  // Reverse the order of the TSOS's to make be order flow from inside to out
  std::reverse(smoothedTrajectory.begin(), smoothedTrajectory.end());

  // Create track
  Trk::TrackInfo info(Trk::TrackInfo::GaussianSumFilter, particleHypothesis);
  info.setTrackProperties(TrackInfo::BremFit);
  info.setTrackProperties(TrackInfo::BremFitSuccessful);
  return std::make_unique<Track>(
      info, convertTrajToTrack(smoothedTrajectory),
      std::move(fitQuality));
}

/*
 * Interface method
 * Refit a track adding a PrepRawData  set
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(const EventContext& ctx,
                            const Track& intrk,
                            const PrepRawDataSet& addPrdColl,
                            const RunOutlierRemoval runOutlier,
                            const ParticleHypothesis matEffects) const
{
  // protection, if empty PrepRawDataSet
  if (addPrdColl.empty()) {
    ATH_MSG_WARNING(
      "client tries to add an empty PrepRawDataSet to the track fit.");
    return fit(ctx, intrk, runOutlier, matEffects);
  }
  // determine the Track Parameter which is the start of the trajectory
  const TrackParameters* estimatedStartParameters =
    *(std::min_element(intrk.trackParameters()->begin(),
                       intrk.trackParameters()->end(),
                       m_trkParametersComparisonFunction));

  // use external preparator class to prepare PRD set for fitter interface
  const PrepRawDataSet PRDColl = Trk::TrackFitInputPreparator::stripPrepRawData(
    intrk, addPrdColl, false, true);

  // delegate to fitting PrepRawData interface method
  return fit(ctx, PRDColl, *estimatedStartParameters, runOutlier, matEffects);
}

/*
 *  Interface method
 *  Refit a track adding a RIO_OnTrack set
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(const EventContext& ctx,
                            const Track& inputTrack,
                            const MeasurementSet& measurementSet,
                            const RunOutlierRemoval runOutlier,
                            const ParticleHypothesis matEffects) const
{

  // protection, if empty MeasurementSet
  if (measurementSet.empty()) {
    ATH_MSG_WARNING(
      "Client tries to add an empty MeasurementSet to the track fit.");
    return fit(ctx, inputTrack, runOutlier, matEffects);
  }
  // Check that the input track has well defined parameters
  if (inputTrack.trackParameters()->empty()) {
    ATH_MSG_FATAL("No estimation of track parameters near origin!");
    return nullptr;
  }
  // Check that the input track has associated MeasurementBase objects
  if (inputTrack.trackStateOnSurfaces()->empty()) {
    ATH_MSG_FATAL("Attempting to fit track to empty MeasurementBase "
                  "collection!");
    return nullptr;
  }
  // Retrieve the set of track parameters closest to the reference point
  const Trk::TrackParameters* parametersNearestReference =
    *(std::min_element(inputTrack.trackParameters()->begin(),
                       inputTrack.trackParameters()->end(),
                       m_trkParametersComparisonFunction));

  const MeasurementSet combinedMS = Trk::TrackFitInputPreparator::stripMeasurements(
    inputTrack, measurementSet);

  // delegate to  measurementBase fit method
  return fit(
    ctx, combinedMS, *parametersNearestReference, runOutlier, matEffects);
}

/*
 * Interface method
 * Combine two tracks by refitting
 */
std::unique_ptr<Trk::Track>
Trk::GaussianSumFitter::fit(const EventContext& ctx,
                            const Track& intrk1,
                            const Track& intrk2,
                            const RunOutlierRemoval runOutlier,
                            const ParticleHypothesis matEffects) const
{
  // Just add the hits on tracks
  // protection against not having measurements on the input tracks
  if (!intrk1.trackStateOnSurfaces() || !intrk2.trackStateOnSurfaces() ||
      intrk1.trackStateOnSurfaces()->size() < 2) {
    ATH_MSG_WARNING("called to refit empty track or track with too little "
                    "information, reject fit");
    return nullptr;
  }
  if (!intrk1.trackParameters() || intrk1.trackParameters()->empty()) {
    ATH_MSG_WARNING("input #1 fails to provide track parameters for "
                    "seeding the GXF, reject fit");
    return nullptr;
  }

  const TrackParameters* minPar = *(intrk1.trackParameters()->begin());
  Trk::MeasurementSet ms;
  for (const auto* tsos : *(intrk1.trackStateOnSurfaces())) {
    if (!(tsos->type(Trk::TrackStateOnSurface::Measurement) ||
          tsos->type(Trk::TrackStateOnSurface::Outlier))) {
      continue;
    }

    if (tsos->measurementOnTrack()->type(
          Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
      continue;
    }

    ms.push_back(tsos->measurementOnTrack());
  }

  for (const auto* tsos : *(intrk2.trackStateOnSurfaces())) {

    if (!(tsos->type(Trk::TrackStateOnSurface::Measurement) ||
          tsos->type(Trk::TrackStateOnSurface::Outlier))) {
      continue;
    }

    if (tsos->measurementOnTrack()->type(
          Trk::MeasurementBaseType::PseudoMeasurementOnTrack)) {
      continue;
    }
    ms.push_back(tsos->measurementOnTrack());
  }
  // call measurement base interface
  return fit(ctx, ms, *minPar, runOutlier, matEffects);
}

/*Helper to convert the GSFTrajectory to a Trk::Track */
std::unique_ptr<MultiComponentStateOnSurfaceDV> Trk::GaussianSumFitter::convertTrajToTrack(
    GSFTrajectory& trajectory) const{
  const bool slimTransientMTSOS = m_slimTransientMTSOS;
  auto MTSOS = std::make_unique<MultiComponentStateOnSurfaceDV>();
  MTSOS->reserve(trajectory.size());
  for (GSFTsos& state : trajectory) {
    MTSOS->push_back(state.convert(slimTransientMTSOS));
  }
  return MTSOS;
}

/*
 * Helper creating a multicomponent
 * perigee TrackStateOnSurface
 */
GSFTsos
Trk::GaussianSumFitter::makePerigee(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  const GSFTrajectory& smoothedTrajectory,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  // Start at the end of the smoothed trajectory
  // we should be the closest to perigee/origin.
  const GSFTsos&
    multiComponentStateOnSurfaceNearestOrigin = smoothedTrajectory.back();
  const Trk::MultiComponentState* multiComponentState =
    &(multiComponentStateOnSurfaceNearestOrigin.multiComponentState);

  // Extrapolate to perigee
  const Trk::PerigeeSurface perigeeSurface;
  Trk::MultiComponentState stateExtrapolatedToPerigee =
    m_extrapolator->extrapolate(ctx,
                                extrapolatorCache,
                                *multiComponentState,
                                perigeeSurface,
                                m_directionToPerigee,
                                false,
                                particleHypothesis);

  if (stateExtrapolatedToPerigee.empty()) {
    return {};
  }

  // Determine the combined state
  std::unique_ptr<Trk::TrackParameters> combinedPerigee =
    MultiComponentStateCombiner::combineToSingle(stateExtrapolatedToPerigee, m_useMode);

  // Perigee is an additional MultiComponentStateOnSurface
  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
    pattern(0);
  pattern.set(Trk::TrackStateOnSurface::Perigee);

  if(std::abs(combinedPerigee->position().z())>5000.) {
    ATH_MSG_WARNING("Pathological perigee well outside of tracking detector!! Returning {}");
    return {};
  }

  if (std::abs(combinedPerigee->parameters()[Trk::qOverP]) > 1e8) {
    ATH_MSG_WARNING(
      "makePerigee() about to return with 0 momentum!! Returning {}");
    return {};
  }

  return {Trk::FitQualityOnSurface{},
          nullptr,
          std::move(combinedPerigee),
          std::move(stateExtrapolatedToPerigee),
          pattern};
}

/*
 * Private method for
 * Forward fit on a set of PrepRawData
 */
Trk::GaussianSumFitter::GSFTrajectory
Trk::GaussianSumFitter::forwardPRDfit(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  const Trk::PrepRawDataSet& inputPrepRawDataSet,
  const Trk::TrackParameters& estimatedTrackParametersNearOrigin,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  // Extract PrepRawDataSet into new local object and check that the PrepRawData
  // is associated with a detector element
  Trk::PrepRawDataSet prepRawDataSet;
  for (const auto* rawData : inputPrepRawDataSet) {
    if (!(rawData->detectorElement())) {
      ATH_MSG_WARNING("PrepRawData has no Element link... disregard it");
    } else {
      prepRawDataSet.push_back(rawData);
    }
  }
  // For starting guess a multicompoment state
  // that has single component with weight 1
  const AmgVector(5)& par = estimatedTrackParametersNearOrigin.parameters();
  Trk::ComponentParameters componentParametersNearOrigin = {
    estimatedTrackParametersNearOrigin.associatedSurface()
      .createUniqueTrackParameters(par[Trk::loc1],
                                   par[Trk::loc2],
                                   par[Trk::phi],
                                   par[Trk::theta],
                                   par[Trk::qOverP],
                                   std::nullopt /*no errors*/),
    1.};

  Trk::MultiComponentState multiComponentStateNearOrigin{};
  multiComponentStateNearOrigin.push_back(
    std::move(componentParametersNearOrigin));

  // Create new trajectory
  GSFTrajectory forwardTrajectory{};
  forwardTrajectory.reserve(prepRawDataSet.size());
  for (const auto* prepRawData : prepRawDataSet) {
    // Every step the ForwardTrajectory is updated
    const bool stepIsValid = stepForwardFit(
      ctx,
      extrapolatorCache,
      forwardTrajectory,
      prepRawData,
      nullptr,
      prepRawData->detectorElement()->surface(prepRawData->identify()),
      multiComponentStateNearOrigin,
      particleHypothesis);

    if (!stepIsValid) {
      return GSFTrajectory{};
    }
  }
  return forwardTrajectory;
}

/*
 * Private method for
 * forward fit on a set of Measurements
 */
Trk::GaussianSumFitter::GSFTrajectory
Trk::GaussianSumFitter::forwardMeasurementFit(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  const Trk::MeasurementSet& inputMeasurementSet,
  const Trk::TrackParameters& estimatedTrackParametersNearOrigin,
  const Trk::ParticleHypothesis particleHypothesis) const
{

  if (inputMeasurementSet.empty()) {
    ATH_MSG_ERROR("forwardMeasurementFit: Input MeasurementSet is empty!");
    return GSFTrajectory{};
  }
  // For starting guess a multicompoment state
  // that has single component with weight 1
  const AmgVector(5)& par = estimatedTrackParametersNearOrigin.parameters();
  Trk::ComponentParameters componentParametersNearOrigin = {
    estimatedTrackParametersNearOrigin.associatedSurface()
      .createUniqueTrackParameters(par[Trk::loc1],
                                   par[Trk::loc2],
                                   par[Trk::phi],
                                   par[Trk::theta],
                                   par[Trk::qOverP],
                                   std::nullopt /*no errors*/),
    1.};
  Trk::MultiComponentState multiComponentStateNearOrigin{};
  multiComponentStateNearOrigin.push_back(
    std::move(componentParametersNearOrigin));

  GSFTrajectory forwardTrajectory{};
  forwardTrajectory.reserve(inputMeasurementSet.size());
  for (const auto* measurement : inputMeasurementSet) {
    // Every step the ForwardTrajectory is updated
    const bool stepIsValid = stepForwardFit(ctx,
                                      extrapolatorCache,
                                      forwardTrajectory,
                                      nullptr,
                                      measurement,
                                      measurement->associatedSurface(),
                                      multiComponentStateNearOrigin,
                                      particleHypothesis);

    if (!stepIsValid) {
      return GSFTrajectory{};
    }
  }
  return forwardTrajectory;
}

/*
 * Actual
 * Implementation method for StepForwardFit
 */
bool
Trk::GaussianSumFitter::stepForwardFit(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  GSFTrajectory& forwardTrajectory,
  const Trk::PrepRawData* originalPrepRawData,
  const Trk::MeasurementBase* originalMeasurement,
  const Trk::Surface& surface,
  Trk::MultiComponentState& updatedState,
  const Trk::ParticleHypothesis particleHypothesis) const
{
  // Protect against undefined Measurement or PrepRawData
  if (!originalPrepRawData && !originalMeasurement) {
    ATH_MSG_WARNING("No measurement base or PrepRawData passed to "
                    "StepForwardFit!");
    return false;
  }

  if (!originalMeasurement && m_refitOnMeasurementBase) {
    ATH_MSG_WARNING("No measurement base information passed to StepForwardFit");
    return false;
  }
  // Propagate the multi-component state to the next measurement surface
  // accounting for the material effects. This gives us
  // the prediction at that surface.
  Trk::MultiComponentState extrapolatedState =
    m_extrapolator->extrapolate(ctx,
                                extrapolatorCache,
                                updatedState,
                                surface,
                                Trk::alongMomentum,
                                false,
                                particleHypothesis);
  if (extrapolatedState.empty()) {
    return false;
  }

  // we need to account for either measurement base input
  // or PrepRawData input.
  std::unique_ptr<Trk::MeasurementBase> measurement = nullptr;
  if (originalMeasurement) { // clone original MeasurementBase object
    measurement.reset(originalMeasurement->clone());
  } else {
    std::unique_ptr<Trk::TrackParameters> combinedState =
      MultiComponentStateCombiner::combineToSingle(extrapolatedState);
    if (!combinedState) {
      ATH_MSG_WARNING("State combination failed... exiting");
      return false;
    }
    // Create a new MeasurementBase object from PrepRawData
    measurement.reset(
      m_rioOnTrackCreator->correct(*originalPrepRawData, *combinedState, ctx));
    combinedState.reset();
  }
  if (!measurement) {
    ATH_MSG_WARNING("stepForwardFit no measurement to update with");
    return false;
  }

  // Perform measurement update
  auto fitQuality = Trk::FitQualityOnSurface{};
  // We need to keep the extrapolatedState so clone
  updatedState = Trk::GsfMeasurementUpdator::update(
    MultiComponentStateHelpers::clone(extrapolatedState),
    *measurement,
    fitQuality);

  if (updatedState.empty()) {
    return false;
  }

  // Hits with excessive chi2 are outliers.
  // We ingore the update, reset back to the extrapolated
  // state before the update
  if (fitQuality.chiSquared() >
      m_cutChiSquaredPerNumberDOF * fitQuality.numberDoF()) {
    fitQuality = FitQualityOnSurface(1, 1);
    std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> type(0);
    type.set(TrackStateOnSurface::Outlier);
    forwardTrajectory.emplace_back(
        fitQuality,
        std::move(measurement),
        nullptr,
        // used below for the updated state so clone
        Trk::MultiComponentStateHelpers::clone(extrapolatedState),
        type);
    // reset the updated state to the extrapolated state
    // before the measurement update
    updatedState = std::move(extrapolatedState);
  } else {
    forwardTrajectory.emplace_back(fitQuality,
                std::move(measurement),
                nullptr,
                std::move(extrapolatedState));
  }
  return true;
}

/*
 * Actual
 * Implementation of the smoothing of the trajectory.
 */
Trk::GaussianSumFitter::GSFTrajectory
Trk::GaussianSumFitter::smootherFit(
  const EventContext& ctx,
  Trk::IMultiStateExtrapolator::Cache& extrapolatorCache,
  GSFTrajectory& forwardTrajectory,
  const ParticleHypothesis particleHypothesis,
  const Trk::CaloCluster_OnTrack* ccot) const
{
  if (forwardTrajectory.empty()) {
    ATH_MSG_ERROR(
      "Attempting to smooth an empty forward trajectory... Exiting!");
    return {};
  }

  GSFTrajectory smoothedTrajectory;
  smoothedTrajectory.reserve(forwardTrajectory.size());
  // For the smoother we start from the end and we go
  // to begin. We need to find the the first track
  // state on surface  in this reverse direction
  GSFTrajectory::reverse_iterator trackStateOnSurfaceItr =
    forwardTrajectory.rbegin();
  bool foundMeasurement = false;
  for (; trackStateOnSurfaceItr != forwardTrajectory.rend();
       ++trackStateOnSurfaceItr) {
    if (!(*trackStateOnSurfaceItr).typeFlags.test(TrackStateOnSurface::Measurement)) {
      smoothedTrajectory.emplace_back(std::move(*trackStateOnSurfaceItr));
    } else {
      foundMeasurement = true;
      break;
    }
  }

  if(!foundMeasurement){
    return {};
  }
  // This is the 1st track state on surface for a measurement
  // in the reverse direction. Our starting point for the smoother
  MultiComponentState smootherPredictionMultiState =
      std::move(trackStateOnSurfaceItr->multiComponentState);
  // Perform the  update with the measurement and create the
  // the 1st updated/smoothed entry in the trajectory
  std::unique_ptr<Trk::MeasurementBase> firstSmootherMeasurementOnTrack =
      std::move(trackStateOnSurfaceItr->measurementOnTrack);
  if (!firstSmootherMeasurementOnTrack) {
    ATH_MSG_WARNING(
      "Initial state on surface in smoother does not have an associated "
      "MeasurementBase object");
    return {};
  }
  Trk::FitQualityOnSurface fitQuality;
  Trk::MultiComponentState firstSmoothedState =
    Trk::GsfMeasurementUpdator::update(std::move(smootherPredictionMultiState),
                                       *firstSmootherMeasurementOnTrack,
                                       fitQuality);
  if (firstSmoothedState.empty()) {
    return {};
  }

  if (!MultiComponentStateHelpers::allHaveCovariance(firstSmoothedState)) {
    ATH_MSG_WARNING(
      "Not all components have covariance. Rejecting smoothed state.");
    return {};
  }
  // The first in reverse (last in normal order)  TSOS is special so we do a proper collapse
  // of the multi component to single TrackParameter
  std::unique_ptr<Trk::TrackParameters> combinedFirstSmoothedState =
    MultiComponentStateCombiner::combineToSingle(firstSmoothedState, m_useMode);
  smoothedTrajectory.emplace_back(
      fitQuality, std::move(firstSmootherMeasurementOnTrack),
      std::move(combinedFirstSmoothedState),
      MultiComponentStateHelpers::clone(firstSmoothedState));
  const auto& updatedFirstStateOnSurface = smoothedTrajectory.back();

  // continue the reverse looping of the TrackStateOnSurfaces
  // in the forward trajectory
  ++trackStateOnSurfaceItr;
  // The is the last one we will see
  auto lasttrackStateOnSurface = forwardTrajectory.rend() - 1;
  // TSOS that the cluster measuremenet will added on.
  auto secondLastTrackStateOnSurface = forwardTrajectory.rend() - 2;
  // Generate a prediction by scaling the covariance of all components in the
  // first smoothed state and perform a measurement update to it.
  // This way there is no dependance on error of prediction
  // NB local Y and theta are not blown out too much to help in the TRT.
  Trk::MultiComponentState smoothedStateWithScaledError =
    MultiComponentStateHelpers::WithScaledError(
      std::move(firstSmoothedState), 15., 5., 15., 5., 15.);
  Trk::FitQualityOnSurface fitQualityWithScaledErrors;
  Trk::MultiComponentState updatedState = Trk::GsfMeasurementUpdator::update(
    std::move(smoothedStateWithScaledError),
    *(updatedFirstStateOnSurface.measurementOnTrack),
    fitQualityWithScaledErrors);
  if (updatedState.empty()) {
    ATH_MSG_WARNING("Smoother prediction could not be determined");
    return {};
  }

  // loopUpdatedState is a plain ptr to the most recent
  // predicted MultiComponentState.
  // We start from our previous inflated prediction
  Trk::MultiComponentState* loopUpdatedState = &updatedState;

  for (; trackStateOnSurfaceItr != forwardTrajectory.rend();
       ++trackStateOnSurfaceItr) {
    auto& trackStateOnSurface = (*trackStateOnSurfaceItr);
    // Retrieve the MeasurementBase object from the TrackStateOnSurface object
    std::unique_ptr<Trk::MeasurementBase> measurement =
        std::move(trackStateOnSurface.measurementOnTrack);
    if (!measurement) {
      ATH_MSG_WARNING("MeasurementBase object could not be extracted from a "
                      "measurement TSOS... continuing");
      continue;
    }
    // Create prediction for the next measurement surface.
    // For the smoother the direction of propagation
    // is opposite to the direction of momentum
    Trk::MultiComponentState extrapolatedState =
      m_extrapolator->extrapolate(ctx,
                                  extrapolatorCache,
                                  (*loopUpdatedState),
                                  measurement->associatedSurface(),
                                  Trk::oppositeMomentum,
                                  false,
                                  particleHypothesis);

    if (extrapolatedState.empty()) {
      return {};
    }
    // Handle the case where Original measurement was flagged as an outlier
    // and not used
    if (!trackStateOnSurface.typeFlags.test(TrackStateOnSurface::Measurement)) {
      std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> type(0);
      type.set(TrackStateOnSurface::Outlier);
      smoothedTrajectory.emplace_back(
          FitQualityOnSurface(1, 1),
          std::move(measurement),
          nullptr,
          std::move(extrapolatedState),
          type);
      loopUpdatedState = &(smoothedTrajectory.back().multiComponentState);
      continue;
    }
    // Update with the measurement
    updatedState = Trk::GsfMeasurementUpdator::update(std::move(extrapolatedState), *measurement, fitQuality);
    if (updatedState.empty()) {
      ATH_MSG_WARNING("Could not update the multi-component state");
      return {};
    }
    // last in reverse (first in normal order) is special as we collapse to single track Parameters
    const bool islast = (trackStateOnSurfaceItr == lasttrackStateOnSurface);
    if (m_combineWithFitter) {
      // Optional combine smoother state with fitter state
      // e.g combine the current tsos (from the forward) with
      // the updated from the smoother
      const Trk::MultiComponentState& forwardsMultiState = trackStateOnSurface.multiComponentState;
      Trk::MultiComponentState combinedfitterState = Trk::MultiComponentStateCombiner::combineWithSmoother(
        forwardsMultiState, updatedState, m_maximumNumberOfComponents);
      //
      if (combinedfitterState.empty()) {
        ATH_MSG_WARNING("Could not combine state from forward fit with "
                        "smoother state");
        return {};
      }
      auto combinedFitQuality = Trk::GsfMeasurementUpdator::fitQuality(combinedfitterState, *measurement);
      smoothedTrajectory.emplace_back(smootherHelper(std::move(combinedfitterState), std::move(measurement),
                                                     combinedFitQuality, islast, m_useMode));
    } else {
      // If combination with forwards state is not done
      smoothedTrajectory.emplace_back(smootherHelper(std::move(updatedState), std::move(measurement),
                                                     fitQuality, islast, m_useMode));
    }
    // Handle adding measurement from calo if it is present
    if (ccot && trackStateOnSurfaceItr == secondLastTrackStateOnSurface) {
      if (!addCCOT(ctx, ccot, smoothedTrajectory)) {
        ATH_MSG_WARNING("Could not add Calo Cluster On Track Measurement");
        return {};
      }
    }
    // For the next iteration start from last added
    loopUpdatedState = &(smoothedTrajectory.back().multiComponentState);
  } // End for loop over all components
  return smoothedTrajectory;
}

/*
 * Account for additional measurement from
 * the calorimeter
 */
bool
Trk::GaussianSumFitter::addCCOT(
  const EventContext& ctx,
  const Trk::CaloCluster_OnTrack* ccot,
  GSFTrajectory& smoothedTrajectory) const
{
  const GSFTsos& currentMultiStateOS = smoothedTrajectory.back();
  if (!ccot) {
    return false;
  }
  const auto& currentMultiComponentState = currentMultiStateOS.multiComponentState;
  const Trk::MeasurementBase* measurement = currentMultiStateOS.measurementOnTrack.get();
  if (!measurement) {
     return false;
  }
  const Trk::Surface& currentSurface = measurement->associatedSurface();

  //Create a ccot that we will own. So we use it to build our own TSOS
  auto ownCCOT = ccot->uniqueClone();
  // Extrapolate to the Calo to get prediction
  Trk::MultiComponentState extrapolatedToCaloState = m_extrapolator->extrapolateDirectly(
    ctx, currentMultiComponentState, ownCCOT->associatedSurface(),
    Trk::alongMomentum, false, Trk::nonInteracting);

  if (extrapolatedToCaloState.empty()) {
    return false;
  }
  // Update newly extrapolated state with measurement
  Trk::FitQualityOnSurface fitQuality;
  Trk::MultiComponentState updatedStateAtCalo =
      Trk::GsfMeasurementUpdator::update(std::move(extrapolatedToCaloState),
                                         *ownCCOT, fitQuality);
  if (updatedStateAtCalo.empty()) {
    return false;
  }

  // Extrapolate back to the surface near the origin
  auto improvedState = m_extrapolator->extrapolateDirectly(
      ctx, updatedStateAtCalo, currentSurface, Trk::oppositeMomentum,
      false, Trk::nonInteracting);

  if (improvedState.empty()) {
    return false;
  }
 // Combine the improved state after extrapolating back from the calo
 // and find the mode of the distribution
  std::unique_ptr<Trk::TrackParameters> combinedSingleState =
      MultiComponentStateCombiner::combineToSingle(improvedState, m_useMode);

  // Now build a dummy measurement for the improved estimation
  AmgSymMatrix(5) covMatrix;
  covMatrix.setZero();
  covMatrix(0, 0) = 1e6;
  const Trk::DefinedParameter locX(0, Trk::locX);
  Trk::LocalParameters locpars(locX);
  auto pseudoMeasurement = std::make_unique<Trk::PseudoMeasurementOnTrack>(
    std::move(locpars), std::move(covMatrix), currentSurface);

  // Build TSOS at the surface of calo
  smoothedTrajectory.emplace_back(
      fitQuality,
      std::move(ownCCOT),
      nullptr,
      std::move(updatedStateAtCalo));

  // Build a TSOS using the dummy measurement and and the final combined state
  smoothedTrajectory.emplace_back(
      FitQualityOnSurface{},  // We do not add the fitquality again. As this would be the one we
           // add at calo, here not a real measurement
      std::move(pseudoMeasurement),
      std::move(combinedSingleState),
      std::move(improvedState));

  return true;
}
