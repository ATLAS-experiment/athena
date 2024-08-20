/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
   */

#include "GfexInputMonitorAlgorithm.h"
#include "TProfile2D.h"
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
	auto Towerfpga = Monitored::Scalar<uint8_t>("TowerFpga",0.0);
	auto Maxet = Monitored::Scalar<int>("MaxEt",0.0);
	auto Towersaturationflag = Monitored::Scalar<char>("TowerSaturationflag",0.0);
	auto Toweret = Monitored::Scalar<int>("TowerEt",0);

	unsigned int nTowers = 0;
	auto maxet = 0.0;

	for(const xAOD::gFexTower* gfexTowerRoI : *gFexTowerContainer){


		Toweret=gfexTowerRoI->towerEt();
		Towersaturationflag=gfexTowerRoI->isSaturated();
		fill("gTowers",Toweret,Towersaturationflag);


		Towereta=gfexTowerRoI->eta();
		Towerphi=gfexTowerRoI->phi();

		if (Towersaturationflag == 1 && gfexTowerRoI->towerEt() >= maxet){
				maxet = gfexTowerRoI->towerEt();
		}


		if (Toweret >= 200){
			nTowers++;

			Towerfpga=gfexTowerRoI->fpga();
			fill("highEtgTowers",Towereta,Towerphi,Toweret,Towerfpga);
		}

		else {
			fill("lowEtgTowers",Towereta,Towerphi);
		}
	}

	Maxet = maxet;
	if (Maxet != 0.0 )
		fill ("gTowers", Maxet);

	if (nTowers != 0) {
		nGfexTowers = nTowers;
		fill ("highEtgTowers",nGfexTowers);
	}

	return StatusCode::SUCCESS;
}

