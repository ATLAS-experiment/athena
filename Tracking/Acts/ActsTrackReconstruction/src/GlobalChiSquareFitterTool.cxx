/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalChiSquareFitterTool.h"

// ACTS
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFitting/GlobalChiSquareFitter.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackContainerUtils.h"

// PACKAGE
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsInterop/Logger.h"

namespace ActsTrk {

StatusCode GlobalChiSquareFitterTool::initialize() {

  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_trackingGeometrySvc.retrieve());
  ATH_CHECK(m_ROTcreator.retrieve(EnableTool{!m_ROTcreator.empty()}));
  ATH_CHECK(m_geometryConvTool.retrieve());
  ATH_CHECK(m_muonCalibrator.retrieve(EnableTool{!m_muonCalibrator.empty()}));

  m_logger = makeActsAthenaLogger(this, "Gx2fRefit");
  if (!m_doStraightLine){
      // Fitter
      CurvedPropagator_t::Stepper stepper{std::make_shared<ATLASMagneticFieldWrapper>()};
      Acts::Navigator::Config navConfig{m_trackingGeometrySvc->trackingGeometry()};
      Acts::Navigator navigator(std::move(navConfig), logger().cloneWithSuffix("Navigator"));
      CurvedPropagator_t propagator{stepper, std::move(navigator), logger().cloneWithSuffix("Prop")};

      m_fitter = std::make_unique<CurvedFitter_t>(std::move(propagator), 
                                                  logger().cloneWithSuffix("GlobalChiSquareFitter"));
  } else {
      Acts::StraightLineStepper stepper{};
      Acts::Navigator::Config navConfig{m_trackingGeometrySvc->trackingGeometry()};
      Acts::Navigator navigator(std::move(navConfig), logger().cloneWithSuffix("Navigator"));
      StraightPropagator_t propagator{stepper, std::move(navigator), logger().cloneWithSuffix("Prop")};

      m_slFitter = std::make_unique<StraightFitter_t>(std::move(propagator), 
                                                      logger().cloneWithSuffix("GlobalChiSquareFitter"));
  }


  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;

  Gx2FitterExtension_t extensionTemplate{};
  extensionTemplate.outlierFinder.connect<&detail::FitterHelperFunctions::ATLASOutlierFinder::operator()
                                            <MutableTrackStateBackend>>(&m_outlierFinder);
 
  /// Configure the fit extensions for Trk::MeasuremenBase pass through fits.
  {
    m_trkMeasCalibrator = detail::TrkMeasurementCalibrator{};
    m_trkMeasSurfAcc = detail::TrkMeasSurfaceAccessor{m_geometryConvTool.get()};

    Gx2FitterExtension_t& configureMe = m_gx2fExtensions[Acts::toUnderlying(detail::SourceLinkType::TrkMeasurement)];
    configureMe = extensionTemplate;
    configureMe.calibrator.connect<&detail::TrkMeasurementCalibrator::calibrate<TrackState_t>>(&m_trkMeasCalibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkMeasSurfaceAccessor::operator()>(&m_trkMeasSurfAcc);
  }
  /// Configure the fit extensions for Trk::PrepRawData fits
  {
    m_prdCalibrator = detail::TrkPrepRawDataCalibrator{m_geometryConvTool.get(), m_ROTcreator.get()};
    m_prdSurfaceAcc = detail::TrkPrepRawDataSurfaceAcc{m_geometryConvTool.get()};

    Gx2FitterExtension_t& configureMe = m_gx2fExtensions[Acts::toUnderlying(detail::SourceLinkType::TrkPrepRawData)];
    configureMe = extensionTemplate;
    configureMe.calibrator.connect<&detail::TrkPrepRawDataCalibrator::calibrate<TrackState_t>>(&m_prdCalibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkPrepRawDataSurfaceAcc::operator()>(&m_prdSurfaceAcc);
  }
  {
    m_unalibMeasSurfAcc = detail::xAODUncalibMeasSurfAcc{m_trackingGeometrySvc.get()};

    /// Needs to be filled with live.
    Gx2FitterExtension_t& configureMe = m_gx2fExtensions[Acts::toUnderlying(detail::SourceLinkType::xAODUnCalibMeas)];
    configureMe = extensionTemplate;
    configureMe.surfaceAccessor.connect<&detail::xAODUncalibMeasSurfAcc::operator()>(&m_unalibMeasSurfAcc);
    configureMe.calibrator.connect<&detail::xAODUncalibMeasCalibrator::calibrate>(&m_uncalibMeasCalibrator);
    m_idCalibrator = xAODItkCalibrator_t::NoCalibration(m_trackingGeometrySvc.get());

    /// Connect the muon types with the muon calibrator
    using enum xAOD::UncalibMeasType;
    if (m_muonCalibrator.isEnabled()) {
      for (const auto muonType : {MdtDriftCircleType, RpcStripType, TgcStripType, MMClusterType, sTgcStripType}) {
        m_uncalibMeasCalibrator.connect<&MuonR4::ISpacePointCalibrator::calibrateSourceLink>(muonType, m_muonCalibrator.get());
      }
    }
    for (const auto idType: {PixelClusterType, StripClusterType, HGTDClusterType}) {
      m_uncalibMeasCalibrator.connect<&xAODItkCalibrator_t::calibrate>(idType, &m_idCalibrator);
    }
    
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
    propagationOption.maxTargetSkipping = m_option_maxNavSurfaces;
    // Set the Gx2Fitter options
    return Gx2FitterOptions_t{tgContext, mfContext, calContext, 
                              m_gx2fExtensions.at(Acts::toUnderlying(slType)), //slType can be 3
                              std::move(propagationOption),
                              surface, m_option_includeScat, 
                              m_option_includeELoss,
                              Acts::FreeToBoundCorrection{m_doJacobianCorr},
                              m_nIterMax};                           
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<MutableTrackContainer> GlobalChiSquareFitterTool::fit(    
    const std::vector<const xAOD::UncalibratedMeasurement*>& measList,
    const Acts::BoundTrackParameters& initialParams,
    const Acts::GeometryContext& tgContext,
    const Acts::MagneticFieldContext& mfContext,
    const Acts::CalibrationContext& calContext,   
    const Acts::Surface* targetSurface) const {
  
  std::vector<Acts::SourceLink> sourceLinks{};
  detail::MeasurementCalibratorBase::pack(measList, sourceLinks);

  return fit(sourceLinks, initialParams, tgContext, mfContext, calContext, targetSurface);
}

std::unique_ptr<MutableTrackContainer> GlobalChiSquareFitterTool::fit(
    const Seed& seed,
    const Acts::BoundTrackParameters& initialParams,
    const Acts::GeometryContext& tgContext,
    const Acts::MagneticFieldContext& mfContext,
    const Acts::CalibrationContext& calContext,
    const Acts::Surface& targetSurface) const {
  
  std::vector<const xAOD::UncalibratedMeasurement*> sourceLinks;
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

std::unique_ptr<MutableTrackContainer> 
GlobalChiSquareFitterTool::fit(const std::vector<Acts::SourceLink>& sourceLinks,
                               const Acts::BoundTrackParameters& initialParams,
                               const Acts::GeometryContext& tgContext,
                               const Acts::MagneticFieldContext& mfContext,
                               const Acts::CalibrationContext& calContext,
                               const Acts::Surface* targetSurface) const {
  if (sourceLinks.empty()) {
      ATH_MSG_DEBUG("No measurements given. Nothing to do");
      return nullptr;
  }
  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::Surface> pSurface{};
  if (!targetSurface) {
    pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
    targetSurface = pSurface.get();
  }

  detail::SourceLinkType slType = detail::MeasurementCalibratorBase::getType(sourceLinks.front());

  Gx2FitterOptions_t kfOptions = configureFit(tgContext, mfContext, calContext, 
                                              targetSurface, slType);

  ActsTrk::MutableTrackBackend trackContainerBackEnd;
  ActsTrk::MutableTrackStateBackend multiTrajBackEnd;
  auto tracks = std::make_unique<MutableTrackContainer>(std::move(trackContainerBackEnd),
                                                        std::move(multiTrajBackEnd));
  
  // Perform the fit
  bool ok = false;
  if (m_fitter) [[likely]]
    ok = m_fitter->fit(sourceLinks.begin(), sourceLinks.end(), initialParams, kfOptions, *tracks).ok();
  else
    ok = m_slFitter->fit(sourceLinks.begin(), sourceLinks.end(), initialParams, kfOptions, *tracks).ok();

  if (not ok) {
      ATH_MSG_VERBOSE("Global chi2 fit failed");
      return nullptr;
  }

  TrackContainerUtils::addFitterTypeProperty(*tracks);
  for (auto trkProxy : *tracks) {
    TrackContainerUtils::setFitterType(trkProxy, xAOD::TrackFitter::GlobalChi2Fitter);
  }

  return tracks;
}
}  // namespace ActsTrk
