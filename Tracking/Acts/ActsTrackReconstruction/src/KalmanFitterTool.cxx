/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/KalmanFitterTool.h"

// ATHENA
#include "Acts/EventData/Types.hpp"
#include "GaudiKernel/TypeNameString.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "GaudiKernel/EventContext.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h" 
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkTrack/Track.h"

// ACTS
#include "Acts/EventData/Types.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Propagator/SympyStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFitting/KalmanFitter.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"

#include "InDetPrepRawData/SCT_Cluster.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsInterop/Logger.h"



#include "Acts/Propagator/DirectNavigator.hpp"
#include "src/detail/OnTrackCalibrator.h"

// STL
#include <vector>

namespace ActsTrk {


StatusCode KalmanFitterTool::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_ROTcreator.retrieve(EnableTool{m_doReFitFromPRD}));
  m_logger = makeActsAthenaLogger(this, "KalmanRefit");

  auto field = std::make_shared<ATLASMagneticFieldWrapper>();

  // Fitter
  Acts::SympyStepper stepper(field);
  Acts::Navigator navigator( Acts::Navigator::Config{ m_trackingGeometryTool->trackingGeometry() },
			     logger().cloneWithSuffix("Navigator"));
  Acts::Propagator<Acts::SympyStepper, Acts::Navigator> propagator(stepper, 
                     std::move(navigator),
                     logger().cloneWithSuffix("Prop"));

  m_fitter = std::make_unique<Fitter>(std::move(propagator),
              logger().cloneWithSuffix("KalmanFitter"));

  // Direct Fitter
  Acts::DirectNavigator directNavigator( logger().cloneWithSuffix("DirectNavigator") );
  Acts::Propagator<Acts::SympyStepper, Acts::DirectNavigator> directPropagator(std::move(stepper),
										 std::move(directNavigator),
										 logger().cloneWithSuffix("DirectPropagator"));

  m_directFitter = std::make_unique<DirectFitter>(std::move(directPropagator),
						  logger().cloneWithSuffix("DirectKalmanFitter"));

  ///
  
  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;
  m_reverseFilteringLogic.momentumMax = m_option_ReverseFilteringPt;

  FitterExtension_t extensionTemplate{};
  extensionTemplate.outlierFinder.connect<&detail::FitterHelperFunctions::ATLASOutlierFinder::operator()<MutableTrackStateBackend>>(&m_outlierFinder);
  extensionTemplate.reverseFilteringLogic.connect<&detail::FitterHelperFunctions::ReverseFilteringLogic::operator()<MutableTrackStateBackend>>(&m_reverseFilteringLogic);
  extensionTemplate.updater.connect<&detail::FitterHelperFunctions::gainMatrixUpdate<MutableTrackStateBackend>>();
  extensionTemplate.smoother.connect<&detail::FitterHelperFunctions::mbfSmoother<MutableTrackStateBackend>>();
   
  extensionTemplate.outlierFinder.connect<&detail::FitterHelperFunctions::ATLASOutlierFinder::operator()
                                            <MutableTrackStateBackend>>(&m_outlierFinder);
  extensionTemplate.updater.connect<&detail::FitterHelperFunctions::gainMatrixUpdate<MutableTrackStateBackend>>();

  /// Configure the fit extensions for Trk::MeasuremenBase pass through fits.
  {
    m_trkSurfAcc = detail::TrkMeasSurfaceAccessor{m_ATLASConverterTool.get()};

    FitterExtension_t& configureMe = m_kfExtensions[static_cast<int>(detail::SourceLinkType::TrkMeasurement)];
    configureMe = extensionTemplate;
    configureMe.calibrator.connect<&detail::TrkMeasurementCalibrator::calibrate<MutableTrackStateBackend>>(&m_trkCalibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkMeasSurfaceAccessor::operator()>(&m_trkSurfAcc);
  }
  /// Configure the fit extensions for the Trk::PrepRawData fits
  {
     m_prdCalibrator = detail::TrkPrepRawDataCalibrator{m_ATLASConverterTool.get(), m_ROTcreator.get()};
     m_prdSurfAcc = detail::TrkPrepRawDataSurfaceAcc{m_ATLASConverterTool.get()};
     FitterExtension_t& configureMe = m_kfExtensions[static_cast<int>(detail::SourceLinkType::TrkPrepRawData)];
     configureMe = extensionTemplate;
     configureMe.calibrator.connect<&detail::TrkPrepRawDataCalibrator::calibrate<MutableTrackStateBackend>>(&m_prdCalibrator);
     configureMe.surfaceAccessor.connect<&detail::TrkPrepRawDataSurfaceAcc::operator()>(&m_prdSurfAcc);
  }
  /// Configure the fit extensions for the uncalibrated measurement fits
  {
    m_unalibMeasSurfAcc = detail::xAODUncalibMeasSurfAcc{m_trackingGeometryTool.get()}; 
    m_uncalibMeasCalibrator = xAODUnCalibrator_t::NoCalibration(m_trackingGeometryTool.get());
    FitterExtension_t& configureMe = m_kfExtensions[static_cast<int>(detail::SourceLinkType::xAODUnCalibMeas)];
    configureMe = extensionTemplate;
    configureMe.surfaceAccessor.connect<&detail::xAODUncalibMeasSurfAcc::operator()>(&m_unalibMeasSurfAcc);
    configureMe.calibrator.connect<&xAODUnCalibrator_t::calibrate>(&m_uncalibMeasCalibrator);
  }
  return StatusCode::SUCCESS;
}


KalmanFitterTool::FitterOptions_t 
    KalmanFitterTool::configureFit(const Acts::GeometryContext& tgContext,
                                   const Acts::MagneticFieldContext& mfContext,
                                   const Acts::CalibrationContext& calContext,
                                   const Acts::Surface* surface,
                                   detail::SourceLinkType slType) const {
  
  
  const auto& kfExtensions = m_kfExtensions[static_cast<int>(slType)];

  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the KalmanFitter options
  return FitterOptions_t{tgContext, mfContext, calContext,
                         kfExtensions, propagationOption,
                         surface};
}
// refit a track
// -------------------------------------------------------
std::unique_ptr<Trk::Track>
KalmanFitterTool::fit(const EventContext& ctx,
		      const Trk::Track& inputTrack,
		      const Trk::RunOutlierRemoval /*runOutlier*/,
		      const Trk::ParticleHypothesis hypothesis) const {
 
  ATH_MSG_VERBOSE ("--> enter KalmanFitter::fit(Track,,)    with Track from author = "
       << inputTrack.info().dumpInfo()<<", "<<hypothesis);

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || inputTrack.measurementsOnTrack()->size() < 2) {
    ATH_MSG_DEBUG("called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input fails to provide track parameters for seeding the KF, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
  
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};
  // Set the KalmanFitter options
  Acts::KalmanFitterOptions kfOptions = configureFit(tgContext, mfContext, calContext, pSurface.get(),
                                                     detail::SourceLinkType::TrkMeasurement);

  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(inputTrack);
  // protection against error in the conversion from Atlas masurement to Acts source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return nullptr;
  }

  const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters((*inputTrack.perigeeParameters()), tgContext);

  // The covariance from already fitted track are too small and would result an incorect smoothing.
  // We scale up the input covaraiance to avoid this.
  Acts::BoundSquareMatrix scaledCov = Acts::BoundSquareMatrix::Identity();
  for (int i=0; i<6; ++i) {
    double scale = m_option_seedCovarianceScale;
    (scaledCov)(i,i) = scale * initialParams.covariance().value()(i,i);
  }

  const Acts::BoundTrackParameters scaledInitialParams(initialParams.referenceSurface().getSharedPtr(),
                                                       initialParams.parameters(),
                                                       scaledCov,
                                                       Acts::ParticleHypothesis::pion());

  MutableTrackContainer tracks;
  
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              scaledInitialParams, kfOptions, tracks);
  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                      Trk::TrackInfo::TrackFitter::KalmanFitter,
                                      detail::SourceLinkType::TrkMeasurement);
}

// fit a set of MeasurementBase objects
// --------------------------------
std::unique_ptr<Trk::Track>
KalmanFitterTool::fit(const EventContext& ctx,
		                  const Trk::MeasurementSet& inputMeasSet,
		                  const Trk::TrackParameters& estimatedStartParameters,
		                  const Trk::RunOutlierRemoval /*runOutlier*/,
		                  const Trk::ParticleHypothesis /*matEffects*/) const {

  // protection against not having measurements on the input track
  if (inputMeasSet.size() < 2) {
    ATH_MSG_DEBUG("called to refit empty measurement set or a measurement set with too little information, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
  
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

  Acts::KalmanFitterOptions kfOptions = configureFit(tgContext, mfContext, calContext, pSurface.get(),
                                                     detail::SourceLinkType::TrkMeasurement);

  std::vector<Acts::SourceLink> trackSourceLinks;
  m_ATLASConverterTool->toSourceLinks(inputMeasSet, trackSourceLinks);
  // protection against error in the conversion from Atlas masurement to Acts source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return nullptr;
  }

  const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(estimatedStartParameters, tgContext); 

  MutableTrackContainer tracks;

  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParams, kfOptions, tracks);
  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                                Trk::TrackInfo::TrackFitter::KalmanFitter,
                                                detail::SourceLinkType::TrkMeasurement);
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track>
KalmanFitterTool::fit(const EventContext& ctx,
		      const Trk::PrepRawDataSet& inputPRDColl,
		      const Trk::TrackParameters& estimatedStartParameters,
		      const Trk::RunOutlierRemoval /*runOutlier*/,
		      const Trk::ParticleHypothesis /*prtHypothesis*/) const {
    ATH_MSG_DEBUG("--> entering KalmanFitter::fit(PRDS,TP,)");
    
    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero()); 
    
    const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
    const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

    Acts::KalmanFitterOptions kfOptions = configureFit(tgContext, mfContext, calContext, pSurface.get(),
                                                       detail::SourceLinkType::TrkPrepRawData);

    std::vector<Acts::SourceLink> trackSourceLinks;
    m_ATLASConverterTool->toSourceLinks(inputPRDColl, trackSourceLinks);
    // protection against error in the conversion from Atlas masurement to Acts source link
    if (trackSourceLinks.empty()) {
      ATH_MSG_WARNING("input contain measurement but no source link created, probable issue with the converter, reject fit ");
      return nullptr;
    }
    //

    const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(estimatedStartParameters, tgContext); 

    MutableTrackContainer tracks;
    // Perform the fit
    auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                                initialParams, kfOptions, tracks);
    return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                                  Trk::TrackInfo::TrackFitter::KalmanFitter,
                                                  detail::SourceLinkType::TrkPrepRawData);
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr< MutableTrackContainer >
KalmanFitterTool::fit(const EventContext&,
		                  const std::vector< ATLASUncalibSourceLink> & clusterList,
		                  const Acts::BoundTrackParameters& initialParams,
		                  const Acts::GeometryContext& tgContext,
		                  const Acts::MagneticFieldContext& mfContext,
		                  const Acts::CalibrationContext& calContext,
		                  const DetectorElementToActsGeometryIdMap& /*detectorElementToGeometryIdMap*/,
		                  const Acts::Surface* targetSurface) const{
  ATH_MSG_DEBUG("--> entering KalmanFitter::fit(xAODMeasure...things,TP,)");
       
  std::vector<Acts::SourceLink> sourceLinks;
  sourceLinks.reserve(clusterList.size()); 

  std::vector<const Acts::Surface*> surfaces;
  surfaces.reserve(clusterList.size());


  for (const ATLASUncalibSourceLink& el : clusterList) {
    sourceLinks.emplace_back( el );
    surfaces.push_back(m_unalibMeasSurfAcc.get(el));
  }

  // Construct a perigee surface as the target surface if none is provided
  std::shared_ptr<Acts::Surface> pSurface{nullptr};
  if (!targetSurface){
    pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
    targetSurface = pSurface.get();
  }

  // Set the KalmanFitter options
  
  Acts::KalmanFitterOptions kfOptions = configureFit(tgContext, mfContext, calContext, targetSurface,
                                                     detail::SourceLinkType::xAODUnCalibMeas);
                                                     
  std::unique_ptr< MutableTrackContainer > tracks = std::make_unique< MutableTrackContainer >();
 
  
  auto result = m_directFitter->fit(sourceLinks.begin(),
				    sourceLinks.end(),
				    initialParams,
				    kfOptions,
				    surfaces,
				    *tracks.get());
   
  if (not result.ok()) {
    ATH_MSG_VERBOSE("Kalman Fitter on Seed has failed");
    return nullptr;
  } 
  return tracks; 
}

// extend a track fit to include an additional set of MeasurementBase objects
// re-implements the TrkFitterUtils/TrackFitter.cxx general code in a more
// mem efficient and stable way
// --------------------------------
std::unique_ptr<Trk::Track>
KalmanFitterTool::fit(const EventContext& ctx,
		      const Trk::Track& inputTrack,
		      const Trk::MeasurementSet& addMeasColl,
		      const Trk::RunOutlierRemoval /*runOutlier*/,
		      const Trk::ParticleHypothesis /*matEffects*/) const
{
  ATH_MSG_VERBOSE ("--> enter KalmanFitter::fit(Track,Meas'BaseSet,,)");
  ATH_MSG_VERBOSE ("    with Track from author = " << inputTrack.info().dumpInfo());

  // protection, if empty MeasurementSet
  if (addMeasColl.empty()) {
    ATH_MSG_DEBUG( "client tries to add an empty MeasurementSet to the track fit." );
    return fit(ctx,inputTrack);
  }

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || (inputTrack.measurementsOnTrack()->size() < 2 && addMeasColl.empty())) {
    ATH_MSG_DEBUG("called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input fails to provide track parameters for seeding the KF, reject fit");
    return nullptr;
  }

 
  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
  
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

  // Set the KalmanFitter options
  Acts::KalmanFitterOptions kfOptions = configureFit(tgContext, mfContext, calContext, pSurface.get(),
                                                     detail::SourceLinkType::TrkMeasurement);
     
  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(inputTrack);
  m_ATLASConverterTool->toSourceLinks(addMeasColl, trackSourceLinks);

  // protection against error in the conversion from Atlas masurement to Acts source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return nullptr;
  }
  const auto initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(*(inputTrack.perigeeParameters()), tgContext);

  MutableTrackContainer tracks;
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              initialParams, kfOptions, tracks);
  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                                Trk::TrackInfo::TrackFitter::KalmanFitter,
                                                detail::SourceLinkType::TrkMeasurement);
}

// extend a track fit to include an additional set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track>
KalmanFitterTool::fit(const EventContext& /*ctx*/,
		      const Trk::Track& /*inputTrack*/,
		      const Trk::PrepRawDataSet& /*addPrdColl*/,
		      const Trk::RunOutlierRemoval /*runOutlier*/,
		      const Trk::ParticleHypothesis /*matEffects*/) const
{
  ATH_MSG_DEBUG("Fit of Track with additional PrepRawDataSet not yet implemented");
  return nullptr;
}

// combined fit of two tracks
// --------------------------------
std::unique_ptr<Trk::Track>
KalmanFitterTool::fit(const EventContext& ctx,
		      const Trk::Track& intrk1,
		      const Trk::Track& intrk2,
		      const Trk::RunOutlierRemoval /*runOutlier*/,
		      const Trk::ParticleHypothesis hypothesis) const
{
  ATH_MSG_VERBOSE ("--> enter KalmanFitter::fit(Track,Track,)");
  ATH_MSG_VERBOSE ("    with Tracks from #1 = " << intrk1.info().dumpInfo()
                   << " and #2 = " << intrk2.info().dumpInfo()<<", "<<hypothesis);

  // protection, if empty track2
  if (!intrk2.measurementsOnTrack()) {
    ATH_MSG_DEBUG( "input #2 is empty try to fit track 1 alone" );
    return fit(ctx,intrk1);
  }

  // protection, if empty track1
  if (!intrk1.measurementsOnTrack()) {
    ATH_MSG_DEBUG( "input #1 is empty try to fit track 2 alone" );
    return fit(ctx,intrk2);
  }

  // protection against not having track parameters on the input track
  if (!intrk1.trackParameters() || intrk1.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input #1 fails to provide track parameters for seeding the KF, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
  
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

  // Set the KalmanFitter options
  Acts::KalmanFitterOptions kfOptions = configureFit(tgContext, mfContext, calContext, pSurface.get(),
                                                     detail::SourceLinkType::TrkMeasurement);

  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(intrk1);
  std::vector<Acts::SourceLink> trackSourceLinks2 = m_ATLASConverterTool->trkTrackToSourceLinks(intrk2);
  trackSourceLinks.insert(trackSourceLinks.end(), std::make_move_iterator(trackSourceLinks2.begin()), 
                          std::make_move_iterator(trackSourceLinks2.end()));
  // protection against error in the conversion from Atlas masurement to Acts source link
  if (trackSourceLinks.empty()) {
    ATH_MSG_DEBUG("input contain measurement but no source link created, probable issue with the converter, reject fit ");
    return nullptr;
  }

  const auto &initialParams = m_ATLASConverterTool->trkTrackParametersToActsParameters(*(intrk1.perigeeParameters()), tgContext);

  // The covariance from already fitted track are too small and would result an incorect smoothing.
  // We scale up the input covaraiance to avoid this.
  Acts::BoundSquareMatrix scaledCov = Acts::BoundSquareMatrix::Identity();
  for (int i=0; i<6; ++i) {
    double scale = m_option_seedCovarianceScale;
    (scaledCov)(i,i) = scale * initialParams.covariance().value()(i,i);
  }

  const Acts::BoundTrackParameters scaledInitialParams(initialParams.referenceSurface().getSharedPtr(),
                                                       initialParams.parameters(),
                                                       scaledCov, Acts::ParticleHypothesis::pion());


  MutableTrackContainer tracks{};
  // Perform the fit
  auto result = m_fitter->fit(trackSourceLinks.begin(), trackSourceLinks.end(),
                              scaledInitialParams, kfOptions, tracks);
  return m_ATLASConverterTool->convertFitResult(ctx, tracks, result,
                                                Trk::TrackInfo::TrackFitter::KalmanFitter,
                                                detail::SourceLinkType::TrkMeasurement);
}

std::unique_ptr< MutableTrackContainer >
KalmanFitterTool::fit(const EventContext& ctx,
		      const Seed &seed,
		      const Acts::BoundTrackParameters& initialParams,
		      const Acts::GeometryContext& tgContext,
		      const Acts::MagneticFieldContext& mfContext,
		      const Acts::CalibrationContext& calContext,
		      const DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap) const
{
  const Acts::TrackingGeometry *
     actsTrackingGeometry = m_trackingGeometryTool->trackingGeometry().get();
  if (!actsTrackingGeometry) {
     throw std::runtime_error("No Acts tracking geometry.");
  }

  std::vector<ATLASUncalibSourceLink> sourceLinks;
  sourceLinks.reserve(6);

  std::vector<const Acts::Surface*> surfaces;
  surfaces.reserve(6);
  
  const auto& sps = seed.sp();
  for (const xAOD::SpacePoint* sp : sps) {
    const auto& measurements = sp->measurements();
    for (const xAOD::UncalibratedMeasurement *umeas : measurements) {
      ATLASUncalibSourceLink el(makeATLASUncalibSourceLink(umeas));
      sourceLinks.emplace_back( el );
      surfaces.push_back(m_unalibMeasSurfAcc.get(umeas));
    }
  }
  return fit(ctx, sourceLinks, initialParams, tgContext, mfContext, calContext, detectorElementToGeometryIdMap, surfaces.front());
}
  StatusCode
  KalmanFitterTool::fit(const EventContext& /*ctx*/,
			const TrackContainer::ConstTrackProxy& /*track*/,          
			MutableTrackContainer& /*trackContainer*/) const
  {
    ATH_MSG_ERROR("Track refit method not implemented in KalmanFitterTool yet");
    return StatusCode::FAILURE;
  }
  
}
