
/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "AthViews/ViewHelper.h"
#include "ViewCreatorCentredOnIParticleROITool.h"
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTrigMissingET/TrigMissingETContainer.h"

using namespace TrigCompositeUtils;

ViewCreatorCentredOnIParticleROITool::ViewCreatorCentredOnIParticleROITool(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent) 
  {}


StatusCode ViewCreatorCentredOnIParticleROITool::initialize()  {
  ATH_CHECK(m_roisWriteHandleKey.initialize());
  return StatusCode::SUCCESS;
}


StatusCode ViewCreatorCentredOnIParticleROITool::attachROILinks(TrigCompositeUtils::DecisionContainer& decisions, const EventContext& ctx) const {
  SG::WriteHandle<TrigRoiDescriptorCollection> roisWriteHandle = createAndStoreNoAux(m_roisWriteHandleKey, ctx);
  
  for ( Decision* outputDecision : decisions ) { 
    
    std::set<const Decision*> cache;
    std::vector<SG::sgkey_t> keys;
    std::vector<uint32_t> clids;
    std::vector<Decision::index_type> indices;
    std::vector<const Decision*> sources;
    typelessFindLinks(outputDecision, m_iParticleLinkName, keys, clids, indices, sources, TrigDefs::lastFeatureOfType, &cache);

    //expect only one link per decision
    if(keys.size()!=1){
      ATH_MSG_ERROR("Did not find exactly one object having searched for a link named '"<<m_iParticleLinkName<<"'. Found "<<keys.size()<<" instead");
      return StatusCode::FAILURE;
    }

    //first check if we've found a MET feature, and attach a FS RoI if so
    if( clids.at(0) == ClassID_traits<xAOD::TrigMissingETContainer>::ID()){
      ATH_MSG_DEBUG("Encountered a MET feature in the CentredOnIParticle ROITool. Cannot centre an ROI on this. Attaching a FullScan RoI");
      roisWriteHandle->push_back( new TrigRoiDescriptor( RoiDescriptor::FULLSCAN ) );
      const ElementLink<TrigRoiDescriptorCollection> roiEL = ElementLink<TrigRoiDescriptorCollection>(*roisWriteHandle, roisWriteHandle->size() - 1, ctx);
      outputDecision->setObjectLink(roiString(), roiEL);
      continue;
    }

    //If not MET, we should have an IParticle
    const ElementLink<xAOD::IParticleContainer> p4EL(keys.at(0), indices.at(0), ctx);
    ATH_CHECK(p4EL.isValid());

    const double reta  = (*p4EL)->eta();
    const double retap = reta + m_roiEtaWidth;
    const double retam = reta - m_roiEtaWidth;
    const double rphi  = (*p4EL)->phi();
    const double rphip = rphi + m_roiPhiWidth;
    const double rphim = rphi - m_roiPhiWidth;

    TrigRoiDescriptor *newROI = nullptr;

    if ( m_roiZedWidth >= 0  ) {

         const xAOD::Muon* muon = dynamic_cast< const xAOD::Muon*>(*p4EL); //get muon of this found object
         double zed0  = 0.0; //initialization
      
         bool update_z_width = true;
      
         if ( m_useZedPosition ) { 
	    if ( muon && muon->primaryTrackParticle() ) { 
	       zed0 = muon->primaryTrackParticle()->z0();
	       if ( m_useBeamspot ) zed0 += muon->primaryTrackParticle()->vz();
	    }
	    else update_z_width = false;
         }
    
	 if ( update_z_width ) { 

	     double zed0p   = zed0 + m_roiZedWidth; // in mm
	     double zed0m   = zed0 - m_roiZedWidth; // in mm
	     
	     if ( m_roiZedSinThetaFlag ) { 
	       /// 1/sin(theta) = cosh( eta ) 
	       double cosheta = std::cosh( (*p4EL)->eta() );
	       zed0p   = zed0 + m_roiZedWidth*cosheta; // in mm
	       zed0m   = zed0 - m_roiZedWidth*cosheta; // in mm
	     }
	     
	     ATH_MSG_DEBUG( "New ROI for xAOD::Particle ET="<< (*p4EL)->p4().Et()
			    << " eta="<< (*p4EL)->eta() << " +- " << m_roiEtaWidth
			    << " phi="<< (*p4EL)->phi() << " +- " << m_roiPhiWidth
			    << " zed0="<<  zed0 << " +- " << m_roiZedWidth );
	   
	     newROI = new TrigRoiDescriptor( reta, retam, retap,
					     rphi, rphim, rphip,
					     zed0, zed0m, zed0p );
	 }
	 else {
	   newROI = new TrigRoiDescriptor( reta, retam, retap,
					   rphi, rphim, rphip);
	 }   
    }
    else {
      newROI = new TrigRoiDescriptor( reta, retam, retap,
				      rphi, rphim, rphip);
    }
    
    roisWriteHandle->push_back( newROI );
    
    const ElementLink<TrigRoiDescriptorCollection> roiEL = ElementLink<TrigRoiDescriptorCollection>(*roisWriteHandle, roisWriteHandle->size() - 1, ctx);
    
    outputDecision->setObjectLink(roiString(), roiEL);
  }
  
  return StatusCode::SUCCESS;
}
