/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MeasurementToTrackParticleDecoration.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "Acts/Surfaces/AnnulusBounds.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"

using namespace Acts::UnitLiterals;


namespace ActsTrk {

    MeasurementToTrackParticleDecoration::MeasurementToTrackParticleDecoration(const std::string &name, ISvcLocator *pSvcLocator) :
      AthReentrantAlgorithm(name,pSvcLocator)
    {}

    StatusCode MeasurementToTrackParticleDecoration::initialize()
    {
        ATH_CHECK(m_trackParticlesKey.initialize());
        ATH_CHECK(m_measurementRegionKey.initialize());
	ATH_CHECK(m_measurementDetectorKey.initialize());
        ATH_CHECK(m_measurementLayerKey.initialize());
	ATH_CHECK(m_chi2HitPredictedKey.initialize());
	ATH_CHECK(m_chi2HitFilteredKey.initialize());
        ATH_CHECK(m_measurementTypeKey.initialize());
        ATH_CHECK(m_measurementPhiWidthKey.initialize());
        ATH_CHECK(m_measurementEtaWidthKey.initialize());
        ATH_CHECK(m_residualLocXkey.initialize());
        ATH_CHECK(m_pullLocXkey.initialize());
        ATH_CHECK(m_measurementLocXkey.initialize());
        ATH_CHECK(m_trackParameterLocXkey.initialize());
        ATH_CHECK(m_measurementLocCovXkey.initialize());
        ATH_CHECK(m_trackParameterLocCovXkey.initialize());
        ATH_CHECK(m_residualLocYkey.initialize());
        ATH_CHECK(m_pullLocYkey.initialize());
        ATH_CHECK(m_measurementLocYkey.initialize());
        ATH_CHECK(m_trackParameterLocYkey.initialize());
        ATH_CHECK(m_measurementLocCovYkey.initialize());
        ATH_CHECK(m_trackParameterLocCovYkey.initialize());

	ATH_CHECK(m_trackingGeometryTool.retrieve());
	
        return StatusCode::SUCCESS;
    }

  StatusCode MeasurementToTrackParticleDecoration::execute(const EventContext& ctx) const
    {
        ATH_MSG_DEBUG("Executing MeasurementToTrackParticleDecoration...");

	auto tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();

	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> measurementRegionHandle(m_measurementRegionKey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> measurementDetectorHandle(m_measurementDetectorKey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> measurementLayerHandle(m_measurementLayerKey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> measurementTypeHandle(m_measurementTypeKey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> chi2HitPredictedHandle(m_chi2HitPredictedKey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> chi2HitFilteredHandle(m_chi2HitFilteredKey, ctx);
	
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> measurementPhiWidthHandle(m_measurementPhiWidthKey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> measurementEtaWidthHandle(m_measurementEtaWidthKey, ctx);
	
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> residualLocXhandle(m_residualLocXkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> pullLocXhandle(m_pullLocXkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> measurementLocXhandle(m_measurementLocXkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> trackParameterLocXhandle(m_trackParameterLocXkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> measurementLocCovXhandle(m_measurementLocCovXkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> trackParameterLocCovXhandle(m_trackParameterLocCovXkey, ctx);
	
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> residualLocYhandle(m_residualLocYkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> pullLocYhandle(m_pullLocYkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> measurementLocYhandle(m_measurementLocYkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> trackParameterLocYhandle(m_trackParameterLocYkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> measurementLocCovYhandle(m_measurementLocCovYkey, ctx);
	SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> trackParameterLocCovYhandle(m_trackParameterLocCovYkey, ctx);
	
	SG::ReadHandle<xAOD::TrackParticleContainer> trackParticlesHandle = SG::makeHandle(m_trackParticlesKey, ctx);
        ATH_CHECK(trackParticlesHandle.isValid());
        const xAOD::TrackParticleContainer *track_particles = trackParticlesHandle.cptr();

        static const SG::AuxElement::ConstAccessor<ElementLink<ActsTrk::TrackContainer> > actsTrackLink("actsTrack");

        for (const xAOD::TrackParticle *track_particle : *track_particles) {
            ElementLink<ActsTrk::TrackContainer> link_to_track = actsTrackLink(*track_particle);
            ATH_CHECK(link_to_track.isValid());

            // to ensure that the code does not suggest something stupid (i.e. creating an unnecessary copy)
            static_assert( std::is_same<ElementLink<ActsTrk::TrackContainer>::ElementConstReference,
                           std::optional<ActsTrk::TrackContainer::ConstTrackProxy> >::value);
            std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track = *link_to_track;

            if ( not optional_track.has_value() ) {
	      ATH_MSG_WARNING("Invalid track link for particle  " << track_particle->index() << ". Skipping track..");
	      continue;  
            }
	    
            ATH_MSG_DEBUG("Track link found for track particle with index  " << track_particle->index());
            ActsTrk::TrackContainer::ConstTrackProxy track = optional_track.value();

	    std::vector<int>& regions{measurementRegionHandle(*track_particle)};
            regions.reserve(track.nMeasurements());
            std::vector<int>& detectors{measurementDetectorHandle(*track_particle)};
            detectors.reserve(track.nMeasurements());
            std::vector<int>& layers{measurementLayerHandle(*track_particle)};
            layers.reserve(track.nMeasurements());
            std::vector<int>& types{measurementTypeHandle(*track_particle)};
            types.reserve(track.nMeasurements());
	    std::vector<float>& predchi2s{chi2HitPredictedHandle(*track_particle)};
	    predchi2s.reserve(track.nMeasurements());
	    std::vector<float>& filtchi2s{chi2HitFilteredHandle(*track_particle)};
	    filtchi2s.reserve(track.nMeasurements());
	    
	    std::vector<int>& sizesPhi{measurementPhiWidthHandle(*track_particle)};
            sizesPhi.reserve(track.nMeasurements());
            std::vector<int>& sizesEta{measurementEtaWidthHandle(*track_particle)};
            sizesEta.reserve(track.nMeasurements());
            std::vector<float>& residualsLocX{residualLocXhandle(*track_particle)};
            residualsLocX.reserve(track.nMeasurements());
            std::vector<float>& pullsLocX{pullLocXhandle(*track_particle)};
            pullsLocX.reserve(track.nMeasurements());
            std::vector<float>& measurementsLocX{measurementLocXhandle(*track_particle)};
            measurementsLocX.reserve(track.nMeasurements());
            std::vector<float>& trackParametersLocX{trackParameterLocXhandle(*track_particle)};
            trackParametersLocX.reserve(track.nMeasurements());
            std::vector<float>& measurementsLocCovX{measurementLocCovXhandle(*track_particle)};
            measurementsLocCovX.reserve(track.nMeasurements());
            std::vector<float>& trackParametersLocCovX{trackParameterLocCovXhandle(*track_particle)};
            trackParametersLocCovX.reserve(track.nMeasurements());
            std::vector<float>& residualsLocY{residualLocYhandle(*track_particle)};
            residualsLocY.reserve(track.nMeasurements());
            std::vector<float>& pullsLocY{pullLocYhandle(*track_particle)};
            pullsLocY.reserve(track.nMeasurements());
            std::vector<float>& measurementsLocY{measurementLocYhandle(*track_particle)};
            measurementsLocY.reserve(track.nMeasurements());
            std::vector<float>& trackParametersLocY{trackParameterLocYhandle(*track_particle)};
            trackParametersLocY.reserve(track.nMeasurements());
            std::vector<float>& measurementsLocCovY{measurementLocCovYhandle(*track_particle)};
            measurementsLocCovY.reserve(track.nMeasurements());
            std::vector<float>& trackParametersLocCovY{trackParameterLocCovYhandle(*track_particle)};
            trackParametersLocCovY.reserve(track.nMeasurements());

            for (const auto state : track.trackStatesReversed()) {

                auto flag = state.typeFlags();
		// consider holes and measurements (also outliers)
                bool anyHit = flag.test(Acts::TrackStateFlag::HoleFlag) or flag.test(Acts::TrackStateFlag::MeasurementFlag);
		if (not anyHit) {
                    ATH_MSG_DEBUG("--- This is not a hit measurement, skipping...");
                    continue;
                }

		// starting with invalid values and setting them where needed.

                int detector = -999;
		int region = -999;
		int layer = -999;
		int type = -999;
		float chi2_hit_predicted = -999.;
		float chi2_hit_filtered = -999.;
		int sizePhi = -999;
		int sizeEta = -999;
                float residualLocX = -999.;
		float pullLocX = -999.;
		float measurementLocX = -999.;
		float trackParameterLocX = -999.;
		float measurementLocCovX = -999.;
		float trackParameterLocCovX = -999;
                float residualLocY = -999.;
		float pullLocY = -999.;
		float measurementLocY = -999.;
		float trackParameterLocY = -999.;
		float measurementLocCovY = -999.;
		float trackParameterLocCovY = -999.;
		bool isAnnulusBound = false;
		
		// Get the measurement type
		if (flag.test(Acts::TrackStateFlag::HoleFlag)) {
		  type = MeasurementType::HOLE;
		  ATH_MSG_DEBUG("--- This is a hole");
                } else if (flag.test(Acts::TrackStateFlag::OutlierFlag)) {
		  type = MeasurementType::OUTLIER;
		  ATH_MSG_DEBUG("--- This is an outlier");
                } else {
		  type = MeasurementType::HIT;
		  ATH_MSG_DEBUG("--- This is a hit");
                }
		
		// Check the location of the state
		if (state.hasReferenceSurface() and state.referenceSurface().associatedDetectorElement()) {
		    const ActsDetectorElement * detectorElement = dynamic_cast<const ActsDetectorElement *>(state.referenceSurface().associatedDetectorElement());
		    if (!detectorElement) {
		      ATH_MSG_WARNING("--- TrackState reference surface returned an invalid associated detector element");
		      continue;
		    }
		    const InDetDD::SiDetectorElement * siliconDetectorElement = dynamic_cast<const InDetDD::SiDetectorElement *>(detectorElement->upstreamDetectorElement());
		    if (!siliconDetectorElement) {
		      ATH_MSG_WARNING("--- TrackState associated detector element is not silicon");
		      continue;
		    }
		    
		    const Acts::AnnulusBounds* annulusBounds = dynamic_cast<const Acts::AnnulusBounds*>(&(state.referenceSurface().bounds()));
                    isAnnulusBound = annulusBounds ? true : false;
		    
                    if (siliconDetectorElement) {
                        Identifier detectorIdentifier = siliconDetectorElement->identify();
                        if (siliconDetectorElement->isPixel()) {
			  const PixelID* pixel_id = static_cast<const PixelID *>(siliconDetectorElement->getIdHelper());
                            int layer_disk = pixel_id->layer_disk(detectorIdentifier);
                            layer = layer_disk;
                            if (pixel_id->barrel_ec(detectorIdentifier) == 0) {
                              if (layer_disk == 0) {
                                detector = Subdetector::INNERMOST_PIXEL;
                              } else detector = Subdetector::PIXEL;
                              region = Region::BARREL;
                            } else {
                              detector = Subdetector::PIXEL;
                              region = Region::ENDCAP;
                            }
                        } else if (siliconDetectorElement->isSCT()) {
			  const SCT_ID* sct_id = static_cast<const SCT_ID *>(siliconDetectorElement->getIdHelper());
			  detector = Subdetector::STRIP;
			  region =  sct_id->barrel_ec(detectorIdentifier) == 0 ?
			    Region::BARREL : Region::ENDCAP;
			  layer = sct_id->layer_disk(detectorIdentifier);;
                        } else ATH_MSG_WARNING("--- Unknown detector type - It is not pixel nor strip detecor element!");
                    } else ATH_MSG_WARNING("--- Missing silicon detector element!");
                } else ATH_MSG_WARNING("--- Missing reference surface or associated detector element!");


		
		// If I have a measurement (hit or outlier) then proceed with computing the residuals / pulls
		
		if (type == MeasurementType::OUTLIER || type == MeasurementType::HIT) {

		  //Get the Chi2 computation
		  
		  if (type == MeasurementType::HIT) {
		    chi2_hit_filtered = state.chi2();
		  }
		  
		  if (state.hasUncalibratedSourceLink()) {
		    chi2_hit_predicted = getChi2Contribution(state);
		  }
		  		  
		  // Skip all states without smoothed parameters or without projector
		  if (!state.hasSmoothed() || !state.hasProjector())
		    continue;
		  
		  // Calling effective Calibrated has some runtime overhead
		  const auto &calibratedParameters = state.effectiveCalibrated();
		  const auto &calibratedCovariance = state.effectiveCalibratedCovariance();
		  
		  // We evaluate the unbiased parameters for:
		  // - measurements added to the fit. For outliers, the measurement is not part of the fit, hence track parameters are already unbiased
		  // - if the filtered parameters and the projector exist.
		  bool evaluateUnbiased = (!flag.test(Acts::TrackStateFlag::OutlierFlag));
		  
		  if (evaluateUnbiased) {
                    ATH_MSG_DEBUG("--- Good for unbiased parameters evaluation!");
                    type = MeasurementType::UNBIASED;
                    // if unbiased, access the associated uncalibrated measurement and store the size
                    if (state.hasUncalibratedSourceLink()) {
		      ATLASUncalibSourceLink sourceLink = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
		      const xAOD::UncalibratedMeasurement &uncalibratedMeasurement = getUncalibratedMeasurement(sourceLink);
		      const xAOD::UncalibMeasType measurementType = uncalibratedMeasurement.type();
		      if (measurementType == xAOD::UncalibMeasType::PixelClusterType) {
			auto pixelCluster = static_cast<const xAOD::PixelCluster *>(&uncalibratedMeasurement);
			sizePhi = pixelCluster->channelsInPhi();
			sizeEta = pixelCluster->channelsInEta();
		      } else if (measurementType == xAOD::UncalibMeasType::StripClusterType) {
			auto stripCluster = static_cast<const xAOD::StripCluster *>(&uncalibratedMeasurement);
			sizePhi = stripCluster->channelsInPhi();
		      } else {
			ATH_MSG_DEBUG("xAOD::UncalibratedMeasurement is neither xAOD::PixelCluster nor xAOD::StripCluster");
		      }
                    }
		  }
		  
		  const auto& [unbiasedParameters, unbiasedCovariance] =
		    evaluateUnbiased ? Acts::calculateUnbiasedParametersCovariance(state) : std::make_pair(state.parameters(), state.covariance());
		  
		  measurementLocX = calibratedParameters[Acts::eBoundLoc0];
		  measurementLocCovX = calibratedCovariance(Acts::eBoundLoc0, Acts::eBoundLoc0);
		  
		  if (!isAnnulusBound) {
		    
		    trackParameterLocX = unbiasedParameters[Acts::eBoundLoc0];
		    residualLocX = (measurementLocX - trackParameterLocX) / 1_um; //in um
		    trackParameterLocCovX = unbiasedCovariance(Acts::eBoundLoc0, Acts::eBoundLoc0);
		  }
		  else {
		    // TODO:: use directly phi instead of r*phi in the future
		    
		    float locR    = unbiasedParameters[Acts::eBoundLoc0];
		    float covR    = unbiasedCovariance(Acts::eBoundLoc0,Acts::eBoundLoc0);
		    float locphi  = unbiasedParameters[Acts::eBoundLoc1];
		    float covphi  = unbiasedCovariance(Acts::eBoundLoc1,Acts::eBoundLoc1);
		    float covRphi = unbiasedCovariance(Acts::eBoundLoc0,Acts::eBoundLoc1);
		    
		    trackParameterLocX = locphi;
		    residualLocX = locR * (measurementLocX - trackParameterLocX) / 1_um;
		    // Compute the error on the local rphi                                                                                                                  
		    trackParameterLocCovX = locR*locR*covphi + locphi*locphi*covR + 2*locphi*locR*covRphi;
		    
		    // Rescale the error of the measurement to Rphi.
		    measurementLocCovX = locR*locR * measurementLocCovX;
		  }
		  
		  pullLocX = evaluatePull(residualLocX, measurementLocCovX,
					  trackParameterLocCovX, evaluateUnbiased);
		  
		  if (state.calibratedSize() == 2) {
		    measurementLocY = calibratedParameters[Acts::eBoundLoc1];
		    trackParameterLocY = unbiasedParameters[Acts::eBoundLoc1];
		    residualLocY = (measurementLocY - trackParameterLocY) / 1_um;
		    measurementLocCovY = calibratedCovariance(Acts::eBoundLoc1, Acts::eBoundLoc1);
		    trackParameterLocCovY = unbiasedCovariance(Acts::eBoundLoc1, Acts::eBoundLoc1);
		    pullLocY = evaluatePull(residualLocY, measurementLocCovY,
					    trackParameterLocCovY, evaluateUnbiased);
		  }
		  
		} // hit or outliers

		else if (type == MeasurementType::HOLE) {
		  
		  // Get the predicted position on sensor
		  auto pred = state.predicted();
		  trackParameterLocX = pred[Acts::eBoundLoc0];
		  trackParameterLocY = pred[Acts::eBoundLoc1];
		}
		
		// Always fill with this information
		
		regions.push_back(region);
                detectors.push_back(detector);
                layers.push_back(layer);
                types.push_back(type);
		predchi2s.push_back(chi2_hit_predicted);
		filtchi2s.push_back(chi2_hit_filtered);
                sizesPhi.push_back(sizePhi);
                sizesEta.push_back(sizeEta);
                residualsLocX.push_back(residualLocX);
                pullsLocX.push_back(pullLocX);
                measurementsLocX.push_back(measurementLocX);
                trackParametersLocX.push_back(trackParameterLocX);
                measurementsLocCovX.push_back(measurementLocCovX);
                trackParametersLocCovX.push_back(trackParameterLocCovX);
                residualsLocY.push_back(residualLocY);
                pullsLocY.push_back(pullLocY);
                measurementsLocY.push_back(measurementLocY);
                trackParametersLocY.push_back(trackParameterLocY);
                measurementsLocCovY.push_back(measurementLocCovY);
                trackParametersLocCovY.push_back(trackParameterLocCovY);
	
		
            } // loop on states
	    
	} // loop on tracks

        return StatusCode::SUCCESS;
    }
  
  float MeasurementToTrackParticleDecoration::getChi2Contribution(const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) const {
    
    auto pred  = state.predicted();
    auto predC = state.predictedCovariance();

    return Acts::visit_measurement(
      state.calibratedSize(),
      [&]<std::size_t measdim>(std::integral_constant<std::size_t, measdim>) {
        Acts::FixedBoundSubspaceHelper<measdim> subspaceHelper =
            state.template projectorSubspaceHelper<measdim>();

        // TODO use subspace helper for projection instead
        auto H = subspaceHelper.projector();

        const auto calibrated    = state.template calibrated<measdim>();
        const auto calibratedCov = state.template calibratedCovariance<measdim>();
        
        auto residual = (H * pred - calibrated).eval();
        auto rescov   = (H * predC * H.transpose() + calibratedCov).eval();

        return ((residual.transpose() * rescov.inverse() * residual).eval())(0,0);
    });


        
  }

  
  float MeasurementToTrackParticleDecoration::evaluatePull(const float residual,
							   const float measurementCovariance,
							   const float trackParameterCovariance,
							   const bool evaluateUnbiased) const {
    float correlation = evaluateUnbiased ? 1. : -1.;
    float residualCovariance = measurementCovariance + correlation*trackParameterCovariance;
    if (residualCovariance<=0.) {
      // If the total covariance is non-positive return 0
      ATH_MSG_DEBUG("--- Total covariance for pull evaluation is non-positive! Returning pulls = 0!");
      return 0.;
    }
    return 0.001 * residual/std::sqrt(residualCovariance);
  }
  
} // namespace

