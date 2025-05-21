/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalChiSquareFitterTool.h"

// ATHENA
#include "Acts/EventData/Types.hpp"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
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

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsInterop/Logger.h"

#include "ActsCalibration/CalibrationContext.h"
#include "ActsCalibration/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibration/xAODUncalibMeasCalibrator.h"
// STL
#include <vector>

namespace ActsTrk {




StatusCode GlobalChiSquareFitterTool::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_ROTcreator.retrieve(EnableTool{m_doReFitFromPRD}));
  

  m_logger = makeActsAthenaLogger(this, "Gx2fRefit");
  // Fitter
  Acts::SympyStepper stepper{std::make_shared<ATLASMagneticFieldWrapper>()};
  Acts::Navigator::Config navConfig{m_trackingGeometryTool->trackingGeometry()};
  Acts::Navigator navigator(std::move(navConfig), logger().cloneWithSuffix("Navigator"));
  Acts::Propagator<Acts::SympyStepper, Acts::Navigator> propagator{stepper, 
                                                                   std::move(navigator), 
                                                                   logger().cloneWithSuffix("Prop")};

  m_fitter = std::make_unique<Fitter>(std::move(propagator), 
                                      logger().cloneWithSuffix("GlobalChiSquareFitter"));

  


  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;

  Gx2FitterExtension_t extensionTemplate{};
  extensionTemplate.outlierFinder.connect<&detail::FitterHelperFunctions::ATLASOutlierFinder::operator()
                                            <MutableTrackStateBackend>>(&m_outlierFinder);
  extensionTemplate.updater.connect<&detail::FitterHelperFunctions::gainMatrixUpdate<MutableTrackStateBackend>>();

  /// Configure the fit extensions for Trk::MeasuremenBase pass through fits.
  {
    m_calibrator = detail::TrkMeasurementCalibrator{};
    m_trkMeasSurfAcc = detail::TrkMeasSurfaceAccessor{m_ATLASConverterTool.get()};

    Gx2FitterExtension_t& configureMe = m_gx2fExtensions[static_cast<int>(detail::SourceLinkType::TrkMeasurement)];
    configureMe = extensionTemplate;
    configureMe.calibrator.connect<&detail::TrkMeasurementCalibrator::calibrate<ActsTrk::MutableTrackStateBackend>>(&m_calibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkMeasSurfaceAccessor::operator()>(&m_trkMeasSurfAcc);
  }
  /// Configure the fit extensions for Trk::PrepRawData fits
  {
    m_prdCalibrator = detail::TrkPrepRawDataCalibrator{m_ATLASConverterTool.get(), m_ROTcreator.get()};
    m_prdSurfaceAcc = detail::TrkPrepRawDataSurfaceAcc{m_ATLASConverterTool.get()};
    Gx2FitterExtension_t& configureMe = m_gx2fExtensions[static_cast<int>(detail::SourceLinkType::TrkPrepRawData)];
    configureMe.calibrator.connect<&detail::TrkPrepRawDataCalibrator::calibrate<MutableTrackStateBackend>>(&m_prdCalibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkPrepRawDataSurfaceAcc::operator()>(&m_prdSurfaceAcc);
  }
  return StatusCode::SUCCESS;
}

GlobalChiSquareFitterTool::Gx2FitterOptions_t 
    GlobalChiSquareFitterTool::configureFit(const Acts::GeometryContext& tgContext,
                                            const Acts::MagneticFieldContext& mfContext,
                                            const Acts::CalibrationContext& calContext,
                                            const Acts::Surface* surface,
                                            detail::SourceLinkType slType) const {
    Acts::PropagatorPlainOptions propagationOption{tgContext, mfContext};
    propagationOption.maxSteps = m_option_maxPropagationStep;
    // Set the Gx2Fitter options
    return Gx2FitterOptions_t{tgContext, mfContext, calContext, 
                              m_gx2fExtensions[static_cast<int>(slType)], 
                              std::move(propagationOption),
                              surface, true, true};
  
                                        
}

// refit a track
// -------------------------------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const Trk::Track& inputTrack,
    const Trk::RunOutlierRemoval /*runOutlier*/,
    const Trk::ParticleHypothesis hypothesis) const {
 
  ATH_MSG_VERBOSE("--> enter GlobalChiSquareFitterTool::fit(Track,,) with Track from author = "
                << inputTrack.info().dumpInfo());

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || inputTrack.measurementsOnTrack()->size() < 2) {
    ATH_MSG_DEBUG("called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input fails to provide track parameters for seeding the GX2F, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(inputTrack);
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue "
                <<"with the converter, reject fit ");
    return nullptr;
  }
 
  const Acts::GeometryContext tgContext{m_trackingGeometryTool->getGeometryContext(ctx).context()};
  const Acts::MagneticFieldContext mfContext{m_extrapolationTool->getMagneticFieldContext(ctx)};
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};
  const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters((*inputTrack.perigeeParameters()), tgContext);

  
  const Acts::BoundTrackParameters initialParamsWithHypothesis(
      initialParams.referenceSurface().getSharedPtr(),
      initialParams.parameters(), initialParams.covariance(), 
      ParticleHypothesis::convert(hypothesis));

  Gx2FitterOptions_t gx2fOptions = configureFit(tgContext, mfContext, calContext, 
                                                pSurface.get(), detail::SourceLinkType::TrkMeasurement);
  MutableTrackContainer tracks;
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParamsWithHypothesis, gx2fOptions, tracks);

  auto trackone =  m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                                Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                                                detail::SourceLinkType::TrkMeasurement);
  return trackone;

}

// fit a set of MeasurementBase objects
// --------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const Trk::MeasurementSet& inputMeasSet,
    const Trk::TrackParameters& estimatedStartParameters,
    const Trk::RunOutlierRemoval /*runOutlier*/,
    const Trk::ParticleHypothesis /*matEffects*/) const {

  // protection against not having measurements on the input track
  if (inputMeasSet.size() < 2) {
    ATH_MSG_DEBUG("Called to refit empty measurement set or a measurement set with too "
                <<"little information, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  const Acts::GeometryContext tgContext{m_trackingGeometryTool->getGeometryContext(ctx).context()};
  const Acts::MagneticFieldContext mfContext{m_extrapolationTool->getMagneticFieldContext(ctx)};
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};
 
  

  std::vector<Acts::SourceLink> trackSourceLinks;
  m_ATLASConverterTool->toSourceLinks(inputMeasSet, trackSourceLinks);
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue "
                <<"with the converter, reject fit ");
    return nullptr;
  }

  const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(estimatedStartParameters, tgContext);

  MutableTrackContainer tracks;

  Gx2FitterOptions_t gx2fOptions = configureFit(tgContext, mfContext, calContext,
                                                pSurface.get(), detail::SourceLinkType::TrkMeasurement);
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParams, gx2fOptions, tracks);
  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                                Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                                                detail::SourceLinkType::TrkMeasurement);
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track> GlobalChiSquareFitterTool::fit(const EventContext& ctx, 
                                                           const Trk::PrepRawDataSet& inputPRDColl,
                                                           const Trk::TrackParameters& estimatedStartParameters,
                                                           const Trk::RunOutlierRemoval /*runOutlier*/,
                                                           const Trk::ParticleHypothesis /*prtHypothesis*/) const {
  
  ATH_MSG_DEBUG("--> entering GlobalChiSquareFitterTool::fit(PRDS,TP,)");

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

  std::vector<Acts::SourceLink> trackSourceLinks;
  m_ATLASConverterTool->toSourceLinks(inputPRDColl, trackSourceLinks);
  
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_WARNING("input contain measurement but no source link created, probable issue "
                    "with the converter, reject fit ");
    return nullptr;
  }

  const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(estimatedStartParameters, tgContext);

  
  Gx2FitterOptions_t gx2fOptions = configureFit(tgContext, mfContext, calContext,
                                                pSurface.get(), detail::SourceLinkType::TrkPrepRawData);

  MutableTrackContainer tracks;
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParams, gx2fOptions, tracks);

  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                  Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                                  detail::SourceLinkType::TrkPrepRawData);
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<MutableTrackContainer> GlobalChiSquareFitterTool::fit(
    const EventContext& /*eventContext*/,
    const std::vector<ATLASUncalibSourceLink>& /*clusterList*/,
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
  ATH_MSG_VERBOSE("--> enter GlobalChiSquareFitterTool::fit(Track,Meas'BaseSet,,)");
  ATH_MSG_VERBOSE("    with Track from author = " << inputTrack.info().dumpInfo());

  // protection, if empty MeasurementSet
  if (addMeasColl.empty()) {
    ATH_MSG_DEBUG("client tries to add an empty MeasurementSet to the track fit.");
    return fit(ctx, inputTrack);
  }

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || (inputTrack.measurementsOnTrack()->size() < 2 && addMeasColl.empty())) {
    ATH_MSG_DEBUG("Called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input fails to provide track parameters for seeding the GX2F, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  const Acts::GeometryContext tgContext{m_trackingGeometryTool->getGeometryContext(ctx).context()};
  const Acts::MagneticFieldContext mfContext{m_extrapolationTool->getMagneticFieldContext(ctx)};
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(inputTrack);
  m_ATLASConverterTool->toSourceLinks(addMeasColl, trackSourceLinks);
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue "
                  <<"with the converter, reject fit ");
    return nullptr;
  }
  const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(*(inputTrack.perigeeParameters()), 
                                                                                      tgContext);

  MutableTrackContainer tracks;

  Gx2FitterOptions_t gx2fOptions = configureFit(tgContext, mfContext, calContext,
                                                pSurface.get(), detail::SourceLinkType::TrkMeasurement);
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParams, gx2fOptions, tracks);

  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                                detail::SourceLinkType::TrkMeasurement);
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
    ATH_MSG_DEBUG("input #1 fails to provide track parameters for seeding the GX2F, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  const Acts::GeometryContext tgContext{m_trackingGeometryTool->getGeometryContext(ctx).context()};
  const Acts::MagneticFieldContext mfContext{m_extrapolationTool->getMagneticFieldContext(ctx)};
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};
  
  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(intrk1);
  std::vector<Acts::SourceLink> trackSourceLinks2 = m_ATLASConverterTool->trkTrackToSourceLinks(intrk2);
  trackSourceLinks.insert(trackSourceLinks.end(), std::make_move_iterator(trackSourceLinks2.begin()),
                          std::make_move_iterator(trackSourceLinks2.end()));
  // protection against error in the conversion from Atlas measurement to ACTS
  // source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue "
                <<"with the converter, reject fit ");
    return nullptr;
  }

  const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(*(intrk1.perigeeParameters()), tgContext);


  const Acts::BoundTrackParameters initialParamsWithHypothesis(
      initialParams.referenceSurface().getSharedPtr(),
      initialParams.parameters(), initialParams.covariance(), 
      ParticleHypothesis::convert(matEffects));

  MutableTrackContainer tracks;

  Gx2FitterOptions_t gx2fOptions = configureFit(tgContext, mfContext, calContext,
                                                pSurface.get(), detail::SourceLinkType::TrkMeasurement);
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParamsWithHypothesis, gx2fOptions, tracks);
  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                                detail::SourceLinkType::TrkMeasurement);
}

std::unique_ptr<MutableTrackContainer> GlobalChiSquareFitterTool::fit(
    const EventContext& ctx, const Seed& seed,
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

  std::vector<ATLASUncalibSourceLink> sourceLinks;
  sourceLinks.reserve(6);

  
  detail::xAODUncalibMeasSurfAcc surfAcc{actsTrackingGeometry, &detectorElementToGeometryIdMap};
  for (const xAOD::SpacePoint* sp : seed.sp()) {
    sourceLinks.insert(sourceLinks.end(), sp->measurements().begin(), sp->measurements().end());
  }
  return fit(ctx, sourceLinks, initialParams, tgContext, mfContext, calContext,
             detectorElementToGeometryIdMap, surfAcc.get(sourceLinks.front()));
}


StatusCode GlobalChiSquareFitterTool::fit(
  const EventContext& /*ctx*/,
    const TrackContainer::ConstTrackProxy& /*track*/,          
  MutableTrackContainer& /*trackContainer*/) const 
{
  ATH_MSG_ERROR("Track refit method not implemented in GlobalChiSquareFitterTool yet");
  return StatusCode::FAILURE;
}
}  // namespace ActsTrk
