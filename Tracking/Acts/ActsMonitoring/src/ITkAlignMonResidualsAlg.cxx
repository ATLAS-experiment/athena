/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "ITkAlignMonResidualsAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>


namespace ActsTrk {

  ITkAlignMonResidualsAlg::ITkAlignMonResidualsAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator)
  {}
  
  StatusCode ITkAlignMonResidualsAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");
    ATH_MSG_DEBUG("Properties:");
    ATH_MSG_DEBUG(m_trackParticlesKey);
    ATH_CHECK(m_trackParticlesKey.initialize());
    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);
    
    ATH_CHECK(m_acc_measurement_regionAcc.initialize());
    
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
        
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> acc_measurement_regionAcc{m_acc_measurement_regionAcc, ctx};

    for (auto trkprt : *trackparticles) {
      auto trk = *trkprt;
      static const SG::ConstAccessor<std::vector<int> >
	measurement_regionAcc("measurement_region");
      const static bool hitDetailsAvailable = measurement_regionAcc.isAvailable(trk);
      
      if (!hitDetailsAvailable) {
	ATH_MSG_WARNING("The hit res plots dont see any data");
	
	auto measurement_region = acc_measurement_regionAcc(trk);

      }
      else {
	
	static const SG::ConstAccessor< std::vector<int> > measurement_detAcc("measurement_det");
	const std::vector<int>& result_det = measurement_detAcc(trk);

	// Read all the other decorators
	if (!result_det.empty()) {
	  
	  static const SG::ConstAccessor< std::vector<int> > measurement_typeAcc("measurement_type");
	  static const SG::ConstAccessor< std::vector<int> > measurement_regionAcc("measurement_region");
	  static const SG::ConstAccessor< std::vector<int> > measurement_layerAcc("measurement_iLayer");
	  static const SG::ConstAccessor< std::vector<float> > hitResiduals_residualLocXAcc("hitResiduals_residualLocX");
	  static const SG::ConstAccessor< std::vector<float> > hitResiduals_pullLocXAcc("hitResiduals_pullLocX");
	  static const SG::ConstAccessor< std::vector<float> > hitResiduals_residualLocYAcc("hitResiduals_residualLocY");
	  static const SG::ConstAccessor< std::vector<float> > hitResiduals_pullLocYAcc("hitResiduals_pullLocY");
	  static const SG::ConstAccessor< std::vector<int> > hitResiduals_phiWidthAcc("hitResiduals_phiWidth");
	  static const SG::ConstAccessor< std::vector<int> > hitResiduals_etaWidthAcc("hitResiduals_etaWidth");
	  
	  const std::vector<int>&   result_measureType = measurement_typeAcc(trk);
	  const std::vector<int>&   result_region = measurement_regionAcc(trk);
	  const std::vector<int>&   result_layer  = measurement_layerAcc(trk);
	  const std::vector<float>& result_residualLocX = hitResiduals_residualLocXAcc(trk);
	  const std::vector<float>& result_pullLocX = hitResiduals_pullLocXAcc(trk);
	  const std::vector<float>& result_residualLocY = hitResiduals_residualLocYAcc(trk);
	  const std::vector<float>& result_pullLocY = hitResiduals_pullLocYAcc(trk);
	  //const std::vector<int>&   result_phiWidth = hitResiduals_phiWidthAcc(trk);
	  //const std::vector<int>&   result_etaWidth = hitResiduals_etaWidthAcc(trk);

	  //const float eta = trk.eta();
	  
	  // NP: this should be fine... resiudal filled with -1 if not hit
	  if (result_det.size() != result_residualLocX.size()) {
	    ATH_MSG_WARNING("Vectors of results are not matched in size!");
	  }
	  const auto resultSize = result_region.size();
	  for (unsigned int idx = 0; idx < resultSize; ++idx) {
	    
	    const int measureType = result_measureType[idx];
	    const int det = result_det[idx];
	    const int layer = result_layer[idx];
	    const int region = result_region[idx];
	    //const int width = result_phiWidth[idx];
	    //const int etaWidth = result_etaWidth[idx];
	    const float residualX = result_residualLocX[idx];
	    const float pullLocX = result_pullLocX[idx];
	    const float residualY = result_residualLocY[idx];
	    const float pullLocY = result_pullLocY[idx];
	    if ((det == -1) or (region == -1)) {
	      continue;
	    }
	    
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
	    } //unbiased residuals
	  }//resultSize
	}//result det not empty
      } //hit Detail available
    }//trk loop

    return StatusCode::SUCCESS;
    
  }//fill histograms
}//name space ActsTrk
