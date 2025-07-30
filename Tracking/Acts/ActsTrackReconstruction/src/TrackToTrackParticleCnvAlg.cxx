/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackToTrackParticleCnvAlg.h"

#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Propagator/detail/JacobianEngine.hpp"
#include "ActsInterop/Logger.h"

#include "AthContainers/Decorator.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "src/detail/CurvilinearCovarianceHelper.h"
#include "src/detail/HitSummaryDataUtils.h"
#include "src/detail/ExpectedHitUtils.h"

#include <Acts/Definitions/TrackParametrization.hpp>
#include <tuple>
#include <sstream>

namespace {

   /** @brief extract the lower triangle for the given covariance matrix and store it in a vector.
    * @param covMatrix the covariance matrix
    * @param n_rows_max only consider this number of rows at most.
    * @return vector of the lower triangle of the covariance matrix i.e. cov(0,0), cov(1,0), cov(1,1), ...
    * Will clear the given vector and fill it with the lower triangle of the covariace up to the maximum row.
    */
   template <typename T, class T_SquareMatrix>
   inline void lowerTriangleToVector(const T_SquareMatrix& covMatrix,
                                     std::vector<T>& vec, unsigned int n_rows_max) {
      assert( covMatrix.rows() == covMatrix.cols());
      vec.clear();
      unsigned int n_rows = std::min(n_rows_max, static_cast<unsigned int>(covMatrix.rows()));
      vec.reserve((n_rows+1)*n_rows/2);
      for (unsigned int i = 0; i < n_rows; ++i) {
         for (unsigned int j = 0; j <= i; ++j) {
            vec.emplace_back(covMatrix(i, j));
         }
      }
   }

   /** @brief extract the lower triangle for the given covariance matrix, store it in a vector and scale the last row by the given factor.
    * @param covMatrix the covariance matrix
    * @param n_rows_max only consider this number of rows at most.
    * @param last_element_scale scale to be applied to the last row
    * @return vector of the lower triangle of the covariance matrix
              i.e. cov(0,0), cov(1,0), cov(1,1), ... cov(last_row-1,0)*scale, ... cov(last_row-1,last_row-1)*scale*scale
    * Will clear the given vector and fill it with the lower triangle of the covariace up to the maximum row, and also
    * scale the elements of the last row by the given factor.
    */
   template <typename T, class T_SquareMatrix>
   inline void lowerTriangleToVectorScaleLastRow(const T_SquareMatrix& covMatrix,
                                                 std::vector<T>& vec, unsigned int n_rows_max,
                                                 typename T_SquareMatrix::Scalar last_element_scale) {
      assert( covMatrix.rows() == covMatrix.cols());
      vec.clear();
      unsigned int n_rows = std::min(n_rows_max, static_cast<unsigned int>(covMatrix.rows()));
      vec.reserve((n_rows+1)*n_rows/2);
      for (unsigned int i = 0; i < n_rows; ++i) {
         for (unsigned int j = 0; j <= i; ++j) {
            vec.emplace_back(covMatrix(i, j));
         }
      }
      typename std::vector<T>::iterator cov_iter = vec.end();
      --cov_iter;
      *cov_iter *= last_element_scale; // apply scale twice to diagonal element
      for (unsigned int i=0; i<n_rows_max; ++i) {
         *cov_iter *= last_element_scale;
         --cov_iter;
      }
   }

   /** @brief Helper to set the given summary value in the summary of the given track particle
    * @param track_particle the track particle for which the summary is updated
    * @param value the new value to set in the track summary
    * @param summary_type the summary type to be updated. 
    */
   void setSummaryValue(xAOD::TrackParticle &track_particle, uint8_t value, xAOD::SummaryType summary_type) {
      uint8_t tmp = value;
      track_particle.setSummaryValue(tmp, summary_type);
   }
}

namespace {
   // Create lut to map neasurement types (pixel and strips only) to hit summary types. 
  std::array<unsigned short,ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)> makeMeasurementToSummaryTypeMap() {
    std::array<unsigned short,ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)> ret;
      for (unsigned short &elm : ret) {
         elm = xAOD::numberOfTrackSummaryTypes;
      }
      ret.at(ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::PixelClusterType)) = xAOD::numberOfPixelHits;
      ret.at(ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::StripClusterType)) = xAOD::numberOfSCTHits;
      return ret;
   }
}

namespace ActsTrk
{
  std::vector<std::pair<Acts::PdgParticle, xAOD::ParticleHypothesis> > TrackToTrackParticleCnvAlg::s_actsHypothesisToxAOD;

  void TrackToTrackParticleCnvAlg::initParticleHypothesisMap() {
      if (s_actsHypothesisToxAOD.empty()) {
         s_actsHypothesisToxAOD.reserve(7);
         s_actsHypothesisToxAOD.push_back( std::make_pair( Acts::eElectron , xAOD::electron) );
         s_actsHypothesisToxAOD.push_back( std::make_pair( Acts::eMuon , xAOD::muon) );
         s_actsHypothesisToxAOD.push_back( std::make_pair( Acts::ePionPlus , xAOD::pion) );
         s_actsHypothesisToxAOD.push_back( std::make_pair( Acts::eProton , xAOD::proton) );
         s_actsHypothesisToxAOD.push_back( std::make_pair( Acts::ePionZero , xAOD::pi0) );
         s_actsHypothesisToxAOD.push_back( std::make_pair( Acts::eNeutron , xAOD::neutron) );
         s_actsHypothesisToxAOD.push_back( std::make_pair( Acts::eGamma , xAOD::photon) );
     }
   }


  TrackToTrackParticleCnvAlg::TrackToTrackParticleCnvAlg(const std::string &name,
                                                         ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode TrackToTrackParticleCnvAlg::initialize()
  {
     std::vector<std::string> supportedStrategies {"BeamLine", "Vertex"};
     bool isAllowedStrategy = false;
     for (const std::string& strategy : supportedStrategies) {
       if (m_perigeeExpression != strategy) continue;
       isAllowedStrategy = true;
       break;
     }
     ATH_MSG_DEBUG("- perigeeExpression: " << m_perigeeExpression.value());
     if (not isAllowedStrategy) {
       ATH_MSG_ERROR("Wrong configuration of the Track to Track Particle Cnv algorithm: perigeeExpression is not supported");
       return StatusCode::FAILURE;
     }

     if (m_perigeeExpression == "BeamLine") m_expression_strategy = expressionStrategy::BeamLine;
     else if (m_perigeeExpression == "Vertex") m_expression_strategy = expressionStrategy::Vertex;
     else if (m_perigeeExpression == "DontRecalculate") m_expression_strategy = expressionStrategy::DontRecalculate;
     else return StatusCode::FAILURE;
    
     ATH_CHECK(m_trackingGeometryTool.retrieve());
     ATH_CHECK( m_tracksContainerKey.initialize() );
     ATH_CHECK( m_trackParticlesOutKey.initialize() );
     ATH_CHECK( m_beamSpotKey.initialize(m_expression_strategy == expressionStrategy::BeamLine) );
     ATH_CHECK( m_vertexHandle.initialize(m_expression_strategy == expressionStrategy::Vertex) );
     ATH_CHECK( m_fieldCacheCondObjInputKey.initialize() );

     m_decorator_actsTracks = m_trackParticlesOutKey.key() + "." + m_decorator_actsTracks.key();
     ATH_CHECK(m_decorator_actsTracks.initialize());
     
     ATH_CHECK( m_extrapolationTool.retrieve() ); // for extrapolation to beamline

     // propagator for conversion to curvilnear parameters
     {
        auto logger = makeActsAthenaLogger(this, "Prop");

        Navigator::Config cfg{m_trackingGeometryTool->trackingGeometry()};
        cfg.resolvePassive = false;
        cfg.resolveMaterial = true;
        cfg.resolveSensitive = true;
        auto navigtor_logger = logger->cloneWithSuffix("Navigator");
        m_propagator = std::make_unique<Propagator>(Stepper(std::make_shared<ATLASMagneticFieldWrapper>()),
                                                    Navigator(cfg,std::move(navigtor_logger)),
                                                    std::move(logger));
     }

     // for layer/module-type information per hit
     ATH_CHECK( m_siDetEleCollKey.initialize() );
     if (m_siDetEleCollToMeasurementType.size() == m_siDetEleCollKey.size()) {
        unsigned int collection_idx=0;
        for (int type : m_siDetEleCollToMeasurementType ) {
           if (type <1 || type >2) {
              ATH_MSG_ERROR("Invalid measurement type (" << type << ") given for collection "  << collection_idx << " : "
                            << m_siDetEleCollKey[collection_idx].key()
                            << ". Expected 1 for pixel, 2 for strips.");
              return StatusCode::FAILURE;
           }
           ++collection_idx;
        }
     }
     else {
        ATH_MSG_ERROR("Expected exactly one value in SiDetEleCollToMeasurementType per SiDetectorElementCollection. But got "
                      << m_siDetEleCollToMeasurementType.size() << " instead of " << m_siDetEleCollKey.size() << ".");
        return StatusCode::FAILURE;
     }

     initParticleHypothesisMap();
     
     return StatusCode::SUCCESS;
  }

  StatusCode TrackToTrackParticleCnvAlg::execute(const EventContext &ctx) const
  {
    SG::WriteHandle<xAOD::TrackParticleContainer> wh_track_particles( m_trackParticlesOutKey, ctx);
    if (wh_track_particles.record(std::make_unique<xAOD::TrackParticleContainer>(),
                                  std::make_unique<xAOD::TrackParticleAuxContainer>()).isFailure()) {
       ATH_MSG_ERROR("Failed to record track particle container with key " << m_trackParticlesOutKey.key() );
       return StatusCode::FAILURE;
    }

    SG::WriteDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::TrackContainer>> trackLink(m_decorator_actsTracks, ctx);

    xAOD::TrackParticleContainer *track_particles = wh_track_particles.ptr();

    const InDet::BeamSpotData *beamspot_data {nullptr};
    const xAOD::VertexContainer *vertexContainer {nullptr};
    const xAOD::Vertex* primaryVertex {nullptr};

    if (m_expression_strategy == expressionStrategy::BeamLine) {
      SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle = SG::makeHandle( m_beamSpotKey, ctx );
      ATH_CHECK(beamSpotHandle.isValid());
      beamspot_data = beamSpotHandle.cptr();
    } 

    if (m_expression_strategy == expressionStrategy::Vertex) {
      SG::ReadHandle<xAOD::VertexContainer> vertexHandle = SG::makeHandle( m_vertexHandle, ctx );
      ATH_CHECK( vertexHandle.isValid() );
      vertexContainer = vertexHandle.cptr();
      if (vertexContainer->size() == 0) {
	ATH_MSG_ERROR("Retrieved an empty vertex container. This is totally wrong!");
	return StatusCode::FAILURE;
      }
      
      for(const xAOD::Vertex* vtx : *vertexContainer) {
         if(vtx->vertexType() == xAOD::VxType::PriVtx) {
	   primaryVertex = vtx;
           break;
         }
      }

      if (not primaryVertex) {
	ATH_MSG_WARNING("Requested to compute track particles wrt primary vertex, but no primary vertex is found. Using dummy vertex");
	primaryVertex = vertexContainer->front();
      }
    }

    std::size_t nTracks = 0ul;
    std::vector<const ActsTrk::TrackContainer *> trackContainers;
    for (const SG::ReadHandleKey<ActsTrk::TrackContainer>& handleKey : m_tracksContainerKey) {
      SG::ReadHandle<ActsTrk::TrackContainer> handle = SG::makeHandle( handleKey, ctx );
      ATH_CHECK(handle.isValid());
      trackContainers.push_back( handle.cptr() );
      nTracks += trackContainers.back()->size();
    }

    // Fast Insertion Trick
    std::vector<xAOD::TrackParticle*> toAddParticles;
    toAddParticles.reserve(nTracks);
    for (std::size_t i(0); i<nTracks; ++i) {
      toAddParticles.push_back( new xAOD::TrackParticle() );
    }
    track_particles->insert(track_particles->end(),
			    toAddParticles.begin(),
			    toAddParticles.end());
    
    SG::ReadCondHandle<AtlasFieldCacheCondObj> fieldHandle = SG::makeHandle( m_fieldCacheCondObjInputKey, ctx );
    ATH_CHECK(fieldHandle.isValid());
    const AtlasFieldCacheCondObj *field_cond_data = fieldHandle.cptr();
    MagField::AtlasFieldCache fieldCache;
    field_cond_data->getInitializedCache(fieldCache);

    const ActsGeometryContext &gctx = m_trackingGeometryTool->getNominalGeometryContext();
    std::shared_ptr<Acts::PerigeeSurface> perigee_surface {nullptr};
    if (m_expression_strategy == expressionStrategy::BeamLine) {
      perigee_surface = makePerigeeSurface(beamspot_data);
    } else if (m_expression_strategy == expressionStrategy::Vertex) {
      perigee_surface = makePerigeeSurface(*primaryVertex);
    }

    std::array<const InDetDD::SiDetectorElementCollection *,ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)> siDetEleColl {};
    for (unsigned int idx=0; idx <m_siDetEleCollToMeasurementType.size(); ++idx ) {
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> detHandle = SG::makeHandle( m_siDetEleCollKey[idx], ctx );
      ATH_CHECK(detHandle.isValid());
      siDetEleColl[m_siDetEleCollToMeasurementType[idx] ] = detHandle.cptr();
    }

    static const std::array<unsigned short, ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)>
      measurementToSummaryType ATLAS_THREAD_SAFE (makeMeasurementToSummaryTypeMap());


    
    // re-used temporaries
    std::vector<float> tmp_cov_vector;
    std::vector<ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType > tmp_param_state_idx;
    tmp_param_state_idx.reserve(30);
    Amg::Vector3D magnFieldVect;
    std::vector<std::vector<float>> parametersVec;
    ActsTrk::detail::HitSummaryData hitInfo;

    unsigned int converted_track_states=0;

    std::size_t particleCounter = 0ul;
    using namespace Acts::UnitLiterals;
    for (const ActsTrk::TrackContainer *tracksContainer : trackContainers) {

      bool precalculatedLayerPattern = detail::ExpectedLayerPatternHelper::exists(*tracksContainer);

      for (const typename ActsTrk::TrackContainer::ConstTrackProxy track : *tracksContainer) {
	xAOD::TrackParticle *track_particle = track_particles->at(particleCounter++);
	
	// convert defining parameters
	// @TODO add support for other modes available in the legacy converter : wrt a vertex, origin, beamspot ?
  Acts::BoundTrackParameters perigeeParam = [&] {
        if (m_expression_strategy == expressionStrategy::DontRecalculate) {
          // If the strategy is "DontRecalculate", we will take the reference surface as is 
          // from the track finding without modification. Consult track finding configuration to
          // find out what that is.
          return track.createParametersAtReference();
        }
        else {
          return parametersAtPerigee(ctx, track, *perigee_surface);
        }
  }();

  Acts::BoundVector boundParams = perigeeParam.parameters();
  track_particle->setDefiningParameters(boundParams[Acts::eBoundLoc0],
                                        boundParams[Acts::eBoundLoc1],
                                        boundParams[Acts::eBoundPhi],
                                        boundParams[Acts::eBoundTheta],
                                        boundParams[Acts::eBoundQOverP] * 1_MeV);

	if (perigeeParam.covariance().has_value()) {
          // only use the 5x5 sub-matrix of the full covariance matrix
          lowerTriangleToVectorScaleLastRow(perigeeParam.covariance().value(),tmp_cov_vector,5, 1_MeV);
          track_particle->setDefiningParametersCovMatrixVec(tmp_cov_vector);
	}
	// optional beam tilt
	if (beamspot_data) {
	  track_particle->setBeamlineTiltX(beamspot_data->beamTilt(0));
	  track_particle->setBeamlineTiltY(beamspot_data->beamTilt(1));
	}
	
	// fit info, quality
	track_particle->setFitQuality(track.chi2(), track.nDoF());
	track_particle->setPatternRecognitionInfo( (1ul << xAOD::SiSPSeededFinder) );
	track_particle->setTrackFitter(xAOD::KalmanFitter);
	
	const Acts::ParticleHypothesis &hypothesis = track.particleHypothesis();
	track_particle->setParticleHypothesis(convertParticleHypothesis( hypothesis.absolutePdg() ));
	constexpr float inv_1_MeV = 1/1_MeV;
	// gather track state indices for parameter conversion
	// @TODO add support for muons
	
	// xAOD::UncalibMeasType::underlying_type is expected to be the number of UncalibMeasTypes
	std::array<std::array<uint8_t, ActsTrk::detail::to_underlying(ActsTrk::detail::HitCategory::N)>,
		   ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)> specialHitCounts{};
	
	ActsTrk::detail::SumOfValues chi2_stat;
	gatherTrackSummaryData(*tracksContainer,
			       track,
			       siDetEleColl,
			       measurementToSummaryType,
			       chi2_stat,
			       hitInfo,
			       tmp_param_state_idx,
			       specialHitCounts);
	
	// Muon
	//    MdtDriftCircleType = 3
	//    RpcStripType = 4,
	//    TgcStripType = 5,
	//    MMClusterType = 6,
	//    sTgcStripType = 7,
	
	// pixel summaries
	std::array< std::tuple< uint8_t, uint8_t, uint8_t, bool >, 4> copy_summary {
          std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelTotal),
                          static_cast<uint8_t>(xAOD::numberOfContribPixelLayers),
                          static_cast<uint8_t>(xAOD::numberOfPixelHits),
                          false),
	  
          std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat),
                          static_cast<uint8_t>(xAOD::numberOfContribPixelBarrelFlatLayers),
                          static_cast<uint8_t>(xAOD::numberOfPixelBarrelFlatHits),
                          true),

          std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelBarrelInclined),
                          static_cast<uint8_t>(xAOD::numberOfContribPixelBarrelInclinedLayers),
                          static_cast<uint8_t>(xAOD::numberOfPixelBarrelInclinedHits),
                          true),

          std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelEndcap),
                          static_cast<uint8_t>(xAOD::numberOfContribPixelEndcap),
                          static_cast<uint8_t>(xAOD::numberOfPixelEndcapHits),\
                          true) };
	
	for (auto [src_region, dest_xaod_summary_layer, dest_xaod_summary_hits, add_outlier] : copy_summary ) {
          setSummaryValue(*track_particle,
                          hitInfo.contributingLayers( static_cast<ActsTrk::detail::HitSummaryData::DetectorRegion>(src_region)),
                          static_cast<xAOD::SummaryType>(dest_xaod_summary_layer));
          setSummaryValue(*track_particle,
			  hitInfo.contributingHits(static_cast<ActsTrk::detail::HitSummaryData::DetectorRegion>(src_region))
                          + ( add_outlier
                              ? hitInfo.contributingOutlierHits(static_cast<ActsTrk::detail::HitSummaryData::DetectorRegion>(src_region))
                              : 0),
                          static_cast<xAOD::SummaryType>(dest_xaod_summary_hits));
	}
	setSummaryValue(*track_particle,
                        hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelEndcap,0)
			+hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap,0),
			xAOD::numberOfInnermostPixelLayerEndcapHits);
	setSummaryValue(*track_particle,
			hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap,0),
			xAOD::numberOfInnermostPixelLayerEndcapOutliers);
	setSummaryValue(*track_particle,
                        hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelEndcap,1)
			+hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelEndcap,2)
			+hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap,1)
			+hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap,2),
			xAOD::numberOfNextToInnermostPixelLayerEndcapHits);
	setSummaryValue(*track_particle,
                        hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap,1)
			+hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap,2),
			xAOD::numberOfNextToInnermostPixelLayerEndcapOutliers);
	setSummaryValue(*track_particle,
			hitInfo.contributingOutlierHits(ActsTrk::detail::HitSummaryData::pixelTotal),
			xAOD::numberOfPixelOutliers);
	setSummaryValue(*track_particle,
			specialHitCounts[ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::PixelClusterType)][ActsTrk::detail::HitCategory::Hole],
			xAOD::numberOfPixelHoles);
	setSummaryValue(*track_particle,
			hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 0),
			xAOD::numberOfInnermostPixelLayerSharedEndcapHits);
	setSummaryValue(*track_particle,
			hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 1)
			+hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 2),
			xAOD::numberOfNextToInnermostPixelLayerSharedEndcapHits);
	setSummaryValue(*track_particle,
			hitInfo.contributingSharedHits(ActsTrk::detail::HitSummaryData::pixelTotal),
			xAOD::numberOfPixelSharedHits);


  std::array<unsigned int,4> expect_layer_pattern{};
  if (precalculatedLayerPattern) {
    // We have a pre-calculated layer pattern from track finding, use as is
    expect_layer_pattern = detail::ExpectedLayerPatternHelper::get(track);
  }
  else {
    // Only check if computeExpectedLayerPattern is true. TODO:: move this computation to the track finding to avoid calling propagator steps here.
    // Do not expect pixel hits if there are not contributing pixel hits in the flat barrel and expectIfPixelContributes is true
    expect_layer_pattern = (m_computeExpectedLayerPattern.value()
                  && (!m_expectIfPixelContributes.value()
                      || hitInfo.contributingLayers(ActsTrk::detail::HitSummaryData::pixelTotal))
                  ? detail::expectedLayerPattern(ctx,
                          *m_extrapolationTool,
                          perigeeParam,
                          m_pixelExpectLayerPathLimitInMM.value() * Acts::UnitConstants::mm)
                  : std::array<unsigned int,4> {0u,0u, 0u,0u} );
  }
	
	// @TODO consider end-caps  for inner most pixel hits ?
	setSummaryValue(*track_particle,
			static_cast<uint8_t>((expect_layer_pattern[0] & (1<<0)) != 0 ),
			xAOD::expectInnermostPixelLayerHit);
	setSummaryValue(*track_particle,
			static_cast<uint8_t>((expect_layer_pattern[0] & (1<<1)) != 0 ),
			xAOD::expectNextToInnermostPixelLayerHit);
	setSummaryValue(*track_particle,
			static_cast<unsigned int >(hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat,0)),
			xAOD::numberOfInnermostPixelLayerHits);
	setSummaryValue(*track_particle,
			static_cast<unsigned int >(hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat,0)),
			xAOD::numberOfInnermostPixelLayerOutliers);
	setSummaryValue(*track_particle,
			static_cast<unsigned int >(hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat,1)),
			xAOD::numberOfNextToInnermostPixelLayerHits);
	setSummaryValue(*track_particle,
			static_cast<unsigned int >(hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat,1)),
			xAOD::numberOfNextToInnermostPixelLayerOutliers);
	setSummaryValue(*track_particle,
			static_cast<unsigned int >(hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat,0)),
			xAOD::numberOfInnermostPixelLayerSharedHits);
	setSummaryValue(*track_particle,
			static_cast<unsigned int >(hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat,1)),
			xAOD::numberOfNextToInnermostPixelLayerSharedHits);
	
	// Strip summaries
	setSummaryValue(*track_particle,
			hitInfo.contributingHits( ActsTrk::detail::HitSummaryData::stripTotal ),
			xAOD::numberOfSCTHits);
	setSummaryValue(*track_particle,
			hitInfo.contributingOutlierHits( ActsTrk::detail::HitSummaryData::stripTotal ),
			xAOD::numberOfSCTOutliers);
	setSummaryValue(*track_particle,
			hitInfo.contributingSharedHits( ActsTrk::detail::HitSummaryData::stripTotal ),
			xAOD::numberOfSCTSharedHits);
	setSummaryValue(*track_particle,
			specialHitCounts[ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::StripClusterType)][ActsTrk::detail::HitCategory::Hole],
			xAOD::numberOfSCTHoles);
	
	double biased_chi2_variance = chi2_stat.biasedVariance();
	setSummaryValue(*track_particle,
			static_cast<uint8_t> (biased_chi2_variance>0.
					      ? std::min(static_cast<unsigned int>(std::sqrt(biased_chi2_variance) * 100),255u)
					      : 0u),
			xAOD::standardDeviationOfChi2OS);
	
	setSummaryValue(*track_particle,
                        hitInfo.contributingOutlierHits( ActsTrk::detail::HitSummaryData::pixelTotal )
			+hitInfo.contributingOutlierHits( ActsTrk::detail::HitSummaryData::stripTotal ),
			xAOD::numberOfOutliersOnTrack);
	
	
	// @TODO slect states for which parameters are stored
	if (m_firstAndLastParamOnly && tmp_param_state_idx.size()>2) {
          tmp_param_state_idx[1]=tmp_param_state_idx.back();
          tmp_param_state_idx.erase(tmp_param_state_idx.begin()+2,tmp_param_state_idx.end());
	}

	// store track parameters and covariances for slected states
	parametersVec.clear();
	parametersVec.reserve(tmp_param_state_idx.size());
	
	for(std::vector<ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType>::const_reverse_iterator
              idx_iter = tmp_param_state_idx.rbegin();
	    idx_iter != tmp_param_state_idx.rend();
	    ++idx_iter) {
          //       for(ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType idx : tmp_param_state_idx) {
          ActsTrk::TrackStateBackend::ConstTrackStateProxy
	    state = tracksContainer->trackStateContainer().getTrackState(*idx_iter);
          const Acts::BoundTrackParameters actsParam = track.createParametersFromState(state);
	  
          Acts::Vector3 position = actsParam.position(gctx.context());
          Acts::Vector3 momentum = actsParam.momentum();
	  
          // scaling from Acts momentume units (GeV) to Athena Units (MeV)
          for (unsigned int i=0; i<momentum.rows(); ++i) {
             momentum(i) *= inv_1_MeV;
          }


          if (actsParam.covariance()) {
             Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
             Acts::GeometryContext tgContext = gctx.context();

             magnFieldVect.setZero();
             fieldCache.getField(position.data(), magnFieldVect.data());
             // scaling from Athena magnetic field units kT to Acts units T
             {
                using namespace Acts::UnitLiterals;
                magnFieldVect *= 1000_T;
             }

             auto curvilinear_cov_result = ActsTrk::detail::convertActsBoundCovToCurvilinearParam(tgContext, actsParam, magnFieldVect, hypothesis);
             if (curvilinear_cov_result.has_value()) {
                Acts::BoundSquareMatrix &curvilinear_cov = curvilinear_cov_result.value();

                // convert q/p components from GeV (Acts) to MeV (Athena)
                for (unsigned int col_i=0; col_i<4; ++col_i) {
                   curvilinear_cov(col_i,4) *= 1_MeV;
                   curvilinear_cov(4,col_i) *= 1_MeV;
                }
                curvilinear_cov(4,4) *= (1_MeV * 1_MeV);

                std::size_t param_idx = parametersVec.size();
                // only use the 5x5 sub-matrix of the full covariance matrix
                lowerTriangleToVector(curvilinear_cov,tmp_cov_vector,5);
                if (tmp_cov_vector.size() != 15) {
                   ATH_MSG_ERROR("Invalid size of lower triangle cov " << tmp_cov_vector.size() <<  " != 15" 
                                 << " input matrix : " << curvilinear_cov.rows() << " x " << curvilinear_cov.cols() );
                }
                track_particle->setTrackParameterCovarianceMatrix(param_idx, tmp_cov_vector);
             }
          }
          parametersVec.emplace_back(std::vector<float>{
	      static_cast<float>(position[0]),static_cast<float>(position[1]),static_cast<float>(position[2]),
	      static_cast<float>(momentum[0]),static_cast<float>(momentum[1]),static_cast<float>(momentum[2]) });
          ++converted_track_states;
	  
	  
	}
	for (const std::vector<float> &param : parametersVec) {
          if (param.size() != 6) {
	    ATH_MSG_ERROR("Invalid size of param element " << param.size() <<  " != 6" );
          }
	}
	
	track_particle->setTrackParameters(parametersVec);
	
	// add element to link to the correspond track
	trackLink(*track_particle)
          = ElementLink<ActsTrk::TrackContainer>( tracksContainer,
                                                  track.index() );
	ATH_CHECK( (trackLink(*track_particle)).isValid() );
      }
    }
    ATH_MSG_DEBUG( "Converted " << nTracks << " acts tracks into " << track_particles->size()
		   << " track particles with parameters for " << converted_track_states << " track states.");
    
    return StatusCode::SUCCESS;
  }

  std::shared_ptr<Acts::PerigeeSurface> TrackToTrackParticleCnvAlg::makePerigeeSurface(const InDet::BeamSpotData *beamspot_data) {
     // @from TrackToVertex::trackAtBeamline
     Acts::Vector3 beamspot(0., 0., 0.);
     float tiltx = 0.0;
     float tilty = 0.0;
     if (beamspot_data) {
        beamspot = Acts::Vector3(beamspot_data->beamVtx().position());
        tiltx =  beamspot_data->beamTilt(0);
        tilty =  beamspot_data->beamTilt(1);
     }
     Acts::Translation3 translation(beamspot);
     Acts::Transform3 transform( translation * Acts::RotationMatrix3::Identity() );
     transform *= Acts::AngleAxis3(tilty, Acts::Vector3(0.,1.,0.));
     transform *= Acts::AngleAxis3(tiltx, Acts::Vector3(1.,0.,0.));
     return Acts::Surface::makeShared<Acts::PerigeeSurface>(transform);
  }

  std::shared_ptr<Acts::PerigeeSurface> TrackToTrackParticleCnvAlg::makePerigeeSurface(const xAOD::Vertex& vertex) {
    Acts::Translation3 translation(Acts::Vector3(vertex.position()));
    Acts::Transform3 transform( translation * Acts::RotationMatrix3::Identity() );
    return Acts::Surface::makeShared<Acts::PerigeeSurface>(transform);    
  }
  
  Acts::BoundTrackParameters TrackToTrackParticleCnvAlg::parametersAtPerigee(const EventContext &ctx,
                                                        const typename ActsTrk::TrackContainer::ConstTrackProxy &track,
                                                        const Acts::PerigeeSurface &perigee_surface) const {
     const Acts::BoundTrackParameters trackParam = track.createParametersAtReference();

     std::optional<const Acts::BoundTrackParameters>
        perigeeParam = m_extrapolationTool->propagate(ctx,
                                                      trackParam,
                                                      perigee_surface,
                                                      Acts::Direction::Backward(), // @TODO try forward if backward fails ?
                                                      m_paramExtrapolationParLimit.value());
     if (!perigeeParam.has_value()) {
        ATH_MSG_WARNING( "Failed to extrapolate to perigee, started from \n" << trackParam << " " << trackParam.referenceSurface().name() );

        return trackParam;
     }
     else {
        ATH_MSG_DEBUG( "Succeeded to extrapolate to perigee ");
        return perigeeParam.value();
     }
  }

}
