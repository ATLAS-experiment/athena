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

#include "ActsCalibBase/CalibrationContext.h"
// STL
#include <vector>

namespace ActsTrk {

StatusCode GlobalChiSquareFitterTool::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_ROTcreator.retrieve(EnableTool{m_doReFitFromPRD}));
  ATH_CHECK(m_muonCalibrator.retrieve(EnableTool{!m_muonCalibrator.empty()}));

  m_logger = makeActsAthenaLogger(this, "Gx2fRefit");
  if (!m_doStraightLine){
      // Fitter
      Acts::SympyStepper stepper{std::make_shared<ATLASMagneticFieldWrapper>()};
      Acts::Navigator::Config navConfig{m_trackingGeometryTool->trackingGeometry()};
      Acts::Navigator navigator(std::move(navConfig), logger().cloneWithSuffix("Navigator"));
      CurvedPropagator_t propagator{stepper, std::move(navigator), logger().cloneWithSuffix("Prop")};

      m_fitter = std::make_unique<CurvedFitter_t>(std::move(propagator), 
                                                  logger().cloneWithSuffix("GlobalChiSquareFitter"));
  } else {
      Acts::StraightLineStepper stepper{};
      Acts::Navigator::Config navConfig{m_trackingGeometryTool->trackingGeometry()};
      Acts::Navigator navigator(std::move(navConfig), logger().cloneWithSuffix("Navigator"));
      StraightPropagator_t propagator{stepper, std::move(navigator), logger().cloneWithSuffix("Prop")};

      m_slFitter = std::make_unique<StraightFitter_t>(std::move(propagator), 
                                                      logger().cloneWithSuffix("GlobalChiSquareFitter"));
  }


  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;

  Gx2FitterExtension_t extensionTemplate{};
  extensionTemplate.outlierFinder.connect<&detail::FitterHelperFunctions::ATLASOutlierFinder::operator()
                                            <MutableTrackStateBackend>>(&m_outlierFinder);
  extensionTemplate.updater.connect<&detail::FitterHelperFunctions::gainMatrixUpdate<MutableTrackStateBackend>>();

  /// Configure the fit extensions for Trk::MeasuremenBase pass through fits.
  {
    m_trkMeasCalibrator = detail::TrkMeasurementCalibrator{};
    m_trkMeasSurfAcc = detail::TrkMeasSurfaceAccessor{m_ATLASConverterTool.get()};

    Gx2FitterExtension_t& configureMe = m_gx2fExtensions[static_cast<int>(detail::SourceLinkType::TrkMeasurement)];
    configureMe = extensionTemplate;
    configureMe.calibrator.connect<&detail::TrkMeasurementCalibrator::calibrate<MutableTrackStateBackend>>(&m_trkMeasCalibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkMeasSurfaceAccessor::operator()>(&m_trkMeasSurfAcc);
  }
  /// Configure the fit extensions for Trk::PrepRawData fits
  {
    m_prdCalibrator = detail::TrkPrepRawDataCalibrator{m_ATLASConverterTool.get(), m_ROTcreator.get()};
    m_prdSurfaceAcc = detail::TrkPrepRawDataSurfaceAcc{m_ATLASConverterTool.get()};
    Gx2FitterExtension_t& configureMe = m_gx2fExtensions[static_cast<int>(detail::SourceLinkType::TrkPrepRawData)];
    configureMe = extensionTemplate;
    configureMe.calibrator.connect<&detail::TrkPrepRawDataCalibrator::calibrate<MutableTrackStateBackend>>(&m_prdCalibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkPrepRawDataSurfaceAcc::operator()>(&m_prdSurfaceAcc);
  }
  {
    m_unalibMeasSurfAcc = detail::xAODUncalibMeasSurfAcc{m_trackingGeometryTool.get()};
    /// Needs to be filled with live.
    Gx2FitterExtension_t& configureMe = m_gx2fExtensions[static_cast<int>(detail::SourceLinkType::xAODUnCalibMeas)];
    configureMe = extensionTemplate;
    configureMe.surfaceAccessor.connect<&detail::xAODUncalibMeasSurfAcc::operator()>(&m_unalibMeasSurfAcc);
    configureMe.calibrator.connect<&detail::xAODUncalibMeasCalibrator::calibrate>(&m_uncalibMeasCalibrator);
    /// Connect the muon types with the muon calibrator
    if (m_muonCalibrator.isEnabled()) {
      for (const auto& muonType : {xAOD::UncalibMeasType::MdtDriftCircleType, xAOD::UncalibMeasType::RpcStripType, 
                                   xAOD::UncalibMeasType::TgcStripType, xAOD::UncalibMeasType::MMClusterType, 
                                   xAOD::UncalibMeasType::sTgcStripType}) {
        m_uncalibMeasCalibrator.connect<&MuonR4::ISpacePointCalibrator::calibrateSourceLink>(muonType, m_muonCalibrator.get());
      }
    }
  }
  return StatusCode::SUCCESS;
}
inline GlobalChiSquareFitterTool::TrackFitResult_t
    GlobalChiSquareFitterTool::fit(const std::vector<Acts::SourceLink>& soureLinks,
                                   const Acts::BoundTrackParameters& initialPars,
                                   const Gx2FitterOptions_t& gx2fOptions, 
                                   MutableTrackContainer& tracks) const {
    return ATH_LIKELY(m_fitter) ? 
        m_fitter->fit(soureLinks.begin(), soureLinks.end(), initialPars, gx2fOptions, tracks) :
        m_slFitter->fit(soureLinks.begin(), soureLinks.end(), initialPars, gx2fOptions, tracks);
}

GlobalChiSquareFitterTool::Gx2FitterOptions_t 
    GlobalChiSquareFitterTool::configureFit(const Acts::GeometryContext& tgContext,
                                            const Acts::MagneticFieldContext& mfContext,
                                            const Acts::CalibrationContext& calContext,
                                            const Acts::Surface* surface,
                                            detail::SourceLinkType slType) const {
    Acts::PropagatorPlainOptions propagationOption{tgContext, mfContext};
    propagationOption.maxSteps = m_option_maxPropagationStep;
    propagationOption.maxTargetSkipping = m_option_maxNavSurfaces;
    // Set the Gx2Fitter options
    return Gx2FitterOptions_t{tgContext, mfContext, calContext, 
                              m_gx2fExtensions[static_cast<int>(slType)], 
                              std::move(propagationOption),
                              surface, m_option_includeScat, 
                              m_option_includeELoss};
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
  auto result = fit(trackSourceLinks, initialParamsWithHypothesis, gx2fOptions, tracks);

  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                                Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                                                detail::SourceLinkType::TrkMeasurement);
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
  auto result = fit(trackSourceLinks, initialParams, gx2fOptions, tracks);
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
  auto result = fit(trackSourceLinks, initialParams, gx2fOptions, tracks);

  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                  Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                                  detail::SourceLinkType::TrkPrepRawData);
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<MutableTrackContainer> GlobalChiSquareFitterTool::fit(    
    const std::vector<ATLASUncalibSourceLink>& measList,
    const Acts::BoundTrackParameters& initialParams,
    const Acts::GeometryContext& tgContext,
    const Acts::MagneticFieldContext& mfContext,
    const Acts::CalibrationContext& calContext,   
    const Acts::Surface* targetSurface) const {
  
  if (measList.empty()) {
      ATH_MSG_DEBUG("No measurements given. Nothing to do");
      return nullptr;
  }
  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::Surface> pSurface{};
  if (!targetSurface) {
    pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
    targetSurface = pSurface.get();
  }
 
  std::vector<Acts::SourceLink> sourceLinks;
  sourceLinks.reserve(measList.size());
  std::ranges::transform(measList, std::back_inserter(sourceLinks), 
                         [](const xAOD::UncalibratedMeasurement* meas){
                             return detail::xAODUncalibMeasCalibrator::pack(meas);
                         });

  Gx2FitterOptions_t gx2fOptions = configureFit(tgContext, mfContext, calContext,
                                                targetSurface, detail::SourceLinkType::xAODUnCalibMeas);

  auto tracks = std::make_unique<MutableTrackContainer>(); 
  // Perform the fit
  auto result = fit(sourceLinks, initialParams, gx2fOptions, *tracks);
  if (not result.ok()) {
      ATH_MSG_VERBOSE("Global chi2 fit failed");
      return nullptr;
  }
  return tracks;
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
  auto result = fit(trackSourceLinks, initialParams, gx2fOptions, tracks);
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
  auto result = fit(trackSourceLinks, initialParamsWithHypothesis, gx2fOptions, tracks);
      
  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                Trk::TrackInfo::TrackFitter::GlobalChi2Fitter,
                                detail::SourceLinkType::TrkMeasurement);
}

std::unique_ptr<MutableTrackContainer> GlobalChiSquareFitterTool::fit(
    const Seed& seed,
    const Acts::BoundTrackParameters& initialParams,
    const Acts::GeometryContext& tgContext,
    const Acts::MagneticFieldContext& mfContext,
    const Acts::CalibrationContext& calContext,
    const Acts::Surface& targetSurface) const {
  
  std::vector<ATLASUncalibSourceLink> sourceLinks;
  sourceLinks.reserve(6);
  
  for (const xAOD::SpacePoint* sp : seed.sp()) {
    sourceLinks.insert(sourceLinks.end(), sp->measurements().begin(), sp->measurements().end());
  }
  return fit(sourceLinks, initialParams, tgContext, mfContext, calContext, &targetSurface);
}


StatusCode GlobalChiSquareFitterTool::fit(
  const EventContext& /*ctx*/,
  const TrackContainer::ConstTrackProxy& /*track*/,          
  MutableTrackContainer& /*trackContainer*/,
  const Acts::PerigeeSurface& /*pSurface*/) const
{
  ATH_MSG_ERROR("Track refit method not implemented in GlobalChiSquareFitterTool yet");
  return StatusCode::FAILURE;
}
}  // namespace ActsTrk
