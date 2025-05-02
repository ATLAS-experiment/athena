/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalChiSquareFitterTool.h"

// ATHENA
#include "Acts/EventData/Types.hpp"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/TypeNameString.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrackSummary/TrackSummary.h"

// ACTS
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/Types.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/SympyStepper.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFitting/GlobalChiSquareFitter.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetPrepRawData/SCT_Cluster.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ATLASSourceLinkSurfaceAccessor.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsInterop/Logger.h"

// STL
#include <vector>

namespace ActsTrk {

/**
 * The following is the implementation of the PRDSourceLinkCalibratorGX2F. To
 * use this implementation, the flag for a refitting from PRD measurement has to
 * be turned on in the appropriate Acts config.
 */
template <typename trajectory_t>
void PRDSourceLinkCalibratorGX2F::calibrate(
    const Acts::GeometryContext& gctx, const Acts::CalibrationContext& /*cctx*/,
    const Acts::SourceLink& sl,
    typename trajectory_t::TrackStateProxy trackState) const {

  const Trk::PrepRawData* prd = sl.template get<PRDSourceLinkGX2F>().prd;
  trackState.setUncalibratedSourceLink(Acts::SourceLink{sl});

  const Acts::BoundTrackParameters actsParam(
      trackState.referenceSurface().getSharedPtr(), trackState.predicted(),
      trackState.predictedCovariance(), Acts::ParticleHypothesis::pion());
  std::unique_ptr<const Trk::TrackParameters> trkParam =
      converterTool->actsTrackParametersToTrkParameters(actsParam, gctx);

  // RIO_OnTrack creation from the PrepRawData
  const Trk::Surface& prdsurf =
      prd->detectorElement()->surface(prd->identify());
  const Trk::RIO_OnTrack* rot = nullptr;
  const Trk::PlaneSurface* plsurf = nullptr;
  if (prdsurf.type() == Trk::SurfaceType::Plane)
    plsurf = static_cast<const Trk::PlaneSurface*>(&prdsurf);

  const Trk::StraightLineSurface* slsurf = nullptr;

  if (prdsurf.type() == Trk::SurfaceType::Line)
    slsurf = static_cast<const Trk::StraightLineSurface*>(&prdsurf);

  //@TODO, there is no way to put a MSG in this framework yet. So the next 5
  // lines are commented
  // if ((slsurf == nullptr) && (plsurf == nullptr)) {
  //   msg(MSG::ERROR) << "Surface is neither PlaneSurface nor
  //   StraightLineSurface!" << endmsg;
  //   // ATH_MSG_ERROR doesn't work here because
  //   // (ActsTrk::PRDSourceLinkCalibratorGX2F' has no member named 'msg')
  //  }
  if (slsurf != nullptr) {
    Trk::AtaStraightLine atasl(slsurf->center(),
                               trackState.predicted()[Trk::phi],
                               trackState.predicted()[Trk::theta],
                               trackState.predicted()[Trk::qOverP], *slsurf);
    if (broadRotCreator) {
      rot =
          broadRotCreator->correct(*prd, atasl, Gaudi::Hive::currentContext());
    } else {
      rot = rotCreator->correct(*prd, atasl, Gaudi::Hive::currentContext());
    }
  } else if (plsurf != nullptr) {
    if ((trkParam.get())->covariance() != nullptr) {
      Trk::AtaPlane atapl(plsurf->center(), trackState.predicted()[Trk::phi],
                          trackState.predicted()[Trk::theta],
                          trackState.predicted()[Trk::qOverP], *plsurf,
                          AmgSymMatrix(5)(*trkParam.get()->covariance()));
      rot = rotCreator->correct(*prd, atapl, Gaudi::Hive::currentContext());
    } else {
      Trk::AtaPlane atapl(plsurf->center(), trackState.predicted()[Trk::phi],
                          trackState.predicted()[Trk::theta],
                          trackState.predicted()[Trk::qOverP], *plsurf);
      rot = rotCreator->correct(*prd, atapl, Gaudi::Hive::currentContext());
    }
  }  // End of RIO_OnTrack creation from the PrepRawData

  int dim = (*rot).localParameters().dimension();
  trackState.allocateCalibrated(dim);

  // Writing the calibrated ROT measurement into the trackState
  if (dim == 0) {
    throw std::runtime_error("Cannot create dim 0 measurement");
  } else if (dim == 1) {
    trackState.template calibrated<1>() =
        (*rot).localParameters().template head<1>();
    trackState.template calibratedCovariance<1>() =
        (*rot).localCovariance().template topLeftCorner<1, 1>();
    Acts::BoundSubspaceIndices subspaceIndices;
    if ((*rot).associatedSurface().bounds().type() ==
        Trk::SurfaceBounds::Annulus) {
      subspaceIndices = {Acts::eBoundLoc1};  // y coordinate is l0
    } else {
      subspaceIndices = {Acts::eBoundLoc0};  // x coordinate is l0
    }
    trackState.setProjectorSubspaceIndices(subspaceIndices);
  } else if (dim == 2) {
    trackState.template calibrated<2>() =
        (*rot).localParameters().template head<2>();
    trackState.template calibratedCovariance<2>() =
        (*rot).localCovariance().template topLeftCorner<2, 2>();
    Acts::BoundSubspaceIndices subspaceIndices = {Acts::eBoundLoc0,
                                                  Acts::eBoundLoc1};
    trackState.setProjectorSubspaceIndices(subspaceIndices);
  } else {
    throw std::runtime_error("Dim " + std::to_string(dim) +
                             " currently not supported.");
  }
  delete rot;  // Delete rot as a precaution
}

const Acts::Surface* PRDSourceLinkSurfaceAccessorGX2F::operator()(
    const Acts::SourceLink& sourceLink) const {
  const auto& sl = sourceLink.get<PRDSourceLinkGX2F>();
  const auto& trkSrf = sl.prd->detectorElement()->surface(sl.prd->identify());
  const auto& actsSrf = converterTool->trkSurfaceToActsSurface(trkSrf);
  return &actsSrf;
}

GlobalChiSquareFitterTool::GlobalChiSquareFitterTool(const std::string& t,
                                                     const std::string& n,
                                                     const IInterface* p)
    : base_class(t, n, p) {}

StatusCode GlobalChiSquareFitterTool::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_trkSummaryTool.retrieve());
  if (m_doReFitFromPRD) {
    ATH_CHECK(m_ROTcreator.retrieve());
    ATH_CHECK(m_broadROTcreator.retrieve());
  }

  m_logger = makeActsAthenaLogger(this, "Gx2fRefit");

  auto field = std::make_shared<ATLASMagneticFieldWrapper>();

  // Fitter
  Acts::SympyStepper stepper(field);
  Acts::Navigator navigator(
      Acts::Navigator::Config{m_trackingGeometryTool->trackingGeometry()},
      logger().cloneWithSuffix("Navigator"));
  Acts::Propagator<Acts::SympyStepper, Acts::Navigator> propagator(
      stepper, std::move(navigator), logger().cloneWithSuffix("Prop"));

  m_fitter = std::make_unique<Fitter>(
      std::move(propagator), logger().cloneWithSuffix("GlobalChiSquareFitter"));

  m_calibrator = std::make_unique<ActsTrk::detail::TrkMeasurementCalibrator>(
      *m_ATLASConverterTool);
  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;

  m_gx2fExtensions.outlierFinder
      .connect<&ActsTrk::detail::FitterHelperFunctions::ATLASOutlierFinder::
               operator()<ActsTrk::MutableTrackStateBackend>>(&m_outlierFinder);
  m_gx2fExtensions.updater
      .connect<&ActsTrk::detail::FitterHelperFunctions::gainMatrixUpdate<
          ActsTrk::MutableTrackStateBackend>>();
  m_gx2fExtensions.calibrator
      .connect<&ActsTrk::detail::TrkMeasurementCalibrator::calibrate<
          ActsTrk::MutableTrackStateBackend>>(m_calibrator.get());

  return StatusCode::SUCCESS;
}

// refit a track
// -------------------------------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const Trk::Track& inputTrack,
    const Trk::RunOutlierRemoval /*runOutlier*/,
    const Trk::ParticleHypothesis hypothesis) const {
  std::unique_ptr<Trk::Track> track = nullptr;
  ATH_MSG_VERBOSE(
      "--> enter GlobalChiSquareFitterTool::fit(Track,,) with Track from "
      "author = "
      << inputTrack.info().dumpInfo());

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() ||
      inputTrack.measurementsOnTrack()->size() < 2) {
    ATH_MSG_DEBUG(
        "called to refit empty track or track with too little information, "
        "reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG(
        "input fails to provide track parameters for seeding the GX2F, reject "
        "fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  Acts::GeometryContext tgContext =
      m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext =
      m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::Experimental::Gx2FitterExtensions<ActsTrk::MutableTrackStateBackend>
      gx2fExtensions = m_gx2fExtensions;

  ATLASSourceLinkSurfaceAccessor surfaceAccessor{&(*m_ATLASConverterTool)};
  gx2fExtensions.surfaceAccessor
      .connect<&ATLASSourceLinkSurfaceAccessor::operator()>(&surfaceAccessor);

  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the Gx2Fitter options
  Acts::Experimental::Gx2FitterOptions gx2fOptions(
      tgContext, mfContext, calContext, gx2fExtensions, propagationOption,
      pSurface.get(), true, true);

  std::vector<Acts::SourceLink> trackSourceLinks =
      m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, inputTrack);
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG(
        "input contain measurement but no source link created, probable issue "
        "with the converter, reject fit ");
    return track;
  }

  const auto initialParams =
      m_ATLASConverterTool->trkTrackParametersToActsParameters(
          (*inputTrack.perigeeParameters()), tgContext);

  
  const Acts::BoundTrackParameters initialParamsWithHypothesis(
      initialParams.referenceSurface().getSharedPtr(),
      initialParams.parameters(), initialParams.covariance(), 
      ParticleHypothesis::convert(hypothesis));

  ActsTrk::MutableTrackContainer tracks;

  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParamsWithHypothesis, gx2fOptions, tracks);
  if (result.ok()) {
    track = makeTrack(ctx, tgContext, tracks, result);
  }
  return track;
}

// fit a set of MeasurementBase objects
// --------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const Trk::MeasurementSet& inputMeasSet,
    const Trk::TrackParameters& estimatedStartParameters,
    const Trk::RunOutlierRemoval /*runOutlier*/,
    const Trk::ParticleHypothesis /*matEffects*/) const {
  std::unique_ptr<Trk::Track> track = nullptr;

  // protection against not having measurements on the input track
  if (inputMeasSet.size() < 2) {
    ATH_MSG_DEBUG(
        "called to refit empty measurement set or a measurement set with too "
        "little information, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  Acts::GeometryContext tgContext =
      m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext =
      m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::Experimental::Gx2FitterExtensions<ActsTrk::MutableTrackStateBackend>
      gx2fExtensions = m_gx2fExtensions;

  ATLASSourceLinkSurfaceAccessor surfaceAccessor{&(*m_ATLASConverterTool)};
  gx2fExtensions.surfaceAccessor
      .connect<&ATLASSourceLinkSurfaceAccessor::operator()>(&surfaceAccessor);

  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the Gx2Fitter options
  Acts::Experimental::Gx2FitterOptions gx2fOptions(
      tgContext, mfContext, calContext, gx2fExtensions, propagationOption,
      pSurface.get(), true, true);

  std::vector<Acts::SourceLink> trackSourceLinks;
  trackSourceLinks.reserve(inputMeasSet.size());

  for (auto it = inputMeasSet.begin(); it != inputMeasSet.end(); ++it) {
    trackSourceLinks.push_back(
        m_ATLASConverterTool->trkMeasurementToSourceLink(tgContext, **it));
  }
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG(
        "input contain measurement but no source link created, probable issue "
        "with the converter, reject fit ");
    return track;
  }

  const auto initialParams =
      m_ATLASConverterTool->trkTrackParametersToActsParameters(
          estimatedStartParameters, tgContext);

  ActsTrk::MutableTrackContainer tracks;

  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParams, gx2fOptions, tracks);
  if (result.ok()) {
    track = makeTrack(ctx, tgContext, tracks, result);
  }
  return track;
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const Trk::PrepRawDataSet& inputPRDColl,
    const Trk::TrackParameters& estimatedStartParameters,
    const Trk::RunOutlierRemoval /*runOutlier*/,
    const Trk::ParticleHypothesis /*prtHypothesis*/) const {
  ATH_MSG_DEBUG("--> entering GlobalChiSquareFitterTool::fit(PRDS,TP,)");

  std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  Acts::GeometryContext tgContext =
      m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext =
      m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::Experimental::Gx2FitterExtensions<ActsTrk::MutableTrackStateBackend>
      gx2fExtensions = m_gx2fExtensions;

  PRDSourceLinkCalibratorGX2F calibrator{};  // @TODO: Set tool pointers
  calibrator.rotCreator = m_ROTcreator.get();
  calibrator.broadRotCreator = m_broadROTcreator.get();
  calibrator.converterTool = m_ATLASConverterTool.get();
  gx2fExtensions.calibrator.connect<&PRDSourceLinkCalibratorGX2F::calibrate<
      ActsTrk::MutableTrackStateBackend>>(&calibrator);

  PRDSourceLinkSurfaceAccessorGX2F surfaceAccessor{m_ATLASConverterTool.get()};
  gx2fExtensions.surfaceAccessor
      .connect<&PRDSourceLinkSurfaceAccessorGX2F::operator()>(&surfaceAccessor);

  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the Gx2Fitter options
  Acts::Experimental::Gx2FitterOptions gx2fOptions(
      tgContext, mfContext, calContext, gx2fExtensions, propagationOption,
      pSurface.get(), true, true);

  std::vector<Acts::SourceLink> trackSourceLinks;
  trackSourceLinks.reserve(inputPRDColl.size());

  for (const Trk::PrepRawData* prd : inputPRDColl) {
    trackSourceLinks.push_back(Acts::SourceLink{PRDSourceLinkGX2F{prd}});
  }
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_WARNING(
        "input contain measurement but no source link created, probable issue "
        "with the converter, reject fit ");
    return track;
  }

  const auto initialParams =
      m_ATLASConverterTool->trkTrackParametersToActsParameters(
          estimatedStartParameters, tgContext);

  ActsTrk::MutableTrackContainer tracks;
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParams, gx2fOptions, tracks);
  if (result.ok()) {
    track = makeTrack(ctx, tgContext, tracks, result, true);
  }
  return track;
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<ActsTrk::MutableTrackContainer> GlobalChiSquareFitterTool::fit(
    const EventContext& /*eventContext*/,
    const std::vector<ActsTrk::ATLASUncalibSourceLink>& /*clusterList*/,
    const Acts::BoundTrackParameters& /*initialParams*/,
    const Acts::GeometryContext& /*tgContext*/,
    const Acts::MagneticFieldContext& /*mfContext*/,
    const Acts::CalibrationContext& /*calContext*/,
    const DetectorElementToActsGeometryIdMap& /*detectorElementToGeometryIdMap*/,
    const Acts::Surface* /*targetSurface*/) const {
  ATH_MSG_ERROR("The ACTS Global Chi Square Fitter has no direct fitter.");
  return nullptr;
}

// extend a track fit to include an additional set of MeasurementBase objects
// re-implements the TrkFitterUtils/TrackFitter.cxx general code in a more
// mem efficient and stable way
// --------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const Trk::Track& inputTrack,
    const Trk::MeasurementSet& addMeasColl,
    const Trk::RunOutlierRemoval /*runOutlier*/,
    const Trk::ParticleHypothesis /*matEffects*/) const {
  ATH_MSG_VERBOSE(
      "--> enter GlobalChiSquareFitterTool::fit(Track,Meas'BaseSet,,)");
  ATH_MSG_VERBOSE(
      "    with Track from author = " << inputTrack.info().dumpInfo());

  // protection, if empty MeasurementSet
  if (addMeasColl.empty()) {
    ATH_MSG_DEBUG(
        "client tries to add an empty MeasurementSet to the track fit.");
    return fit(ctx, inputTrack);
  }

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() ||
      (inputTrack.measurementsOnTrack()->size() < 2 && addMeasColl.empty())) {
    ATH_MSG_DEBUG(
        "called to refit empty track or track with too little information, "
        "reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG(
        "input fails to provide track parameters for seeding the GX2F, reject "
        "fit");
    return nullptr;
  }

  std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  Acts::GeometryContext tgContext =
      m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext =
      m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::Experimental::Gx2FitterExtensions<ActsTrk::MutableTrackStateBackend>
      gx2fExtensions = m_gx2fExtensions;

  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the Gx2Fitter options
  Acts::Experimental::Gx2FitterOptions gx2fOptions(
      tgContext, mfContext, calContext, gx2fExtensions, propagationOption,
      pSurface.get(), true, true);

  std::vector<Acts::SourceLink> trackSourceLinks =
      m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, inputTrack);
  const auto initialParams =
      m_ATLASConverterTool->trkTrackParametersToActsParameters(
          *(inputTrack.perigeeParameters()), tgContext);

  for (auto it = addMeasColl.begin(); it != addMeasColl.end(); ++it) {
    trackSourceLinks.push_back(
        m_ATLASConverterTool->trkMeasurementToSourceLink(tgContext, **it));
  }
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG(
        "input contain measurement but no source link created, probable issue "
        "with the converter, reject fit ");
    return track;
  }

  ActsTrk::MutableTrackContainer tracks;
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParams, gx2fOptions, tracks);
  if (result.ok()) {
    track = makeTrack(ctx, tgContext, tracks, result);
  }
  return track;
}

// extend a track fit to include an additional set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(
    const EventContext& /*ctx*/, const Trk::Track& /*inputTrack*/,
    const Trk::PrepRawDataSet& /*addPrdColl*/,
    const Trk::RunOutlierRemoval /*runOutlier*/,
    const Trk::ParticleHypothesis /*matEffects*/) const {
  ATH_MSG_DEBUG(
      "Fit of Track with additional PrepRawDataSet not yet implemented");
  return nullptr;
}

// combined fit of two tracks
// --------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const Trk::Track& intrk1, const Trk::Track& intrk2,
    const Trk::RunOutlierRemoval /*runOutlier*/,
    const Trk::ParticleHypothesis  matEffects) const {
  ATH_MSG_VERBOSE("--> enter GlobalChiSquareFitterTool::fit(Track,Track,)");
  ATH_MSG_VERBOSE("    with Tracks from #1 = " << intrk1.info().dumpInfo()
                                               << " and #2 = "
                                               << intrk2.info().dumpInfo());

  // protection, if empty track2
  if (!intrk2.measurementsOnTrack()) {
    ATH_MSG_DEBUG("input #2 is empty try to fit track 1 alone");
    return fit(ctx, intrk1);
  }

  // protection, if empty track1
  if (!intrk1.measurementsOnTrack()) {
    ATH_MSG_DEBUG("input #1 is empty try to fit track 2 alone");
    return fit(ctx, intrk2);
  }

  // protection against not having track parameters on the input track
  if (!intrk1.trackParameters() || intrk1.trackParameters()->empty()) {
    ATH_MSG_DEBUG(
        "input #1 fails to provide track parameters for seeding the GX2F, "
        "reject fit");
    return nullptr;
  }

  std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  Acts::GeometryContext tgContext =
      m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext =
      m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  Acts::Experimental::Gx2FitterExtensions<ActsTrk::MutableTrackStateBackend>
      gx2fExtensions = m_gx2fExtensions;

  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the Gx2Fitter options
  Acts::Experimental::Gx2FitterOptions gx2fOptions(
      tgContext, mfContext, calContext, gx2fExtensions, propagationOption,
      pSurface.get(), true, true);

  std::vector<Acts::SourceLink> trackSourceLinks =
      m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, intrk1);
  std::vector<Acts::SourceLink> trackSourceLinks2 =
      m_ATLASConverterTool->trkTrackToSourceLinks(tgContext, intrk2);
  trackSourceLinks.insert(trackSourceLinks.end(), trackSourceLinks2.begin(),
                          trackSourceLinks2.end());
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG(
        "input contain measurement but no source link created, probable issue "
        "with the converter, reject fit ");
    return track;
  }

  const auto initialParams =
      m_ATLASConverterTool->trkTrackParametersToActsParameters(
          *(intrk1.perigeeParameters()), tgContext);


  const Acts::BoundTrackParameters initialParamsWithHypothesis(
      initialParams.referenceSurface().getSharedPtr(),
      initialParams.parameters(), initialParams.covariance(), 
      ParticleHypothesis::convert(matEffects));

  ActsTrk::MutableTrackContainer tracks;
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParamsWithHypothesis, gx2fOptions, tracks);
  if (result.ok()) {
    track = makeTrack(ctx, tgContext, tracks, result);
  }
  return track;
}

std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::makeTrack(
    const EventContext& ctx, Acts::GeometryContext& tgContext,
    ActsTrk::MutableTrackContainer& tracks,
    Acts::Result<ActsTrk::MutableTrackContainer::TrackProxy, std::error_code>&
        fitResult,
    bool SourceLinkType) const {

  if (not fitResult.ok()) {
    return nullptr;
  }

  std::unique_ptr<Trk::Track> newtrack = nullptr;

  // Get the fit output object
  const auto& acts_track = fitResult.value();
  auto finalTrajectory = std::make_unique<Trk::TrackStates>();

  // initialise the number of dead Pixel and ACTS strip
  int numberOfDeadPixel = 0;
  int numberOfDeadSCT = 0;

  std::vector<std::unique_ptr<const Acts::BoundTrackParameters>>
      actsSmoothedParam;
  // Loop over all the output state to create track state
  tracks.trackStateContainer().visitBackwards(
      acts_track.tipIndex(), [&](const auto& state) -> void {
        // First only consider states with an associated detector element not in
        // the TRT
        auto flag = state.typeFlags();
        const auto* associatedDetEl =
            state.referenceSurface().associatedDetectorElement();
        if (not associatedDetEl) {
          return;
        }

        const auto* actsElement =
            dynamic_cast<const ActsDetectorElement*>(associatedDetEl);
        if (not actsElement) {
          return;
        }

        const auto* upstreamDetEl = actsElement->upstreamDetectorElement();
        if (not upstreamDetEl) {
          return;
        }

        //                ATH_MSG_VERBOSE("Try casting to TRT for if");
        //                if (dynamic_cast<const
        //                InDetDD::TRT_BaseElement*>(upstreamDetEl)) { return; }

        const auto* trkDetElem =
            dynamic_cast<const Trk::TrkDetElementBase*>(upstreamDetEl);
        if (not trkDetElem) {
          return;
        }

        ATH_MSG_VERBOSE(
            "trkDetElem type: "
            << static_cast<std::underlying_type_t<Trk::DetectorElemType>>(
                   trkDetElem->detectorType()));

        ATH_MSG_VERBOSE("Try casting to SiDetectorElement");
        const auto* detElem =
            dynamic_cast<const InDetDD::SiDetectorElement*>(upstreamDetEl);
        if (not detElem) {
          return;
        }
        ATH_MSG_VERBOSE("detElem = " << detElem);

        // We need to determine the type of state
        std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
            typePattern;
        std::unique_ptr<Trk::TrackParameters> parm;

        // State is a hole (no associated measurement), use predicted parameters
        if (flag.test(Acts::TrackStateFlag::HoleFlag)) {
          ATH_MSG_VERBOSE("State is a hole (no associated measurement)");
          const Acts::BoundTrackParameters actsParam(
              state.referenceSurface().getSharedPtr(), state.smoothed(),
              state.smoothedCovariance(), acts_track.particleHypothesis());
          parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(
              actsParam, tgContext);
          auto boundaryCheck = m_boundaryCheckTool->boundaryCheck(*parm);
          // Check if this is a hole, a dead sensors or a state outside the
          // sensor boundary
          ATH_MSG_VERBOSE(
              "Check if this is a hole, a dead sensors or a state outside the "
              "sensor boundary");
          if (boundaryCheck == Trk::BoundaryCheckResult::DeadElement) {
            if (detElem->isPixel()) {
              ++numberOfDeadPixel;
            } else if (detElem->isSCT()) {
              ++numberOfDeadSCT;
            }
            // Dead sensors states are not stored
            return;
          } else if (boundaryCheck != Trk::BoundaryCheckResult::Candidate) {
            // States outside the sensor boundary are ignored
            return;
          }
          typePattern.set(Trk::TrackStateOnSurface::Hole);
        } else if (flag.test(Acts::TrackStateFlag::OutlierFlag)) {
          ATH_MSG_VERBOSE("The state was tagged as an outlier");
          const Acts::BoundTrackParameters actsParam(
              state.referenceSurface().getSharedPtr(), state.smoothed(),
              state.smoothedCovariance(), acts_track.particleHypothesis());
          parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(
              actsParam, tgContext);
          typePattern.set(Trk::TrackStateOnSurface::Outlier);
        } else {
          ATH_MSG_VERBOSE("The state is a measurement state");

          const Acts::BoundTrackParameters actsParam(
              state.referenceSurface().getSharedPtr(), state.smoothed(),
              state.smoothedCovariance(), acts_track.particleHypothesis());

          actsSmoothedParam.push_back(
              std::make_unique<const Acts::BoundTrackParameters>(
                  Acts::BoundTrackParameters(actsParam)));
          parm = m_ATLASConverterTool->actsTrackParametersToTrkParameters(
              actsParam, tgContext);
          typePattern.set(Trk::TrackStateOnSurface::Measurement);
        }

        std::unique_ptr<Trk::MeasurementBase> measState;
        if (state.hasUncalibratedSourceLink() && !SourceLinkType) {
          auto sl =
              state.getUncalibratedSourceLink().template get<ATLASSourceLink>();
          assert(sl);
          measState = sl->uniqueClone();
        } else if (state.hasUncalibratedSourceLink() && SourceLinkType) {
          // If the SourceLink is of type PRDSourceLinkGX2F, we need to create
          // the RIO_OnTrack here.
          auto sl = state.getUncalibratedSourceLink()
                        .template get<PRDSourceLinkGX2F>()
                        .prd;

          // ROT creation
          const IdentifierHash idHash = sl->detectorElement()->identifyHash();
          int dim = state.calibratedSize();
          std::unique_ptr<Trk::RIO_OnTrack> rot;
          if (dim == 1) {
            const InDet::SCT_Cluster* sct_Cluster =
                dynamic_cast<const InDet::SCT_Cluster*>(sl);
            if (!sct_Cluster) {
              ATH_MSG_ERROR("ERROR could not cast PRD to SCT_Cluster");
              return;
            }
            rot = std::make_unique<InDet::SCT_ClusterOnTrack>(
                sct_Cluster,
                Trk::LocalParameters(Trk::DefinedParameter(
                    state.template calibrated<1>()[0], Trk::loc1)),
                state.template calibratedCovariance<1>(), idHash);
          } else if (dim == 2) {
            const InDet::PixelCluster* pixelCluster =
                dynamic_cast<const InDet::PixelCluster*>(sl);
            if (!pixelCluster) {
              // sometimes even with dim=2, only the SCT_Cluster implementation
              // work for RIO_OnTrack creation
              ATH_MSG_VERBOSE(
                  "Dimension is 2 but we need SCT_Cluster for this "
                  "measurement");
              const InDet::SCT_Cluster* sct_Cluster =
                  dynamic_cast<const InDet::SCT_Cluster*>(sl);
              rot = std::make_unique<InDet::SCT_ClusterOnTrack>(
                  sct_Cluster,
                  Trk::LocalParameters(Trk::DefinedParameter(
                      state.template calibrated<1>()[0], Trk::loc1)),
                  state.template calibratedCovariance<1>(), idHash);
            } else {
              rot = std::make_unique<InDet::PixelClusterOnTrack>(
                  pixelCluster,
                  Trk::LocalParameters(state.template calibrated<2>()),
                  state.template calibratedCovariance<2>(), idHash);
            }
          } else {
            throw std::domain_error("Cannot handle measurement dim>2");
          }
          measState = rot->uniqueClone();
        }

        double nDoF = state.calibratedSize();
        auto quality = Trk::FitQualityOnSurface(state.chi2(), nDoF);
        const Trk::TrackStateOnSurface* perState =
            new Trk::TrackStateOnSurface(quality, std::move(measState),
                                         std::move(parm), nullptr, typePattern);
        // If a state was successfully created add it to the trajectory
        if (perState) {
          ATH_MSG_VERBOSE(
              "State successfully created, adding it to the trajectory");
          finalTrajectory->insert(finalTrajectory->begin(), perState);
        }
      });

  // Convert the perigee state and add it to the trajectory
  const Acts::BoundTrackParameters actsPer(
      acts_track.referenceSurface().getSharedPtr(), acts_track.parameters(),
      acts_track.covariance(), acts_track.particleHypothesis());
  std::unique_ptr<Trk::TrackParameters> per =
      m_ATLASConverterTool->actsTrackParametersToTrkParameters(actsPer,
                                                               tgContext);
  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
      typePattern;
  typePattern.set(Trk::TrackStateOnSurface::Perigee);
  const Trk::TrackStateOnSurface* perState = new Trk::TrackStateOnSurface(
      nullptr, std::move(per), nullptr, typePattern);
  if (perState) {
    finalTrajectory->insert(finalTrajectory->begin(), perState);
  }

  // Create the track using the states
  Trk::TrackInfo newInfo(Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                         Trk::noHypothesis);
  // Mark the fitter as GlobalChi2Fitter
  newInfo.setTrackFitter(Trk::TrackInfo::TrackFitter::GlobalChi2Fitter);
  newtrack = std::make_unique<Trk::Track>(newInfo, std::move(finalTrajectory),
                                          nullptr);
  if (newtrack) {
    // Create the track summary and update the holes information
    if (!newtrack->trackSummary()) {
      newtrack->setTrackSummary(std::make_unique<Trk::TrackSummary>());
      newtrack->trackSummary()->update(Trk::numberOfPixelHoles, 0);
      newtrack->trackSummary()->update(Trk::numberOfSCTHoles, 0);
      newtrack->trackSummary()->update(Trk::numberOfTRTHoles, 0);
      newtrack->trackSummary()->update(Trk::numberOfPixelDeadSensors,
                                       numberOfDeadPixel);
      newtrack->trackSummary()->update(Trk::numberOfSCTDeadSensors,
                                       numberOfDeadSCT);
    }
    m_trkSummaryTool->updateTrackSummary(ctx, *newtrack, true);
  }
  return newtrack;
}

std::unique_ptr<ActsTrk::MutableTrackContainer> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const ActsTrk::Seed& seed,
    const Acts::BoundTrackParameters& initialParams,
    const Acts::GeometryContext& tgContext,
    const Acts::MagneticFieldContext& mfContext,
    const Acts::CalibrationContext& calContext,
    const DetectorElementToActsGeometryIdMap& detectorElementToGeometryIdMap)
    const {
  const Acts::TrackingGeometry* actsTrackingGeometry =
      m_trackingGeometryTool->trackingGeometry().get();
  if (!actsTrackingGeometry) {
    throw std::runtime_error("No Acts tracking geometry.");
  }

  std::vector<ActsTrk::ATLASUncalibSourceLink> sourceLinks;
  sourceLinks.reserve(6);

  std::vector<const Acts::Surface*> surfaces;
  surfaces.reserve(6);

  const auto& sps = seed.sp();
  for (const xAOD::SpacePoint* sp : sps) {
    const auto& measurements = sp->measurements();
    for (const xAOD::UncalibratedMeasurement* umeas : measurements) {
      ActsTrk::ATLASUncalibSourceLink el(makeATLASUncalibSourceLink(umeas));
      sourceLinks.emplace_back(el);
      surfaces.push_back(ActsTrk::getSurfaceOfMeasurement(
          *actsTrackingGeometry, detectorElementToGeometryIdMap, *umeas));
    }
  }
  return fit(ctx, sourceLinks, initialParams, tgContext, mfContext, calContext,
             detectorElementToGeometryIdMap, surfaces.front());
}


StatusCode GlobalChiSquareFitterTool::fit(
  const EventContext& /*ctx*/,
    const ActsTrk::TrackContainer::ConstTrackProxy& /*track*/,          
  ActsTrk::MutableTrackContainer& /*trackContainer*/) const 
{
  ATH_MSG_ERROR("Track refit method not implemented in GlobalChiSquareFitterTool yet");
  return StatusCode::FAILURE;
}
}  // namespace ActsTrk
