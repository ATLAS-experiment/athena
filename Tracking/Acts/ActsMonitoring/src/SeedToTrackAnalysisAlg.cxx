/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/SeedToTrackAnalysisAlg.h"
#include "TruthUtils/MagicNumbers.h"

namespace ActsTrk {

  SeedToTrackAnalysisAlg::SeedToTrackAnalysisAlg(const std::string& name,
						 ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) 
  {}

  StatusCode SeedToTrackAnalysisAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");

    ATH_CHECK( m_seedsKey.initialize() );
    ATH_CHECK( m_paramsKey.initialize() );
    ATH_CHECK( m_destiniesKey.initialize() );
    ATH_CHECK( m_beamSpotKey.initialize() );    

    ATH_CHECK( m_pixelAssociuationMapKey.initialize( not m_pixelAssociuationMapKey.empty() ) );
    ATH_CHECK( m_stripAssociuationMapKey.initialize( not m_stripAssociuationMapKey.empty() ) );

    m_seedVars = Monitored::buildToolMap<int>(m_tools, "seedVars", m_nLayers);
    m_elasticDecayUtil.setEnergyLossBinning( m_energyLossBinning );
    
    return AthMonitorAlgorithm::initialize();
  }

  StatusCode SeedToTrackAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Filling Histograms for " << name() << " ... " );

    SG::ReadHandle< ActsTrk::SeedContainer > seedsHandle = SG::makeHandle( m_seedsKey, ctx );
    ATH_CHECK( seedsHandle.isValid() );
    const ActsTrk::SeedContainer* seeds = seedsHandle.cptr();
    
    SG::ReadHandle< ActsTrk::BoundTrackParametersContainer > paramsHandle = SG::makeHandle( m_paramsKey, ctx );
    ATH_CHECK( paramsHandle.isValid() );
    const ActsTrk::BoundTrackParametersContainer* params = paramsHandle.cptr();
    
    SG::ReadHandle< std::vector<int> > destiniesHandle = SG::makeHandle( m_destiniesKey, ctx );
    ATH_CHECK( destiniesHandle.isValid() );
    const std::vector<int>* destinies = destiniesHandle.cptr();
    
    // Read the Beam Spot information
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle = SG::makeHandle( m_beamSpotKey, ctx );
    ATH_CHECK( beamSpotHandle.isValid() );
    const InDet::BeamSpotData* beamSpotData = beamSpotHandle.cptr();
      
    // Beam Spot Position
    Acts::Vector3 beamPos( beamSpotData->beamPos().x() * Acts::UnitConstants::mm,
                           beamSpotData->beamPos().y() * Acts::UnitConstants::mm,
                           beamSpotData->beamPos().z() * Acts::UnitConstants::mm );

    if (seeds->size() != params->size() or
	seeds->size() != destinies->size()) {
      ATH_MSG_ERROR("Inconsistent size of collections");
      ATH_MSG_ERROR("- " << m_seedsKey);
      ATH_MSG_ERROR("- " << m_paramsKey);
      ATH_MSG_ERROR("- " << m_destiniesKey);
      return StatusCode::FAILURE;
    }


    std::vector< const ActsTrk::MeasurementToTruthParticleAssociation* > truthAssociationMaps (DetectorType::nTypes, nullptr);
    if ( not m_pixelAssociuationMapKey.empty() ) {
      SG::ReadHandle<ActsTrk::MeasurementToTruthParticleAssociation> pixelAssociuationMapHandle =
	SG::makeHandle( m_pixelAssociuationMapKey, ctx );
      ATH_CHECK( pixelAssociuationMapHandle.isValid() );
      truthAssociationMaps[DetectorType::PIXEL] = pixelAssociuationMapHandle.cptr();
    }

    if ( not m_stripAssociuationMapKey.empty() ) {
      SG::ReadHandle<ActsTrk::MeasurementToTruthParticleAssociation> stripAssociuationMapHandle =
	SG::makeHandle( m_stripAssociuationMapKey, ctx );
      ATH_CHECK( stripAssociuationMapHandle.isValid() );
      truthAssociationMaps[DetectorType::STRIP] = stripAssociuationMapHandle.cptr();
    }

    
    std::size_t nElements = seeds->size();

    for (std::size_t i(0); i<nElements; ++i) {
      ActsTrk::Seed seed = seeds->at(i);
      const Acts::BoundTrackParameters* pars = params->at(i);
      const int destiny = destinies->at(i);

      // in case param estimation for this seed failed somehow
      if (not pars) continue;

      const auto& sps = seed.sp();
      const auto& bottom = sps[0];
      const auto& middle = sps[1];
      const auto& top = sps[2];
      
      float bottomX = bottom->x() - beamPos.x();
      float bottomY = bottom->y() - beamPos.y();
      float bottomZ = bottom->z();
      float bottomR = std::sqrt( bottomX * bottomX + bottomY * bottomY );

      float middleX = middle->x() - beamPos.x();
      float middleY = middle->y() - beamPos.y();
      float middleZ = middle->z();
      float middleR = std::sqrt( middleX * middleX + middleY * middleY );

      float topX = top->x() - beamPos.x();
      float topY = top->y() - beamPos.y();
      float topZ = top->z();
      float topR = std::sqrt( topX * topX + topY * topY );      

      float probability = 0.f;
      ATH_CHECK( getTruthProbability(seed,
				     truthAssociationMaps,
				     probability) );

      auto monitor_bottom_x = Monitored::Scalar<float>( "bottomX", bottomX );
      auto monitor_bottom_y = Monitored::Scalar<float>( "bottomY", bottomY );
      auto monitor_bottom_z = Monitored::Scalar<float>( "bottomZ", bottomZ );
      auto monitor_bottom_r = Monitored::Scalar<float>( "bottomR", bottomR );

      auto monitor_middle_x = Monitored::Scalar<float>( "middleX", middleX );
      auto monitor_middle_y = Monitored::Scalar<float>( "middleY", middleY );
      auto monitor_middle_z = Monitored::Scalar<float>( "middleZ", middleZ );
      auto monitor_middle_r = Monitored::Scalar<float>( "middleR", middleR );

      auto monitor_top_x = Monitored::Scalar<float>( "topX", topX );
      auto monitor_top_y = Monitored::Scalar<float>( "topY", topY );
      auto monitor_top_z = Monitored::Scalar<float>( "topZ", topZ );
      auto monitor_top_r = Monitored::Scalar<float>( "topR", topR );

      auto monitor_cotTheta_bm = Monitored::Scalar<float>( "cotTheta_BM", (middleZ - bottomZ) / (middleR - bottomR) );
      auto monitor_cotTheta_mt = Monitored::Scalar<float>( "cotTheta_MT", (topZ - middleZ) / (topR - middleR) );
      auto monitor_cotTheta_bt = Monitored::Scalar<float>( "cotTheta_BT", (topZ - bottomZ) / (topR - bottomR) );

      auto monitor_delta_cotTheta_bm_mt = Monitored::Scalar<float>( "deltaCotTheta_BM_MT", monitor_cotTheta_mt - monitor_cotTheta_bm );

      auto monitor_delta_r_bt = Monitored::Scalar<float>( "deltaR_BT", topR - bottomR );
      auto monitor_delta_r_bm = Monitored::Scalar<float>( "deltaR_BM", middleR - bottomR );
      auto monitor_delta_r_mt = Monitored::Scalar<float>( "deltaR_MT", topR - middleR );

      auto monitor_seed_eta = Monitored::Scalar<float>( "eta", Acts::VectorHelpers::eta(pars->momentum()) );
      auto monitor_seed_pt = Monitored::Scalar<float>( "pt", pars->transverseMomentum() );
      auto monitor_seed_quality = Monitored::Scalar<float>( "quality",  seed.seedQuality() );
      auto monitor_seed_vtx_z = Monitored::Scalar<float>( "vtxZ", seed.z() );

      auto monitor_seed_probability = Monitored::Scalar<float>( "truthProb", probability );
      
      // fill inclusive
      fill(m_tools[m_seedVars[4]],
	   monitor_bottom_x, monitor_bottom_y, monitor_bottom_z, monitor_bottom_r,
	   monitor_middle_x, monitor_middle_y, monitor_middle_z, monitor_middle_r,
	   monitor_top_x, monitor_top_y, monitor_top_z, monitor_top_r,
	   monitor_seed_eta, monitor_seed_pt, monitor_seed_quality, monitor_seed_vtx_z,
	   monitor_delta_r_bt, monitor_delta_r_bm, monitor_delta_r_mt,
	   monitor_cotTheta_bm, monitor_cotTheta_mt, monitor_cotTheta_bt,
	   monitor_delta_cotTheta_bm_mt,
	   monitor_seed_probability);

      fill(m_tools[m_seedVars[destiny]],
	   monitor_bottom_x, monitor_bottom_y, monitor_bottom_z, monitor_bottom_r,
	   monitor_middle_x, monitor_middle_y, monitor_middle_z, monitor_middle_r,
	   monitor_top_x, monitor_top_y, monitor_top_z, monitor_top_r,
	   monitor_seed_eta, monitor_seed_pt, monitor_seed_quality, monitor_seed_vtx_z,
	   monitor_delta_r_bt, monitor_delta_r_bm, monitor_delta_r_mt,
	   monitor_cotTheta_bm, monitor_cotTheta_mt, monitor_cotTheta_bt,
           monitor_delta_cotTheta_bm_mt,
	   monitor_seed_probability);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode SeedToTrackAnalysisAlg::getTruthProbability(const ActsTrk::Seed& seed,
							 const std::vector< const ActsTrk::MeasurementToTruthParticleAssociation* >& associationMaps,
							 float& probability) const
  {
    bool hasTruthInfo = false;
    for ( const ActsTrk::MeasurementToTruthParticleAssociation* map : associationMaps ) {
      if (not map) continue;
      hasTruthInfo = true;
      break;
    }
    if (not hasTruthInfo)
      return StatusCode::SUCCESS;

    std::size_t nMeasurements = 0ul;
    std::unordered_map<std::size_t, int> particleIds {};

    const auto& sps = seed.sp();
    for (const xAOD::SpacePoint* sp : sps) {
      const auto& measurements = sp->measurements();
      for (const xAOD::UncalibratedMeasurement* meas : measurements ) {
	++nMeasurements;

	const ActsTrk::MeasurementToTruthParticleAssociation* truth = nullptr;
	const xAOD::UncalibMeasType measType = meas->type();
	if (measType == xAOD::UncalibMeasType::PixelClusterType) {
	  truth = associationMaps.at(DetectorType::PIXEL);
	} else if (measType == xAOD::UncalibMeasType::StripClusterType) {
	  truth = associationMaps.at(DetectorType::STRIP);
	} else {
	  ATH_MSG_ERROR("Cluster type is not supported");
	  return StatusCode::FAILURE;
	}
	
	if (not truth) {
	  ATH_MSG_ERROR("Cannot use truth information");
	  return StatusCode::FAILURE;
	}
	
	auto tps = truth->at(meas->index());	
	if (tps.empty()) continue;
	
	for (const auto* tp : tps) {
	  std::size_t pid = HepMC::uniqueID(tp);
	  particleIds.try_emplace( pid, 0 );
	  ++particleIds[pid];

	  // get the mother particle
	  const xAOD::TruthParticle* motherParticle = m_elasticDecayUtil.getMother(*tp, m_maxEnergyLoss);
	  if (not motherParticle) continue;
	  
	  std::size_t motherPid = HepMC::uniqueID(motherParticle);
	  if (pid == motherPid) continue;
	  particleIds.try_emplace( motherPid, 0 );
	  ++particleIds[motherPid];
	} // loop on tps
	
      } // loop on measurements
    } // loop on sps

    // probability
    if (nMeasurements!=0){
      for (const auto [pid, nEntries] : particleIds) {
        float prob = static_cast<float>(nEntries) / nMeasurements;
        probability = std::max(probability, prob);
      }
    } else {
      ATH_MSG_WARNING("nMeasurements is zero!");
    }
    
    return StatusCode::SUCCESS;
  }
  
}

