/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : NSWRawDataMonAlg
// Authors:   M. Biglietti (Roma Tre)
//
// DESCRIPTION:
// Subject: NSW-->Offline Muon Data Quality
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "NSWDataMonAlg.h"
#include "MuonRIO_OnTrack/MMClusterOnTrack.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"

/////////////////////////////////////////////////////////////////////////////
// *********************************************************************
// Public Methods
// ********************************************************************* 

NSWDataMonAlg::NSWDataMonAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
	AthMonitorAlgorithm(name,pSvcLocator)
{ }

/*---------------------------------------------------------*/
StatusCode NSWDataMonAlg::initialize()
/*---------------------------------------------------------*/
{
	//init message stream
	ATH_MSG_DEBUG("initialize NSWDataMonAlg");

	ATH_CHECK(AthMonitorAlgorithm::initialize());
	ATH_CHECK(m_muonKey.initialize());
	ATH_CHECK(m_idHelperSvc.retrieve());



	ATH_MSG_DEBUG(" end of initialize " );
	ATH_MSG_INFO("NSWDataMonAlg initialization DONE " );

	return StatusCode::SUCCESS;
} 

StatusCode NSWDataMonAlg::fillHistograms(const EventContext& ctx) const
{

	ATH_MSG_DEBUG("NSWDataMonAlg::MM RawData Monitoring Histograms being filled" );
	SG::ReadHandle<xAOD::MuonContainer> muonContainer(m_muonKey, ctx);
	if (!muonContainer.isValid()) {
	  ATH_MSG_FATAL("Could not get muon container: " << m_muonKey.fullKey());
	  return StatusCode::FAILURE;
	}
	
	std::string NSWMonGroup = "NSWMonGroup";

	int lumiblock = GetEventInfo(ctx)->lumiBlock();

	auto lb = Monitored::Scalar<float>("lb", lumiblock);

	for (const xAOD::Muon* mu : *muonContainer) {
	  if(mu -> pt() < m_cutPt) continue;


	  if(!(mu -> author() == xAOD::Muon::Author::MuidCo || (mu -> author() == xAOD::Muon::Author::MuidSA && std::abs(mu->eta()) > 2.5) ) ) continue;
	  const xAOD::TrackParticle* saTP = mu->primaryTrackParticle();
	  if(saTP == nullptr) continue;
	  const Trk::Track* saTrack = saTP->track();
	  if(!saTrack) continue;

	  float eta_mu=saTP->eta();

	  int	  nMMG=0; int nSTG=0;
	  int ref_sector=-99;
	  float z_nsw=-99;
	  float theta_mu=saTP->theta();
	  float phi_mu=saTP->phi();


	  
	  std::string side = "sideA";
	  int iside = 1;
	  if(eta_mu<0) {
	    side = "sideC";
	    iside=-1;
	  }

	  auto mu_phi = Monitored::Scalar<float>("phi_mu_"+side, phi_mu);



	  std::vector<int> layers_mm;	    std::vector<int> layers_stg;
	  int layer_max=-99;
	  float min_z_nsw=9999;
	  
	  for(const Trk::TrackStateOnSurface* trkState : *saTrack->trackStateOnSurfaces()) {
	    if(!(trkState)) continue;
	    
	    if (!trkState->type(Trk::TrackStateOnSurface::Measurement)) continue;
	    const Trk::MeasurementBase* meas = trkState->measurementOnTrack() ;
	    if(!meas) continue;
	    
	    const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
	    if(!rot) continue;

	            
	    Identifier rot_id = rot->identify();

	    int channelType = m_idHelperSvc->stgcIdHelper().channelType(rot_id);
	    if(!(m_idHelperSvc->isMM(rot_id) or channelType == sTgcIdHelper::sTgcChannelTypes::Strip)) continue;

	  

	    const Muon::MuonClusterOnTrack* cluster = dynamic_cast<const Muon::MuonClusterOnTrack*>(rot);

	    if (!cluster)             continue;

	    const Muon::MuonCluster* mmPrepData = cluster->prepRawData();
	    Amg::Vector3D pos = mmPrepData->globalPosition();
	    if(fabs(pos.z())<min_z_nsw){
	      z_nsw=pos.z();
	      min_z_nsw=fabs(z_nsw);
	    }
	    int multi_mm=0;
	    int gap_mm=0;
	    int multi_stg=0;
	    int gap_stg=0;

	    if(m_idHelperSvc->isMM(rot_id)) {
	      nMMG++;
	      multi_mm          = m_idHelperSvc->mmIdHelper().multilayer(rot_id);
	      gap_mm            = m_idHelperSvc->mmIdHelper().gasGap(rot_id);
	      auto layer_mm=gap_mm+(multi_mm-1)*4;
	      
	      if (std::find(layers_mm.begin(), layers_mm.end(), layer_mm) == layers_mm.end())   layers_mm.push_back(layer_mm);
	      if(layer_mm>layer_max){
		layer_max=layer_mm;
		ref_sector= m_idHelperSvc->sector(rot_id)   ;
	      }
	    }
	    if(m_idHelperSvc->issTgc(rot_id)) {
	      if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip)  {
		nSTG++;
		multi_stg  = m_idHelperSvc -> stgcIdHelper().multilayer(rot_id);
		gap_stg    = m_idHelperSvc -> stgcIdHelper().gasGap(rot_id);
		auto layer_stg=gap_stg+(multi_stg-1)*4;

		if (std::find(layers_stg.begin(), layers_stg.end(), layer_stg) == layers_stg.end())   layers_stg.push_back(layer_stg);
		if(layer_stg>layer_max) {
		  layer_max=layer_stg;
		  ref_sector= m_idHelperSvc->sector(rot_id)   ; 
		}
	      }
	    }

	  }//loop trackStates

	  // --- fallback: assign sector from phi if no NSW hits and eta > 1.2
	  if (ref_sector < 0 && std::fabs(eta_mu) > 1.25) {
	    Muon::MuonSectorMapping smapping;
	    ref_sector = smapping.getSector(mu_phi);
	  }
	  
	  float x_mu=z_nsw*tan(theta_mu)*cos(phi_mu);
	  float y_mu=z_nsw*tan(theta_mu)*sin(phi_mu);
	  auto x_trk = Monitored::Scalar<float>("x_trk_"+side, x_mu);
	  auto y_trk = Monitored::Scalar<float>("y_trk_"+side, y_mu);

	  int isEff= (layers_mm.size()>3 || layers_stg.size()>3);
	  auto hitcut = Monitored::Scalar<int>("hitcut", (int)isEff);
	  auto lb_side = Monitored::Scalar<float>("lb_"+side, lumiblock);
	  auto sector_phi = Monitored::Scalar<int>("sector_phi", ref_sector*iside);
	  if (ref_sector > 0)  fill(NSWMonGroup, hitcut, mu_phi, lb, lb_side, sector_phi, x_trk, y_trk);
	}
	
	
	return StatusCode::SUCCESS;
}
