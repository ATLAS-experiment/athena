/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
   */

#include "GfexInputMonitorAlgorithm.h"
#include "TProfile2D.h"
#include "TMath.h"
GfexInputMonitorAlgorithm::GfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
	: AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode GfexInputMonitorAlgorithm::initialize() {

	ATH_MSG_DEBUG("GfexInputMonitorAlgorith::initialize");
	ATH_MSG_DEBUG("Package Name "<< m_packageName);
	ATH_MSG_DEBUG("m_gFexTowerContainer"<< m_gFexTowerContainerKey);

	// we initialise all the containers that we need
	ATH_CHECK( m_gFexTowerContainerKey.initialize() );

	return AthMonitorAlgorithm::initialize();
}

StatusCode GfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

	ATH_MSG_DEBUG("GfexInputMonitorAlgorithm::fillHistograms");

	// Access gFex gTower container
	SG::ReadHandle<xAOD::gFexTowerContainer> gFexTowerContainer{m_gFexTowerContainerKey, ctx};
	if(!gFexTowerContainer.isValid()){
		ATH_MSG_ERROR("No gFex Tower container found in storegate  "<< m_gFexTowerContainerKey);
		return StatusCode::SUCCESS;
	}

	// monitored variables for histograms
	auto nGfexTowers = Monitored::Scalar<int>("NGfexTowers",0.0);
	auto Towereta = Monitored::Scalar<float>("TowerEta",0.0);
	auto Towerphi = Monitored::Scalar<float>("TowerPhi",0.0);
	auto Towersaturationflag = Monitored::Scalar<char>("TowerSaturationflag",0.0);
	auto Toweret = Monitored::Scalar<int>("TowerEt",0);
	auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());
    auto lbnString = Monitored::Scalar<std::string>("LBNString",std::to_string(GetEventInfo(ctx)->lumiBlock()));
    auto lbn = Monitored::Scalar<int>("LBN",GetEventInfo(ctx)->lumiBlock());
	auto binNumber = Monitored::Scalar<int>("binNumber",0);

	unsigned int nTowers = 0;
	auto maxet = 0.0;

	for(const xAOD::gFexTower* gfexTowerRoI : *gFexTowerContainer){


		Toweret=gfexTowerRoI->towerEt();
		Towersaturationflag=gfexTowerRoI->isSaturated();
		fill("gTowers",Toweret,Towersaturationflag);


		float eta = gfexTowerRoI->eta();
		float phi = gfexTowerRoI->phi();
		Towereta = eta;
		Towerphi = phi;
		
		uint8_t etaidx = gfexTowerRoI->iEta();
		uint8_t phiidx = gfexTowerRoI->iPhi();
		int x = etaidx+1;
		int y = phiidx+1;
		binNumber = 40*(y-1)+x;
		
		
		if (Towersaturationflag == 1 && gfexTowerRoI->towerEt() >= maxet){
				maxet = gfexTowerRoI->towerEt();
		}

		if(gfexTowerRoI->towerEt() >= 200 ){
			nTowers++;
		}
		
		if (gfexTowerRoI->towerEt() >= 10){
			if (std::abs(eta) >= 3.3 ){
				Towerphi = phi - M_PI/32;
				fill("highEtgTowers",Towereta,Towerphi);
				Towerphi = phi + M_PI/32;
				fill("highEtgTowers",Towereta,Towerphi);
				fill("highEtgTowers",lbn,binNumber);
			} else {
				fill("highEtgTowers",Towereta,Towerphi);
				fill("highEtgTowers",lbn,binNumber);
			}
			
		}
     // only for h_gTower_coldtowers_etaphimap
		else if (gfexTowerRoI->towerEt() <= -10){
			if (std::abs(eta) >= 3.3 ){
				Towerphi = phi - M_PI/32;
				fill("lowEtgTowers",Towereta,Towerphi);
				Towerphi = phi + M_PI/32;
				fill("lowEtgTowers",Towereta,Towerphi);
				fill("lowEtgTowers",lbn,binNumber);
			} else {
				fill("lowEtgTowers",Towereta,Towerphi);
				fill("lowEtgTowers",lbn,binNumber);
				
			}
		}
	}

	nGfexTowers = nTowers;
	fill ("highEtgTowers",lbn,nGfexTowers);


	return StatusCode::SUCCESS;
}

