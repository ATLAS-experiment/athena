/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GaussianSumFitterTool.h"

// ACTS
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFitting/GsfMixtureReduction.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackContainerUtils.h"

// PACKAGE
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsInterop/Logger.h"
#include "Acts/Propagator/DirectNavigator.hpp"
#include "src/detail/RefittingCalibrator.h"
// STL
#include <vector>
#include <type_traits>
#include <fstream>

#include "PathResolver/PathResolver.h"

namespace {
// Read an ATLAS Bethe-Heitler .par file (format: "n_cmps  degree\n[data]").
Acts::AtlasBetheHeitlerApprox::Data readBHParFile(const std::string& path) {
  std::ifstream fin(path);
  if (!fin) {
    throw std::invalid_argument("Could not open BH par file: " + path);
  }
  std::size_t n_cmps = 0, degree = 0;
  fin >> n_cmps >> degree;
  if (!fin || n_cmps == 0 || degree == 0) {
    throw std::invalid_argument("Bad header in BH par file: " + path);
  }
  Acts::AtlasBetheHeitlerApprox::Data data(n_cmps);
  for (auto& cmp : data) {
    cmp.weightCoeffs.resize(degree + 1);
    cmp.meanCoeffs.resize(degree + 1);
    cmp.varCoeffs.resize(degree + 1);
    for (double& c : cmp.weightCoeffs) { fin >> c; }
    for (double& c : cmp.meanCoeffs)   { fin >> c; }
    for (double& c : cmp.varCoeffs)    { fin >> c; }
  }
  if (!fin) {
    throw std::invalid_argument("Truncated data in BH par file: " + path);
  }
  return data;
}
} // anonymous namespace

namespace ActsTrk {

StatusCode GaussianSumFitterTool::initialize() {
  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  ATH_CHECK(m_trackingGeometrySvc.retrieve());
  ATH_CHECK(m_ctxProvider.initialize());
  ATH_CHECK(m_geometryConvTool.retrieve());
  ATH_CHECK(m_ROTcreator.retrieve(EnableTool{!m_ROTcreator.empty()}));
  m_logger = makeActsAthenaLogger(this, "Acts Gaussian Sum Refit");

  auto field = std::make_shared<ATLASMagneticFieldWrapper>();

  Acts::MultiEigenStepperLoop<> stepper(field);

  // Use the GeantSim Bethe-Heitler parameterisation, which clamps at X/X0=0.20
  // matching the behaviour in Trk::ElectronCombinedMaterialEffects. The files
  // store coefficients in logit(z) space; transform=true applies sigmoid/exp to
  // recover physical z in (0,1).
  const std::string bhLow  = PathResolver::find_file("GeantSim_LT01_cdf_nC6_O5.par", "DATAPATH");
  const std::string bhHigh = PathResolver::find_file("GeantSim_GT01_cdf_nC6_O5.par", "DATAPATH");
  ATH_MSG_INFO("ACTS GSF: loading GeantSim BH parameterisation (" << bhLow << ", " << bhHigh << ")");
  auto bha = std::make_shared<Acts::AtlasBetheHeitlerApprox>(
      readBHParFile(bhLow), readBHParFile(bhHigh),
      /*lowTransform=*/true, /*highTransform=*/true,
      /*lowLimit=*/0.1, /*highLimit=*/0.2, /*clampToRange=*/true,
      /*noChangeLimit=*/0.0001, /*singleGaussianLimit=*/0.002);

  
  if( m_useDirectNavigation ){
    // Direct Fitter
    Acts::DirectNavigator directNavigator( logger().cloneWithSuffix("DirectNavigator") );
    Acts::MultiEigenStepperLoop<> stepperDirect(field);
    Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::DirectNavigator> directPropagator(std::move(stepperDirect),
											    std::move(directNavigator),
											    logger().cloneWithSuffix("DirectPropagator"));
    m_directFitter = std::make_unique<DirectFitter>(std::move(directPropagator), bha,
						    logger().cloneWithSuffix("DirectGaussianSumFitter"));

  } else {
    Acts::Navigator navigator(Acts::Navigator::Config{ m_trackingGeometrySvc->trackingGeometry() },
                              logger().cloneWithSuffix("Navigator") );
    Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::Navigator> propagator(std::move(stepper), 
                      std::move(navigator),
                      logger().cloneWithSuffix("Prop"));
    m_fitter = std::make_unique<Fitter>(std::move(propagator), bha,
              logger().cloneWithSuffix("GaussianSumFitter"));
  }

  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;

  FitterExtension_t gsfExtensionsTemplate;
  gsfExtensionsTemplate.outlierFinder.connect<&ActsTrk::detail::FitterHelperFunctions::ATLASOutlierFinder::operator()<ActsTrk::MutableTrackStateBackend>>(&m_outlierFinder);
  gsfExtensionsTemplate.updater.connect<&ActsTrk::detail::FitterHelperFunctions::gainMatrixUpdate<ActsTrk::MutableTrackStateBackend>>();
  gsfExtensionsTemplate.mixtureReducer.connect<&Acts::reduceMixtureWithKLDistance>();

  /// Configure the fit extensions for Trk::MeasuremenBase pass through fits.
  {
    m_trkSurfAcc = detail::TrkMeasSurfaceAccessor{m_geometryConvTool.get()};

    FitterExtension_t& configureMe = m_gsfExtensions[Acts::toUnderlying(detail::SourceLinkType::TrkMeasurement)];
    configureMe = gsfExtensionsTemplate;
    //coverity has hard time matching arguments to these passed parameters
    //coverity[RW.NO_MATCHING_FUNCTION:FALSE]
    configureMe.calibrator.connect<&detail::TrkMeasurementCalibrator::calibrate<TrackState_t>>(&m_trkCalibrator);
    configureMe.surfaceAccessor.connect<&detail::TrkMeasSurfaceAccessor::operator()>(&m_trkSurfAcc);
  }
  /// Configure the fit extensions for the Trk::PrepRawData fits
  {
     m_prdCalibrator = detail::TrkPrepRawDataCalibrator{m_geometryConvTool.get(), m_ROTcreator.get()};
     m_prdSurfAcc = detail::TrkPrepRawDataSurfaceAcc{m_geometryConvTool.get()};

     FitterExtension_t& configureMe = m_gsfExtensions[Acts::toUnderlying(detail::SourceLinkType::TrkPrepRawData)];
     configureMe = gsfExtensionsTemplate;
     //coverity[RW.NO_MATCHING_FUNCTION:FALSE]
     configureMe.calibrator.connect<&detail::TrkPrepRawDataCalibrator::calibrate<TrackState_t>>(&m_prdCalibrator);
     configureMe.surfaceAccessor.connect<&detail::TrkPrepRawDataSurfaceAcc::operator()>(&m_prdSurfAcc);
  }
  /// Configure the fit extensions for the uncalibrated measurement fits
  {
    m_unalibMeasSurfAcc = detail::xAODUncalibMeasSurfAcc{m_trackingGeometrySvc.get()}; 
    m_uncalibMeasCalibrator = xAODUnCalibrator_t::NoCalibration(m_trackingGeometrySvc.get());

    m_refitCalibrator = std::make_unique<detail::RefittingCalibrator>(m_geometryConvTool.get(), m_ROTcreator.get());
    m_refitCalibrator->connect<&xAODUnCalibrator_t::calibrate>(xAOD::UncalibMeasType::PixelClusterType, &m_uncalibMeasCalibrator);
    m_refitCalibrator->connect<&xAODUnCalibrator_t::calibrate>(xAOD::UncalibMeasType::StripClusterType, &m_uncalibMeasCalibrator);
    
    FitterExtension_t& configureMe = m_gsfExtensions[Acts::toUnderlying(detail::SourceLinkType::xAODUnCalibMeas)];
    configureMe = gsfExtensionsTemplate;
    configureMe.surfaceAccessor.connect<&detail::xAODUncalibMeasSurfAcc::operator()>(&m_unalibMeasSurfAcc);
    configureMe.calibrator.connect<&detail::RefittingCalibrator::calibrate>(m_refitCalibrator.get());
  }

  if(m_option_componentMergeMethod == "Mean" ){
    m_componentMergeMethod = Acts::ComponentMergeMethod::eMean;
  }else if(m_option_componentMergeMethod == "MaxWeight"){
    m_componentMergeMethod = Acts::ComponentMergeMethod::eMaxWeight;
  }else{
    throw std::runtime_error("Unknown option for ComponentMergeMethod: " + m_option_componentMergeMethod.value());
  }
  
  ATH_MSG_INFO("ACTS GSF direct nav   " << m_useDirectNavigation.value());
  ATH_MSG_INFO("ACTS GSF max cmps     " << m_maxComponents.value());
  ATH_MSG_INFO("ACTS GSF merge meth   " << m_option_componentMergeMethod.value());
  ATH_MSG_INFO("ACTS GSF weight ctf   " << m_weightCutOff.value());
  ATH_MSG_INFO("ACTS GSF outlier chi2 " << m_option_outlierChi2Cut.value());
  
  return StatusCode::SUCCESS;
}

GaussianSumFitterTool::FitterOptions_t
GaussianSumFitterTool::configureFit(const Acts::GeometryContext& tgContext,
				      const Acts::MagneticFieldContext& mfContext,
				      const Acts::CalibrationContext& calContext,
				      const Acts::PerigeeSurface& surface,
              detail::SourceLinkType slType) const
{ 
  //slType can be 3
  const auto& gsfExtensions = m_gsfExtensions.at(Acts::toUnderlying(slType));

  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;

  FitterOptions_t gsfOptions(tgContext, mfContext, calContext);
  gsfOptions.extensions=gsfExtensions;
  gsfOptions.propagatorPlainOptions=std::move(propagationOption);
  gsfOptions.referenceSurface = &surface;

  // Set abortOnError to false, else the refitting crashes if no forward propagation is done. Here, we just skip the event and continue.
  gsfOptions.abortOnError = false;
  gsfOptions.maxComponents = m_maxComponents;
  gsfOptions.weightCutoff = m_weightCutOff;
  gsfOptions.componentMergeMethod = m_componentMergeMethod;

  return gsfOptions;
}

// Acts track refit
std::unique_ptr< ActsTrk::MutableTrackContainer >
GaussianSumFitterTool::fit(const ActsTrk::Seed & /*seed*/,
        const Acts::BoundTrackParameters& /*initialParams*/,
        const Acts::GeometryContext& /*tgContext*/,
        const Acts::MagneticFieldContext& /*mfContext*/,
	const Acts::CalibrationContext& /*calContext*/,
	const Acts::Surface& /*targetSurface*/) const
{
  ATH_MSG_VERBOSE("ACTS seed refit is not implemented in GaussianSumFitterTool");
  return nullptr;
}

std::unique_ptr< ActsTrk::MutableTrackContainer >
GaussianSumFitterTool::fit(const std::vector< const xAOD::UncalibratedMeasurement*> & /*clusterList*/,
         const Acts::BoundTrackParameters& /*initialParams*/,
         const Acts::GeometryContext& /*tgContext*/,
         const Acts::MagneticFieldContext& /*mfContext*/,
         const Acts::CalibrationContext& /*calContext*/,         
         const Acts::Surface* /*targetSurface*/) const
{
  ATH_MSG_VERBOSE("ACTS uncalib slink refit is not implemented in GaussianSumFitterTool");  
  return nullptr;
}


StatusCode GaussianSumFitterTool::fit(
  const EventContext& ctx,  
  const ActsTrk::TrackContainer::ConstTrackProxy& track,          
  ActsTrk::MutableTrackContainer& trackContainer,
  const Acts::PerigeeSurface& pSurface) const {
  ATH_MSG_VERBOSE("GaussianSumFitterTool::fit(TrackProxy) called");

  const Acts::BoundTrackParameters initialParams = track.createParametersAtReference();
  std::vector<Acts::SourceLink> sourceLinks;

  for (auto ts : track.trackStates()){
    if (!ts.hasCalibrated()) {
      continue;
    } 
    if (ts.typeFlags().hasMeasurement()) {
      sourceLinks.push_back(ts.getUncalibratedSourceLink());
    }
  }

  if (sourceLinks.size() < 2) {
    ATH_MSG_DEBUG("called to refit 0 or 1 sourceLink with too little information, reject fit");
    return StatusCode::SUCCESS;
  }

  const Acts::GeometryContext tgContext{m_ctxProvider.getGeometryContext(ctx)};
  const Acts::MagneticFieldContext mfContext{m_ctxProvider.getMagneticFieldContext(ctx)};
  const Acts::CalibrationContext calContext{m_ctxProvider.getCalibrationContext(ctx)};

  std::unique_ptr< ActsTrk::MutableTrackContainer > refittedTracks = 
    fit(sourceLinks, initialParams, tgContext, mfContext, calContext, &pSurface);

  if (!refittedTracks) {
    ATH_MSG_WARNING("Refit failed");
    return StatusCode::SUCCESS;
  }

  TrackContainerUtils::addFitterTypeProperty(*refittedTracks);
  trackContainer.ensureDynamicColumns(*refittedTracks);
  
  for (auto trkProxy : *refittedTracks) {
    TrackContainerUtils::setFitterType(trkProxy, xAOD::TrackFitter::GaussianSumFilter);

    auto destProxy = trackContainer.getTrack(trackContainer.addTrack());
    destProxy.copyFrom(trkProxy);
  }

  return StatusCode::SUCCESS;
}

//! fit a set of source links
std::unique_ptr<MutableTrackContainer> 
GaussianSumFitterTool::fit(const std::vector<Acts::SourceLink>& sourceLinks,
                           const Acts::BoundTrackParameters& initialParams,
                           const Acts::GeometryContext& tgContext,
                           const Acts::MagneticFieldContext& mfContext,
                           const Acts::CalibrationContext& calContext,
                           const Acts::Surface* /*targetSurface*/ ) const {
  if (sourceLinks.empty()) {
    ATH_MSG_DEBUG("No measurements given. Nothing to do");
    return nullptr;
  }
  // Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  detail::SourceLinkType slType = detail::MeasurementCalibratorBase::getType(sourceLinks.front());

  FitterOptions_t gsfOptions = configureFit(tgContext, mfContext, calContext, *pSurface, slType);

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
      initialParams, gsfOptions, surfaces, *tracks).ok();

  } else {
    fitSuccess = m_fitter->fit(sourceLinks.begin(), sourceLinks.end(), 
      initialParams, gsfOptions, *tracks).ok();
  }

  if (!fitSuccess) {
    ATH_MSG_VERBOSE("Fitter has failed");
    return nullptr;
  }

  TrackContainerUtils::addFitterTypeProperty(*tracks);
  for (auto trkProxy : *tracks) {
    TrackContainerUtils::setFitterType(trkProxy, xAOD::TrackFitter::GaussianSumFilter);
  }
  return tracks;
}

/// Private access to the logger
const Acts::Logger& 
GaussianSumFitterTool::logger() const 
{ 
  return *m_logger;
}

}
