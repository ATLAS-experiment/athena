/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GaussianSumFitterTool.h"

// ATHENA
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TrkTrack/TrackStateOnSurface.h"

// ACTS
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFitting/GsfMixtureReduction.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "ActsEvent/TrackContainer.h"

// PACKAGE
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsInterop/Logger.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/Propagator/DirectNavigator.hpp"
#include "src/detail/RefittingCalibrator.h"
#include "src/detail/OnTrackCalibrator.h"
// STL
#include <vector>
#include <bitset>
#include <type_traits>
#include <system_error>
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

  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_geometryConvTool.retrieve());
  ATH_CHECK(m_trkSummaryTool.retrieve(EnableTool{!m_refitOnly}));
  if (m_refitOnly) {
    ATH_MSG_INFO("Running GSF without track summary");
  }
  ATH_CHECK(m_ROTcreator.retrieve(EnableTool{!m_ROTcreator.empty()}));

  m_logger = makeActsAthenaLogger(this, "Acts Gaussian Sum Refit");

  auto field = std::make_shared<ATLASMagneticFieldWrapper>();
  Acts::MultiEigenStepperLoop<> stepper(field);
  Acts::Navigator navigator( Acts::Navigator::Config{ m_trackingGeometryTool->trackingGeometry() },
                             logger().cloneWithSuffix("Navigator") );
  Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::Navigator> propagator(std::move(stepper), 
                     std::move(navigator),
                     logger().cloneWithSuffix("Prop"));

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
  m_fitter = std::make_unique<Fitter>(std::move(propagator), bha,
              logger().cloneWithSuffix("GaussianSumFitter"));

  // Direct Fitter
  if( m_useDirectNavigation ){
    Acts::DirectNavigator directNavigator( logger().cloneWithSuffix("DirectNavigator") );
    Acts::MultiEigenStepperLoop<> stepperDirect(field);
    Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::DirectNavigator> directPropagator(std::move(stepperDirect),
											    std::move(directNavigator),
											    logger().cloneWithSuffix("DirectPropagator"));
    m_directFitter = std::make_unique<DirectFitter>(std::move(directPropagator), bha,
						    logger().cloneWithSuffix("DirectGaussianSumFitter"));

  }

  m_gsfExtensions.updater.connect<&ActsTrk::detail::FitterHelperFunctions::gainMatrixUpdate<ActsTrk::MutableTrackStateBackend>>();
  m_calibrator = std::make_unique<ActsTrk::detail::TrkMeasurementCalibrator>();
  m_gsfExtensions.calibrator.connect<&ActsTrk::detail::TrkMeasurementCalibrator::calibrate<TrackState_t>>(m_calibrator.get());

  m_surfaceAccessor = detail::TrkMeasSurfaceAccessor{m_geometryConvTool.get()};
  m_gsfExtensions.surfaceAccessor.connect<&detail::TrkMeasSurfaceAccessor::operator()>(&m_surfaceAccessor);
  m_gsfExtensions.mixtureReducer.connect<&Acts::reduceMixtureWithKLDistance>();
  
  m_outlierFinder.StateChiSquaredPerNumberDoFCut = m_option_outlierChi2Cut;
  m_gsfExtensions.outlierFinder.connect<&ActsTrk::detail::FitterHelperFunctions::ATLASOutlierFinder::operator()<ActsTrk::MutableTrackStateBackend>>(&m_outlierFinder);
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

// refit a track
// -------------------------------------------------------
std::unique_ptr<Trk::Track>
GaussianSumFitterTool::fit(const EventContext& ctx,
			   const Trk::Track& inputTrack,
			   const Trk::RunOutlierRemoval /*runOutlier*/,
			   const Trk::ParticleHypothesis /*prtHypothesis*/) const
{

  
  std::unique_ptr<Trk::Track> track = nullptr;
  ATH_MSG_VERBOSE ("--> enter GaussianSumFitter::fit(Track,,)    with Track from author = "
       << inputTrack.info().dumpInfo());

  // protection against not having measurements on the input track
  if (!inputTrack.measurementsOnTrack() || inputTrack.measurementsOnTrack()->size() < 2) {
    ATH_MSG_DEBUG("called to refit empty track or track with too little information, reject fit");
    return nullptr;
  }

  // protection against not having track parameters on the input track
  if (!inputTrack.trackParameters() || inputTrack.trackParameters()->empty()) {
    ATH_MSG_DEBUG("input fails to provide track parameters for seeding the GSF, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3::Zero());
  
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};
    
  // Set the GaussianSumFitter options
  Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>
    gsfOptions = prepareOptions(tgContext, 
				mfContext, 
				calContext, 
				*pSurface);
  gsfOptions.maxComponents = m_maxComponents;
  gsfOptions.weightCutoff = m_weightCutOff;
  gsfOptions.componentMergeMethod = m_componentMergeMethod;
  

  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(inputTrack);
  const auto initialParams = m_geometryConvTool->convertTrackParametersToActs(ctx, *inputTrack.perigeeParameters());

  return performFit(ctx, 
		    gsfOptions,
		    trackSourceLinks, 
		    initialParams);
}

// fit a set of MeasurementBase objects
// --------------------------------
std::unique_ptr<Trk::Track>
GaussianSumFitterTool::fit(const EventContext& ctx,
			   const Trk::MeasurementSet& inputMeasSet,
			   const Trk::TrackParameters& estimatedStartParameters,
			   const Trk::RunOutlierRemoval /*runOutlier*/,
			   const Trk::ParticleHypothesis /*matEffects*/) const
{
  std::unique_ptr<Trk::Track> track = nullptr;
  // protection against not having measurements on the input track
  if (inputMeasSet.size() < 2) {
    ATH_MSG_DEBUG("called to refit empty measurement set or a measurement set with too little information, reject fit");
    return nullptr;
  }

  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
												   Acts::Vector3::Zero());
  
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};
                         
  // Set the GaussianSumFitter options
  Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>
    gsfOptions = prepareOptions(tgContext, 
				mfContext, 
				calContext, 
				*pSurface);

  // Set abortOnError to false, else the refitting crashes if no forward propagation is done. Here, we just skip the event and continue.
  gsfOptions.abortOnError = false;
  
  std::vector< Acts::SourceLink > trackSourceLinks;
  detail::MeasurementCalibratorBase::pack(inputMeasSet, trackSourceLinks);

  const auto initialParams = m_geometryConvTool->convertTrackParametersToActs(ctx, estimatedStartParameters);
  
  if(m_useDirectNavigation){
    
    std::vector<const Acts::Surface*> surfaces;
    surfaces.reserve(inputMeasSet.size());
    std::ranges::transform(trackSourceLinks, std::back_inserter(surfaces), m_surfaceAccessor);
    
    return performDirectFit(ctx,
			    gsfOptions,
			    trackSourceLinks,
			    initialParams,
			    surfaces);
  }else{
    return performFit(ctx,
		      gsfOptions,
		      trackSourceLinks,
		      initialParams);
  }
}

// fit a set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track>
GaussianSumFitterTool::fit(const EventContext& /*ctx*/,
			   const Trk::PrepRawDataSet& /*inputPRDColl*/,
			   const Trk::TrackParameters& /*estimatedStartParameters*/,
			   const Trk::RunOutlierRemoval /*runOutlier*/,
			   const Trk::ParticleHypothesis /*prtHypothesis*/) const
{
  ATH_MSG_DEBUG("Fit of PrepRawDataSet not yet implemented");
  return nullptr;
}

// extend a track fit to include an additional set of MeasurementBase objects
// re-implements the TrkFitterUtils/TrackFitter.cxx general code in a more
// mem efficient and stable way
// --------------------------------
std::unique_ptr<Trk::Track>
GaussianSumFitterTool::fit(const EventContext& ctx,
			   const Trk::Track& inputTrack,
			   const Trk::MeasurementSet& addMeasColl,
			   const Trk::RunOutlierRemoval /*runOutlier*/,
			   const Trk::ParticleHypothesis /*matEffects*/) const
{
  ATH_MSG_VERBOSE ("--> enter GaussianSumFitter::fit(Track,Meas'BaseSet,,)");
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
    ATH_MSG_DEBUG("input fails to provide track parameters for seeding the GSF, reject fit");
    return nullptr;
  }

  std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
  std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());
  
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

  // Set the GaussianSumFitter options
  Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>
    gsfOptions = prepareOptions(tgContext, 
				mfContext, 
				calContext,
				*pSurface);

  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(inputTrack);
  const auto initialParams = m_geometryConvTool->convertTrackParametersToActs(ctx, *inputTrack.perigeeParameters());

  detail::MeasurementCalibratorBase::pack(addMeasColl, trackSourceLinks);

  return performFit(ctx, gsfOptions,
                    trackSourceLinks,
                    initialParams);
}

// extend a track fit to include an additional set of PrepRawData objects
// --------------------------------
std::unique_ptr<Trk::Track>
GaussianSumFitterTool::fit(const EventContext& /*ctx*/,
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
GaussianSumFitterTool::fit(const EventContext& ctx,
			   const Trk::Track& intrk1,
			   const Trk::Track& intrk2,
			   const Trk::RunOutlierRemoval /*runOutlier*/,
			   const Trk::ParticleHypothesis /*matEffects*/) const
{ 
  ATH_MSG_VERBOSE ("--> enter GaussianSumFitter::fit(Track,Track,)");
  ATH_MSG_VERBOSE ("    with Tracks from #1 = " << intrk1.info().dumpInfo()
                   << " and #2 = " << intrk2.info().dumpInfo());

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
    ATH_MSG_DEBUG("input #1 fails to provide track parameters for seeding the GSF, reject fit");
    return nullptr;
  }

   std::unique_ptr<Trk::Track> track = nullptr;

  // Construct a perigee surface as the target surface
   std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3::Zero());
  
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};
    
  // Set the GaussianSumFitter options
  Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>
    gsfOptions = prepareOptions(tgContext, 
				mfContext, 
				calContext,
				*pSurface);

  std::vector<Acts::SourceLink> trackSourceLinks = m_ATLASConverterTool->trkTrackToSourceLinks(intrk1);
  std::vector<Acts::SourceLink> trackSourceLinks2 = m_ATLASConverterTool->trkTrackToSourceLinks(intrk2);
  trackSourceLinks.insert(trackSourceLinks.end(), trackSourceLinks2.begin(), trackSourceLinks2.end());
  const auto initialParams = m_geometryConvTool->convertTrackParametersToActs(ctx, *intrk1.perigeeParameters());

  return performFit(ctx, gsfOptions,
                    trackSourceLinks,
                    initialParams);
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
  std::vector<const Acts::Surface*> surfSequence;

  for (auto ts : track.trackStates()){
    surfSequence.push_back(&ts.referenceSurface());
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

  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  Acts::CalibrationContext calContext{getCalibrationContext(ctx)};

  Acts::GsfOptions<ActsTrk::MutableTrackStateBackend> gsfOptions = prepareOptions(tgContext, mfContext, calContext, pSurface);

  detail::RefittingCalibrator calibrator{m_geometryConvTool.get(), m_ROTcreator.get()};
  using xAODUnCalibrator_t = detail::OnTrackCalibrator<ActsTrk::MutableTrackStateBackend>;
  auto xODCalibrator = xAODUnCalibrator_t::NoCalibration(m_trackingGeometryTool.get());
  calibrator.connect<&xAODUnCalibrator_t::calibrate>(xAOD::UncalibMeasType::PixelClusterType, &xODCalibrator);
  calibrator.connect<&xAODUnCalibrator_t::calibrate>(xAOD::UncalibMeasType::StripClusterType, &xODCalibrator);
  

  detail::RefittingSurfaceAccesor surfaceAcc{m_geometryConvTool.get(),
                                             m_trackingGeometryTool.get()};

  auto gsfExtensions = m_gsfExtensions;
  gsfExtensions.calibrator.connect<&detail::RefittingCalibrator::calibrate>(&calibrator);
  gsfExtensions.surfaceAccessor.connect<&detail::RefittingSurfaceAccesor::operator()>(&surfaceAcc);
  gsfOptions.extensions = gsfExtensions;
  gsfOptions.abortOnError = false;

  if (m_useDirectNavigation) {
    m_directFitter->fit(sourceLinks.begin(), sourceLinks.end(), initialParams, gsfOptions, surfSequence, trackContainer);
  } else {
    m_fitter->fit(sourceLinks.begin(), sourceLinks.end(), initialParams, gsfOptions, trackContainer);
  }

  return StatusCode::SUCCESS;
}

const Acts::GsfExtensions<typename ActsTrk::MutableTrackStateBackend>& 
GaussianSumFitterTool::getExtensions() const 
{ 
  return m_gsfExtensions;
}

/// Private access to the logger
const Acts::Logger& 
GaussianSumFitterTool::logger() const 
{ 
  return *m_logger;
}

Acts::GsfOptions<typename ActsTrk::MutableTrackStateBackend> 
GaussianSumFitterTool::prepareOptions(const Acts::GeometryContext& tgContext,
				      const Acts::MagneticFieldContext& mfContext,
				      const Acts::CalibrationContext& calContext,
				      const Acts::PerigeeSurface& surface) const
{
  Acts::PropagatorPlainOptions propagationOption(tgContext, mfContext);
  propagationOption.maxSteps = m_option_maxPropagationStep;

  Acts::GsfOptions<typename ActsTrk::MutableTrackStateBackend> gsfOptions(tgContext, mfContext, calContext);
  gsfOptions.extensions=m_gsfExtensions;
  gsfOptions.propagatorPlainOptions=propagationOption;
  gsfOptions.referenceSurface = &surface;

  // Set abortOnError to false, else the refitting crashes if no forward propagation is done. Here, we just skip the event and continue.
  gsfOptions.abortOnError = false;
  gsfOptions.maxComponents = m_maxComponents;
  gsfOptions.weightCutoff = m_weightCutOff;
  gsfOptions.componentMergeMethod = m_componentMergeMethod;

  return gsfOptions;
}

}
