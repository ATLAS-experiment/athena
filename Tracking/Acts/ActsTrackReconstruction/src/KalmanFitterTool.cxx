/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/KalmanFitterTool.h"

// ACTS
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

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackContainerUtils.h"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsInterop/Logger.h"

#include "Acts/Propagator/DirectNavigator.hpp"

namespace ActsTrk {


StatusCode KalmanFitterTool::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_trackingGeometrySvc.retrieve());
  ATH_CHECK(m_geometryConvTool.retrieve());
  ATH_CHECK(m_ROTcreator.retrieve(EnableTool{!m_ROTcreator.empty()}));
  m_logger = makeActsAthenaLogger(this, "KalmanRefit");

  auto field = std::make_shared<ATLASMagneticFieldWrapper>();

  // Fitter
  Acts::SympyStepper stepper(field);
  if (m_useDirectNavigation) {
    // Direct Fitter
    Acts::DirectNavigator directNavigator( logger().cloneWithSuffix("DirectNavigator") );
    Acts::Propagator<Acts::SympyStepper, Acts::DirectNavigator> directPropagator(std::move(stepper),
                      std::move(directNavigator),
                      logger().cloneWithSuffix("DirectPropagator"));

    m_directFitter = std::make_unique<DirectFitter>(std::move(directPropagator),
                logger().cloneWithSuffix("DirectKalmanFitter"));

  } else {
    Acts::Navigator navigator( Acts::Navigator::Config{ m_trackingGeometrySvc->trackingGeometry() },
            logger().cloneWithSuffix("Navigator"));
    Acts::Propagator<Acts::SympyStepper, Acts::Navigator> propagator(stepper, 
                      std::move(navigator),
                      logger().cloneWithSuffix("Prop"));

    m_fitter = std::make_unique<Fitter>(std::move(propagator),
            logger().cloneWithSuffix("KalmanFitter"));
  }

  ///
  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;
  m_reverseFilteringLogic.momentumMax = m_option_ReverseFilteringPt;

  FitterExtension_t extensionTemplate{};
  extensionTemplate.outlierFinder.connect<&detail::FitterHelperFunctions::ATLASOutlierFinder::operator()<MutableTrackStateBackend>>(&m_outlierFinder);
  extensionTemplate.reverseFilteringLogic.connect<&detail::FitterHelperFunctions::ReverseFilteringLogic::operator()<MutableTrackStateBackend>>(&m_reverseFilteringLogic);
  extensionTemplate.updater.connect<&detail::FitterHelperFunctions::gainMatrixUpdate<MutableTrackStateBackend>>();
  extensionTemplate.smoother.connect<&detail::FitterHelperFunctions::mbfSmoother<MutableTrackStateBackend>>();

  /// Configure the fit extensions for Trk::MeasuremenBase pass through fits.
  {
    m_trkSurfAcc = detail::TrkMeasSurfaceAccessor{m_geometryConvTool.get()};

    FitterExtension_t& configureMe = m_kfExtensions[Acts::toUnderlying(detail::SourceLinkType::TrkMeasurement)];
    configureMe = extensionTemplate;
    configureMe.calibrator.connect<&detail::TrkMeasurementCalibrator::calibrate<TrackState_t>>(&m_trkCalibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkMeasSurfaceAccessor::operator()>(&m_trkSurfAcc);
  }
  /// Configure the fit extensions for the Trk::PrepRawData fits
  {
     m_prdSurfAcc = detail::TrkPrepRawDataSurfaceAcc{m_geometryConvTool.get()};
     m_prdCalibrator = detail::TrkPrepRawDataCalibrator{m_geometryConvTool.get(), m_ROTcreator.get()};

     FitterExtension_t& configureMe = m_kfExtensions[Acts::toUnderlying(detail::SourceLinkType::TrkPrepRawData)];
     configureMe = extensionTemplate;
     configureMe.calibrator.connect<&detail::TrkPrepRawDataCalibrator::calibrate<TrackState_t>>(&m_prdCalibrator);
     configureMe.surfaceAccessor.connect<&detail::TrkPrepRawDataSurfaceAcc::operator()>(&m_prdSurfAcc);
  }
  /// Configure the fit extensions for the uncalibrated measurement fits
  {
    m_unalibMeasSurfAcc = detail::xAODUncalibMeasSurfAcc{m_trackingGeometrySvc.get()}; 
    m_uncalibMeasCalibrator = xAODUnCalibrator_t::NoCalibration(m_trackingGeometrySvc.get());

    FitterExtension_t& configureMe = m_kfExtensions[Acts::toUnderlying(detail::SourceLinkType::xAODUnCalibMeas)];
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
  
  
  const auto& kfExtensions = m_kfExtensions[Acts::toUnderlying(slType)];

  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;
  // Set the KalmanFitter options
  return FitterOptions_t{tgContext, mfContext, calContext,
                         kfExtensions, propagationOption,
                         surface};
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr< MutableTrackContainer >
KalmanFitterTool::fit(const std::vector< const xAOD::UncalibratedMeasurement*> & clusterList,
                      const Acts::BoundTrackParameters& initialParams,
                      const Acts::GeometryContext& tgContext,
                      const Acts::MagneticFieldContext& mfContext,
                      const Acts::CalibrationContext& calContext,                      
                      const Acts::Surface* targetSurface) const{
  ATH_MSG_DEBUG("--> entering KalmanFitter::fit(xAODMeasure...things,TP,)");
       
  std::vector<Acts::SourceLink> sourceLinks;
  sourceLinks.reserve(clusterList.size()); 

  detail::MeasurementCalibratorBase::pack(clusterList, sourceLinks);
  
  return fit(sourceLinks, initialParams, tgContext, mfContext, calContext, targetSurface);
}

std::unique_ptr< MutableTrackContainer >
KalmanFitterTool::fit(const Seed &seed,
                      const Acts::BoundTrackParameters& initialParams,
                      const Acts::GeometryContext& tgContext,
                      const Acts::MagneticFieldContext& mfContext,
                      const Acts::CalibrationContext& calContext,
		              const Acts::Surface& targetSurface) const {
  
  std::vector<const xAOD::UncalibratedMeasurement*> measList;
  measList.reserve(6);

  const auto& sps = seed.sp();
  for (const xAOD::SpacePoint* sp : sps) {
    const auto& measurements = sp->measurements();
    for (const xAOD::UncalibratedMeasurement *umeas : measurements) {     
      measList.push_back(umeas);
    }
  }
  return fit(measList, initialParams, tgContext, mfContext, calContext, &targetSurface);
}
  
  StatusCode
  KalmanFitterTool::fit(const EventContext& /*ctx*/,
      const TrackContainer::ConstTrackProxy& /*track*/,          
      MutableTrackContainer& /*trackContainer*/,
      const Acts::PerigeeSurface& /*pSurface*/) const
  {
    ATH_MSG_ERROR("Track refit method not implemented in KalmanFitterTool yet");
    return StatusCode::FAILURE;
  }

std::unique_ptr< MutableTrackContainer >
KalmanFitterTool::fit(const std::vector<Acts::SourceLink>& sourceLinks,
                      const Acts::BoundTrackParameters& initialParams,
                      const Acts::GeometryContext& tgContext,
                      const Acts::MagneticFieldContext& mfContext,
                      const Acts::CalibrationContext& calContext,
                      const Acts::Surface* targetSurface) const {

  if (sourceLinks.empty()) {
    ATH_MSG_DEBUG("No measurements given. Nothing to do");
    return nullptr;
  }
  
  // Construct a perigee surface as the target surface if none is provided
  std::shared_ptr<Acts::Surface> pSurface{nullptr};
  if (!targetSurface){
    pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
    targetSurface = pSurface.get();
  }

  detail::SourceLinkType slType = detail::MeasurementCalibratorBase::getType(sourceLinks.front());

  Acts::KalmanFitterOptions kfOptions = configureFit(tgContext, mfContext, calContext, 
                                             targetSurface, slType);

  ActsTrk::MutableTrackBackend trackContainerBackEnd;
  ActsTrk::MutableTrackStateBackend multiTrajBackEnd;
  auto tracks = std::make_unique<MutableTrackContainer>(std::move(trackContainerBackEnd),
                                                        std::move(multiTrajBackEnd));

  bool fitSuccess = false;                                                      
  if (m_useDirectNavigation) {

    std::vector<const Acts::Surface*> surfaces;
    surfaces.reserve(sourceLinks.size());
    switch (slType) {
      case detail::SourceLinkType::TrkMeasurement: {
        std::ranges::for_each(sourceLinks, [this, &surfaces](const Acts::SourceLink& sl) {
          surfaces.push_back(m_trkSurfAcc(sl));
        });
        break;
      }
      case detail::SourceLinkType::TrkPrepRawData: {
        std::ranges::for_each(sourceLinks, [this, &surfaces](const Acts::SourceLink& sl) {
          surfaces.push_back(m_prdSurfAcc(sl));
        });
        break;
      }
      case detail::SourceLinkType::xAODUnCalibMeas: {
        std::ranges::for_each(sourceLinks, [this, &surfaces](const Acts::SourceLink& sl) {
          surfaces.push_back(m_unalibMeasSurfAcc(sl));
        });
        break;
      }
      default:
        ATH_MSG_ERROR("Unsupported source link type for KalmanFitterTool::fit");
        return nullptr;
    }

    fitSuccess = m_directFitter->fit(sourceLinks.begin(), sourceLinks.end(), 
      initialParams, kfOptions, surfaces, *tracks.get()).ok();

  } else {

    fitSuccess = m_fitter->fit(sourceLinks.begin(), sourceLinks.end(), 
      initialParams, kfOptions, *tracks.get()).ok();
  }

  if (!fitSuccess) {
    ATH_MSG_VERBOSE("Kalman Fitter on Seed has failed");
    return nullptr;
  }

  TrackContainerUtils::addFitterTypeProperty(*tracks);
  for (auto trkProxy : *tracks) {
    TrackContainerUtils::setFitterType(trkProxy, xAOD::TrackFitter::KalmanFitter);
  }
  
  return tracks; 
}
  
}
