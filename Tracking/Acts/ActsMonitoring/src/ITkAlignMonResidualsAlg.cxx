/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "ITkAlignMonResidualsAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>


namespace ActsTrk {

  ITkAlignMonResidualsAlg::ITkAlignMonResidualsAlg(const std::string& name,
						   ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator)
  {}
  
  StatusCode ITkAlignMonResidualsAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");
    
    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);

    ATH_CHECK(m_trackParticlesKey.initialize());

    // Decorators
    m_measurement_det = m_trackParticlesKey.key() + "." + m_measurement_det.key();
    m_measurement_region = m_trackParticlesKey.key() + "." + m_measurement_region.key();    
    m_measurement_type = m_trackParticlesKey.key() + "." + m_measurement_type.key();
    m_measurement_layer = m_trackParticlesKey.key() + "." + m_measurement_layer.key();
    m_hitResiduals_residualLocX = m_trackParticlesKey.key() + "." + m_hitResiduals_residualLocX.key();
    m_hitResiduals_pullLocX = m_trackParticlesKey.key() + "." + m_hitResiduals_pullLocX.key();
    m_hitResiduals_residualLocY = m_trackParticlesKey.key() + "." + m_hitResiduals_residualLocY.key();
    m_hitResiduals_pullLocY = m_trackParticlesKey.key() + "." + m_hitResiduals_pullLocY.key();
    m_hitResiduals_phiWidth = m_trackParticlesKey.key() + "." + m_hitResiduals_phiWidth.key();
    m_hitResiduals_etaWidth = m_trackParticlesKey.key() + "." + m_hitResiduals_etaWidth.key();    

    ATH_CHECK(m_measurement_det.initialize());
    ATH_CHECK(m_measurement_region.initialize());
    ATH_CHECK(m_measurement_type.initialize());
    ATH_CHECK(m_measurement_layer.initialize());
    ATH_CHECK(m_hitResiduals_residualLocX.initialize());
    ATH_CHECK(m_hitResiduals_pullLocX.initialize());
    ATH_CHECK(m_hitResiduals_residualLocY.initialize());
    ATH_CHECK(m_hitResiduals_pullLocY.initialize());
    ATH_CHECK(m_hitResiduals_phiWidth.initialize());
    ATH_CHECK(m_hitResiduals_etaWidth.initialize());
    
    // Monitoring tools
    m_pixResidualX = Monitored::buildToolMap<int>(m_tools, "PixResidualX", m_nSiBlayers);
    m_pixResidualY = Monitored::buildToolMap<int>(m_tools, "PixResidualY", m_nSiBlayers);
    m_pixPullX     = Monitored::buildToolMap<int>(m_tools, "PixPullX",     m_nSiBlayers);
    m_pixPullY     = Monitored::buildToolMap<int>(m_tools, "PixPullY",     m_nSiBlayers);

    m_stripResidualX = Monitored::buildToolMap<int>(m_tools, "StripResidualX", m_nSiBlayers);
    m_stripPullX     = Monitored::buildToolMap<int>(m_tools, "StripPullX",     m_nSiBlayers);
    
    return AthMonitorAlgorithm::initialize();
  }
  
  StatusCode ITkAlignMonResidualsAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Filling histograms for "<<name()<< "...");

    // Retrieve the track particles
    
    SG::ReadHandle<xAOD::TrackParticleContainer> trackParticlesHandle = SG::makeHandle(m_trackParticlesKey, ctx);
    ATH_CHECK(trackParticlesHandle.isValid());
    const xAOD::TrackParticleContainer *trackparticles = trackParticlesHandle.cptr();

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decorator_measurement_det( m_measurement_det, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decorator_measurement_region( m_measurement_region, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decorator_measurement_type( m_measurement_type, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decorator_measurement_iLayer( m_measurement_layer, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decorator_hitResiduals_residualLocX( m_hitResiduals_residualLocX, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decorator_hitResiduals_pullLocX( m_hitResiduals_pullLocX, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decorator_hitResiduals_residualLocY( m_hitResiduals_residualLocY, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decorator_hitResiduals_pullLocY( m_hitResiduals_pullLocY, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decorator_hitResiduals_phiWidth( m_hitResiduals_phiWidth, ctx );
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decorator_hitResiduals_etaWidth( m_hitResiduals_etaWidth, ctx );

    ATH_CHECK(decorator_measurement_det.initialize());
    ATH_CHECK(decorator_measurement_region.isValid());
    ATH_CHECK(decorator_measurement_type.isValid());
    ATH_CHECK(decorator_measurement_iLayer.isValid());
    ATH_CHECK(decorator_hitResiduals_residualLocX.isValid());
    ATH_CHECK(decorator_hitResiduals_pullLocX.isValid());
    ATH_CHECK(decorator_hitResiduals_residualLocY.isValid());
    ATH_CHECK(decorator_hitResiduals_pullLocY.isValid());
    ATH_CHECK(decorator_hitResiduals_phiWidth.isValid());
    ATH_CHECK(decorator_hitResiduals_etaWidth.isValid());
    
    for (const xAOD::TrackParticle* track : *trackparticles) {
      const std::vector<int>& result_det = decorator_measurement_det(*track);
      if (result_det.empty()) continue;
      
      const std::vector<int>&   result_region = decorator_measurement_region(*track);
      const std::vector<int>&   result_measureType = decorator_measurement_type(*track);
      const std::vector<int>&   result_layer  = decorator_measurement_iLayer(*track);
      const std::vector<float>& result_residualLocX = decorator_hitResiduals_residualLocX(*track);
      const std::vector<float>& result_pullLocX = decorator_hitResiduals_pullLocX(*track);
      const std::vector<float>& result_residualLocY = decorator_hitResiduals_residualLocY(*track);
      const std::vector<float>& result_pullLocY = decorator_hitResiduals_pullLocY(*track);
      // const std::vector<int>&   result_phiWidth = decorator_hitResiduals_phiWidth(*track);
      // const std::vector<int>&   result_etaWidth = decorator_hitResiduals_etaWidth(*track);
      // const float eta = track->eta();
      
      // NP: this should be fine... resiudal filled with -1 if not hit
      if (result_det.size() != result_residualLocX.size()) {
	ATH_MSG_WARNING("Vectors of results are not matched in size!");
      }
      
      const auto resultSize = result_region.size();
      for (unsigned int idx = 0; idx < resultSize; ++idx) {
	
	const int measureType = result_measureType.at(idx);
	const int det = result_det.at(idx);
	const int layer = result_layer.at(idx);
	const int region = result_region.at(idx);
	//const int width = result_phiWidth.at(idx);
	//const int etaWidth = result_etaWidth.at(idx);
	const float residualX = result_residualLocX.at(idx);
	const float pullLocX = result_pullLocX.at(idx);
	const float residualY = result_residualLocY.at(idx);
	const float pullLocY = result_pullLocY.at(idx);
	
	if (det == -1) continue;
	if (region == -1) continue;
	
	// Unbiased residuals only for the moment
	if (measureType == 4) {
	  
	  //1 PIXEL, 2 STRIP
	  if (det == 1 || det == 0 )  {// PIXEL or PIXEL LY 0
	    if (region == 0) { // BARREL
	      //std::cout<<"Filling pixel layer "<<layer<<std::endl;
	      auto pix_b_residualsx_m = Monitored::Scalar<float>("m_pix_residualsx", residualX);
	      auto pix_b_residualsy_m = Monitored::Scalar<float>("m_pix_residualsy", residualY);
	      auto pix_b_pullsx_m     = Monitored::Scalar<float>("m_pix_pullsx",     pullLocX);
	      auto pix_b_pullsy_m     = Monitored::Scalar<float>("m_pix_pullsy",     pullLocY);
	      
	      fill(m_tools[m_pixResidualX[layer]], pix_b_residualsx_m);
	      fill(m_tools[m_pixResidualY[layer]], pix_b_residualsy_m);
	      fill(m_tools[m_pixPullX[layer]], pix_b_pullsx_m);
	      fill(m_tools[m_pixPullY[layer]], pix_b_pullsy_m);
	      
	    } //Barrel
	  } // Pixel
	  
	  else if (det == 2) { // Strips
	    if (region == 0) { // BARREL		
	      auto strip_b_residualsx_m = Monitored::Scalar<float>("m_strip_residualsx", residualX);
	      auto strip_b_pullsx_m     = Monitored::Scalar<float>("m_strip_pullsx",     pullLocX);
	      fill(m_tools[m_stripResidualX[layer]], strip_b_residualsx_m);
	      fill(m_tools[m_stripPullX[layer]], strip_b_pullsx_m);
	    }
	  }
	} // measure type
      } // resultSize
    } // loop on track
    
    return StatusCode::SUCCESS;
    
  } // fill histograms

} //name space ActsTrk
