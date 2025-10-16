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
    ATH_CHECK( m_gFexEmulatedTowerKey.initialize(SG::AllowEmpty) );

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
    auto TowerId = Monitored::Scalar<uint32_t>("TowerId",0);
	auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());
    auto lbnString = Monitored::Scalar<std::string>("LBNString",std::to_string(GetEventInfo(ctx)->lumiBlock()));
    auto lbn = Monitored::Scalar<int>("LBN",GetEventInfo(ctx)->lumiBlock());
	auto binNumber = Monitored::Scalar<int>("binNumber",0);

    std::map<uint32_t, const xAOD::gFexTower*> emulatedTowers;
    if(!m_gFexEmulatedTowerKey.empty()) {
        SG::ReadHandle<xAOD::gFexTowerContainer> gFexEmulatedTowerContainer{m_gFexEmulatedTowerKey, ctx};
        if(!gFexEmulatedTowerContainer.isValid()){
            ATH_MSG_ERROR("No gFex Emulated Tower container found in storegate  "<< m_gFexEmulatedTowerKey);
            return StatusCode::FAILURE;
        }

        for(const xAOD::gFexTower* tower : *gFexEmulatedTowerContainer){
            if(emulatedTowers.find(tower->gFEXtowerID())!=emulatedTowers.end()) {
                ATH_MSG_WARNING("Duplicate towers with ID = " << tower->gFEXtowerID());
            }
            emulatedTowers[tower->gFEXtowerID()] = tower;
        }
    }

    auto Decision = Monitored::Scalar<std::string>("Error", "");
    auto refTowerET = Monitored::Scalar<int>("RefTowerEt",0);
    auto refTowerSat = Monitored::Scalar<char>("RefTowerSat",0.0);
    auto FillTree = Monitored::Scalar<bool>("FillTree",true);

    unsigned int nTowers = 0;
    for(const xAOD::gFexTower* gfexTowerRoI : *gFexTowerContainer){
		// working with "local" fiber number, iFiber
        unsigned int towerID = gfexTowerRoI->gFEXtowerID();
        unsigned int offset = (towerID > 20000) ? 20000 : (towerID > 10000 && towerID < 20000) ? 10000 : 0;
        unsigned int iFiber = (towerID - offset)/16;

        // Do not exceed maximum number of fibers for FPGA
        unsigned int maxFiberN =  (towerID > 20000) ? LVL1::gFEXPos::C_FIBERS : LVL1::gFEXPos::AB_FIBERS;
        if (iFiber >= maxFiberN) continue;

        int fiber_type  = (towerID < 10000) ? LVL1::gFEXPos::AMPD_NFI[iFiber] :
          (towerID > 10000 && towerID < 20000) ? LVL1::gFEXPos::BMPD_NFI[iFiber] :
          LVL1::gFEXPos::CMPD_NFI[iFiber];

        // Data Type: 1 is Tile
        int dataType = (towerID < 10000) ? LVL1::gFEXPos::AMPD_DTYP_ARR[fiber_type][towerID%16] :
          (towerID > 10000 && towerID < 20000) ? LVL1::gFEXPos::BMPD_DTYP_ARR[fiber_type][towerID%16] :
          LVL1::gFEXPos::CMPD_DTYP_ARR[fiber_type][towerID%16];


        Toweret=gfexTowerRoI->towerEt(); //returns MLE value
        Towersaturationflag=gfexTowerRoI->isSaturated();
        float eta = gfexTowerRoI->eta();
        float phi = gfexTowerRoI->phi();
        if (eta == 0.0 && phi == 0.0) continue; // skip the disconnected fibers

        if(!emulatedTowers.empty()) {
            Towereta = eta; Towerphi = phi;
            TowerId=gfexTowerRoI->gFEXtowerID();
            // compare to emulated towers
            auto eTowerItr = emulatedTowers.find(gfexTowerRoI->gFEXtowerID());
            if(eTowerItr == emulatedTowers.end()) {
                // missing emulated tower?
                Decision = "MissingTower";
                fill("errors",FillTree,Decision,lbn,evtNumber,TowerId,Towereta,Towerphi,Toweret,refTowerET,refTowerSat,Towersaturationflag);
                continue;
            }

            const auto eTower = eTowerItr->second;
            refTowerET = eTower->towerEt();
            refTowerSat = eTower->isSaturated();

            if(refTowerET != Toweret) {
                Decision = "ETMismatch";
                fill("errors",FillTree,Decision,lbn,evtNumber,TowerId,Towereta,Towerphi,Toweret,refTowerET,refTowerSat,Towersaturationflag);
            }
            if(refTowerSat != Towersaturationflag) {
                Decision = "SatMismatch";
                fill("errors",FillTree,Decision,lbn,evtNumber,TowerId,Towereta,Towerphi,Toweret,refTowerET,refTowerSat,Towersaturationflag);
            }

        }

        // Tile gTowers have dataType ==1
        if (dataType != 1) fill("gTowers",Toweret);
        else fill("gTileTowers",Toweret);

		if (eta < -3.17 && eta > -3.25){ eta = -3.225;}
		if (eta < 3.3 && eta > 3.17){ eta = 3.275;}

		Towereta = eta;	
			
		if(gfexTowerRoI->towerEt() >= 1662 ){
			nTowers++;
		}

		//looking at only saturated gTowers
		if (int(Towersaturationflag) == 1){	
			if (std::abs(eta) >= 3.2 ){ //FPGAc
				Towerphi = phi- 0.1;
				fill("SatgTowers",Towereta,Towerphi,Toweret);	
				Towerphi = phi + 0.1;
				fill("SatgTowers",Towereta,Towerphi,Toweret);
			} else { //FPGA a&b
				Towerphi = phi;
				fill("SatgTowers",Towereta,Towerphi,Toweret);
			}
		}


		//GREATER THAN 2GEV MLE=1342
		if (gfexTowerRoI->towerEt() >= 1342){
			if (std::abs(eta) >= 3.2 ){
				Towerphi = phi- 0.1;
				binNumber = getBinNumberTower(eta,phi-0.1,0,0);
				fill("highEtgTowers",Towereta,Towerphi,Toweret);
				fill("highEtgTowers",lbn,binNumber);	
				Towerphi = phi + 0.1;
				binNumber = getBinNumberTower(eta, phi+0.1,0,0);
				fill("highEtgTowers",Towereta,Towerphi,Toweret);
				fill("highEtgTowers",lbn,binNumber);
			} else {
				Towerphi = phi;
				binNumber = getBinNumberTower(eta,phi,0,0);
				fill("highEtgTowers",Towereta,Towerphi,Toweret);
				fill("highEtgTowers",lbn,binNumber);
			}
			
		}
      	//only for h_gTower_coldtowers_etaphimap MLE = 1182
		else if (gfexTowerRoI->towerEt() <= 1182){
			if (std::abs(eta) >= 3.2){
				Towerphi = phi- 0.1;
				binNumber = getBinNumberTower(eta,phi-0.1,0,0);
				fill("lowEtgTowers",Towereta,Towerphi,Toweret);
				fill("lowEtgTowers",lbn,binNumber);	
				Towerphi = phi + 0.1;
				binNumber = getBinNumberTower(eta, phi+0.1,0,0);
				fill("lowEtgTowers",Towereta,Towerphi,Toweret);
				fill("lowEtgTowers",lbn,binNumber);
			} else {
				Towerphi = phi;
				binNumber = getBinNumberTower(eta,phi,0,0);
				fill("lowEtgTowers",Towereta,Towerphi,Toweret);
				fill("lowEtgTowers",lbn,binNumber);
			}
		}
	}

	nGfexTowers = nTowers;
	fill ("highEtgTowers",lbn,nGfexTowers);

    
	return StatusCode::SUCCESS;
}

int GfexInputMonitorAlgorithm::getBinNumberTower (const float& inputEta, const float& inputPhi, int xbin, int ybin) const{
    const std::vector<float> eta = {-4.9, -4.1,-3.5,-3.25,-3.2,-3.1,-2.9,-2.7,-2.5,-2.2,-2.0,-1.8,-1.6,-1.4,-1.2,-1.0,-0.8,-0.6,-0.4,-0.2,0.0,0.2,0.4,0.6,0.8,1.0,1.2,1.4,1.6,1.8,2.0,2.2,2.5,2.7,2.9,3.1,3.25,3.3,3.5,4.1,4.9};

   for (int i = 0; i <= 40; i++){ 
       if (inputEta >= eta[i] && inputEta < eta[i+1]){
           xbin = i+1;
           continue;
        }
    }  
    int j=1;
	for (float phi = -3.2; phi <= 3.2;phi = phi+ 0.2){
        if (inputPhi >= phi && inputPhi < phi+0.2){
            ybin = j;
            break;
            }
        j++;
	}
    int binN = 32*(xbin-1)+ybin; 
    return binN;
}


