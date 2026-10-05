/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** Adapted from code by A.Hamilton to check trigger EDM; R.Goncalo 21/11/07 */

#include "TrigEDMChecker.h"

#include "AthViews/ViewHelper.h"
#include "AthViews/View.h"

#include "TrigConfHLTUtils/HLTUtils.h"
#include "TrigNavStructure/TriggerElement.h"
#include "TrigT1Interfaces/RecEmTauRoI.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/PhotonContainer.h"

#include "xAODJet/JetConstituentVector.h"
#include "xAODJet/JetContainer.h"

#include "xAODMuon/MuonContainer.h"

#include "xAODTau/TauDefs.h"
#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "xAODTrigBphys/TrigBphys.h"
#include "xAODTrigBphys/TrigBphysContainer.h"

#include "xAODTrigCalo/TrigEMCluster.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"

#include "xAODTrigEgamma/TrigElectron.h"
#include "xAODTrigEgamma/TrigElectronContainer.h"
#include "xAODTrigEgamma/TrigPhoton.h"
#include "xAODTrigEgamma/TrigPhotonContainer.h"

#include "xAODTrigMinBias/TrigSpacePointCounts.h"
#include "xAODTrigMinBias/TrigSpacePointCountsContainer.h"
#include "xAODTrigMinBias/TrigT2MbtsBits.h"
#include "xAODTrigMinBias/TrigT2MbtsBitsContainer.h"
#include "xAODTrigMinBias/TrigTrackCounts.h"
#include "xAODTrigMinBias/TrigTrackCountsContainer.h"
#include "xAODTrigMinBias/TrigVertexCounts.h"
#include "xAODTrigMinBias/TrigVertexCountsContainer.h"

#include "xAODTrigMissingET/TrigMissingETAuxContainer.h"
#include "xAODTrigMissingET/TrigMissingETContainer.h"

#include "xAODTrigger/TrigPassBits.h"
#include "xAODTrigger/TrigPassBitsContainer.h"
#include "xAODTrigger/TriggerMenuContainer.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigRoiConversion/RoiSerialise.h"
#include "xAODTrigger/RoiDescriptorStore.h"

#include <iostream>
#include <fstream>
#include <queue>


StatusCode TrigEDMChecker::initialize() {

  ATH_CHECK( m_navigationHandleKey.initialize() );
  ATH_CHECK( m_decisionsKey.initialize() );
  ATH_CHECK( m_navigationTool.retrieve() );

  ATH_MSG_DEBUG("Initializing TrigEDMChecker");

  ATH_MSG_INFO("REGTEST Initializing...");
  ATH_MSG_INFO("REGTEST m_doDumpAll                      = " << m_doDumpAll );
  ATH_MSG_INFO("REGTEST m_doDumpLVL1_ROI                 = " << m_doDumpLVL1_ROI);
  ATH_MSG_INFO("REGTEST m_doDumpxAODTrigMissingET        = " << m_doDumpxAODTrigMissingET );
  ATH_MSG_INFO("REGTEST m_doDumpTrigL2BphysContainer     = " << m_doDumpTrigL2BphysContainer );
  ATH_MSG_INFO("REGTEST m_doDumpTrigEFBphysContainer     = " << m_doDumpTrigEFBphysContainer );
  ATH_MSG_INFO("REGTEST m_doDumpxAODJetContainer         = " << m_doDumpxAODJetContainer );
  ATH_MSG_INFO("REGTEST m_doDumpxAODMuonContainer        = " << m_doDumpxAODMuonContainer );
  ATH_MSG_INFO("REGTEST m_doDumpxAODTrigElectronContainer= " << m_doDumpxAODTrigElectronContainer );
  ATH_MSG_INFO("REGTEST m_doDumpxAODTrigPhotonContainer  = " << m_doDumpxAODTrigPhotonContainer );
  ATH_MSG_INFO("REGTEST m_doDumpxAODElectronContainer    = " << m_doDumpxAODElectronContainer );
  ATH_MSG_INFO("REGTEST m_doDumpxAODPhotonContainer      = " << m_doDumpxAODPhotonContainer );
  ATH_MSG_INFO("REGTEST m_doDumpxAODTrackParticle        = " << m_doDumpxAODTrackParticle );
  ATH_MSG_INFO("REGTEST m_doDumpxAODVertex               = " << m_doDumpxAODVertex );
  ATH_MSG_INFO("REGTEST m_doDumpxAODTauJetContainer      = " << m_doDumpxAODTauJetContainer );
  ATH_MSG_INFO("REGTEST m_doDumpxAODTrigMinBias          = " << m_doDumpxAODTrigMinBias );
  ATH_MSG_INFO("REGTEST m_doDumpStoreGate                = " << m_doDumpStoreGate );
  ATH_MSG_INFO("REGTEST m_doDumpAllTrigComposite         = " << m_doDumpAllTrigComposite );
  ATH_MSG_INFO("REGTEST m_dumpTrigCompositeContainers    = " << m_dumpTrigCompositeContainers );
  ATH_MSG_INFO("REGTEST m_doDumpTrigCompsiteNavigation   = " << m_doDumpTrigCompsiteNavigation );
  ATH_MSG_INFO("REGTEST m_doTDTCheck                     = " << m_doTDTCheck );

  if(m_doDumpxAODMuonContainer || m_doDumpAll) {
    ATH_CHECK( m_muonPrinter.retrieve() );
  }
  else m_muonPrinter.disable();   // to avoid auto-retrieval

  if (m_doDumpTrigCompsiteNavigation) {
    ATH_CHECK( m_clidSvc.retrieve() );
  }

  if (m_doTDTCheck || m_doDumpTrigCompsiteNavigation) {
    ATH_CHECK( m_trigDec.retrieve() );
    ATH_MSG_INFO("TDT Executing with navigation format: " << m_trigDec->getNavigationFormat());
  }
  ATH_CHECK(m_muonTracksKey.initialize(m_doDumpAll || m_doTDTCheck));

  return StatusCode::SUCCESS;
}


StatusCode TrigEDMChecker::execute(const EventContext& ctx) {

    ATH_MSG_INFO( " ==========START of event===========" );

	if(m_doDumpAll || m_doDumpLVL1_ROI ){
		StatusCode sc = dumpLVL1_ROI();
		if (sc.isFailure()) {
          ATH_MSG_ERROR("The method dumpLVL1_ROI() failed");
		}
	}

	if(m_doDumpAll || m_doDumpxAODTrigMissingET){
		StatusCode sc = dumpxAODTrigMissingET();
		if (sc.isFailure()) {
          ATH_MSG_ERROR("The method dumpxAODTrigMissingET() failed");
		}
    }

    if(m_doDumpAll || m_doDumpxAODTrigEMCluster){
      StatusCode sc = dumpxAODTrigEMCluster();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpxAODTrigEMCluster() failed");
      }
    }

    if(m_doDumpAll || m_doDumpxAODTrigEMClusterContainer){
      StatusCode sc = dumpxAODTrigEMClusterContainer();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpxAODTrigEMClusterContainer() failed");
      }
    }

	if(m_doDumpAll || m_doDumpxAODJetContainer){
		StatusCode sc = dumpxAODJetContainer();
		if (sc.isFailure()) {
          ATH_MSG_ERROR("The method dumpxAODJetContainer() failed");          
		}
	}

	if(m_doDumpAll || m_doDumpTrigL2BphysContainer){
		StatusCode sc = dumpTrigL2BphysContainer();
		if (sc.isFailure()) {
          ATH_MSG_ERROR("The method dumpTrigL2BphysContainer() failed");
		}
	}

	if(m_doDumpAll || m_doDumpTrigEFBphysContainer){
		StatusCode sc = dumpTrigEFBphysContainer();
		if (sc.isFailure()) {
          ATH_MSG_ERROR("The method dumpTrigEFBphysContainer() failed");
		}
	}

	if(m_doDumpAll || m_doDumpxAODMuonContainer) {
	  StatusCode sc = dumpxAODMuonContainer();
	  if(sc.isFailure()) {
	    ATH_MSG_ERROR("The method dumpxAODMuonContainer() failed");
	  }
	}

	if(m_doDumpAll || m_doDumpxAODTrigElectronContainer){
		StatusCode sc = dumpxAODTrigElectronContainer();
		if (sc.isFailure()) {
          ATH_MSG_ERROR("The method dumpxAODTrigElectronContainer() failed");
		}
	}

	if(m_doDumpAll || m_doDumpxAODTrigPhotonContainer){
		StatusCode sc = dumpxAODTrigPhotonContainer();
		if (sc.isFailure()) {
          ATH_MSG_ERROR("The method dumpxAODTrigElectronContainer() failed");
		}
	}

    if(m_doDumpAll || m_doDumpxAODElectronContainer){
      StatusCode sc = dumpxAODElectronContainer();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpxAODTrigElectronContainer() failed");
      }
	}

	if(m_doDumpAll || m_doDumpxAODPhotonContainer){
      StatusCode sc = dumpxAODPhotonContainer();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpxAODTrigElectronContainer() failed");        
      }
	}

	if(m_doDumpAll || m_doDumpxAODTauJetContainer){
      StatusCode sc = dumpxAODTauJetContainer();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpxAODTauJetContainer() failed");
      }
	}

	if(m_doDumpAll || m_doDumpxAODTrackParticle){
      StatusCode sc = dumpxAODTrackParticle();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpxAODTrackParticle() failed");      
      }
	}

	if(m_doDumpAll || m_doDumpxAODVertex){
      StatusCode sc = dumpxAODVertex();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpxAODVertex() failed");
      }
	}

	if (m_doDumpAll || m_doDumpxAODTrigMinBias){
      StatusCode sc = dumpxAODTrigMinBias();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpxAODTrigMinBias() failed");
      }
	}

	if (m_doDumpTrigPassBits){
      StatusCode sc = dumpTrigPassBits();
      if (sc.isFailure()) {
        ATH_MSG_ERROR("The method dumpTrigPassBits() failed");
      }      
	}
	
  if (m_doDumpAll || m_doDumpStoreGate) {
    ATH_MSG_DEBUG(evtStore()->dump());
  }

  if (m_doDumpAll || m_doDumpNavigation) {
    StatusCode sc = dumpNavigation(ctx);
    if ( sc.isFailure() ) {
      ATH_MSG_ERROR("The method dumpNavigation() failed");
    }
  }

  if (m_doDumpAll || m_doTDTCheck) {
    ATH_CHECK(dumpTDT(ctx));
  }

  if (m_doDumpAll || m_doDumpAllTrigComposite || m_dumpTrigCompositeContainers.size() > 0) {
  	ATH_CHECK( dumpTrigComposite() );
  }

  if (m_doDumpAll || m_doDumpTrigCompsiteNavigation) {
    std::string trigCompositeSteering;
    bool pass;
    ATH_CHECK(TrigCompositeNavigationToDot(trigCompositeSteering, pass));
    const std::string evtNumber = std::to_string(ctx.eventID().event_number());
    const std::string passStr = (pass ? "Pass" : "Fail"); 
    std::ofstream ofile(std::string("NavGraph_" + m_dumpNavForChain + "_Ev" + evtNumber + "_" + passStr + ".dot").c_str());
    ofile << trigCompositeSteering;
  }



  ATH_MSG_INFO( " ==========END of event===========" );
	return StatusCode::SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////
StatusCode TrigEDMChecker::dumpTrigPassBits(){
    const std::string name="HLT_xAOD__TrigPassBitsContainer_passbits";
    const xAOD::TrigPassBitsContainer *xbitscont=nullptr;
    StatusCode sc = evtStore()->retrieve(xbitscont,name);
    if (sc.isFailure() ){
        ATH_MSG_INFO("Cannot retrieve TrigPassBits");
    }
    else {
        ATH_MSG_INFO("Size of PassBits container : " << xbitscont->size());
        for(const auto bits:*xbitscont){
            if(bits==nullptr){
                ATH_MSG_INFO("TrigPassBits point nullptr ");
                continue;
            }
            ATH_MSG_DEBUG("Analyzing bits for " << bits->containerClid() << " of size " << bits->size() << " with bit size " << bits->passBits().size());
        }

        for(const xAOD::TrigPassBits* bits : *xbitscont){
            if(bits==nullptr){
                ATH_MSG_INFO("TrigPassBits point nullptr ");
                continue;
            }
            ATH_MSG_DEBUG("Analyzing bits for " << bits->containerClid() << " of size " << bits->size() << " with bit size " << bits->passBits().size());
        }
    }
    return StatusCode::SUCCESS;
}

void TrigEDMChecker::dumpTrigSpacePointCounts()
{
    ATH_MSG_INFO("MinBias in dumpTrigSpacePointCounts()");

	std::string METTag="HLT_xAOD__TrigSpacePointCountsContainer_spacepoints";

	const xAOD::TrigSpacePointCountsContainer* SpacePointCountsCont=0;
	StatusCode sc = evtStore()->retrieve(SpacePointCountsCont,METTag);

	if (sc.isFailure())
      ATH_MSG_INFO("failed to retrieve " << METTag);
    else {
      ATH_MSG_INFO("Accessing " << METTag << " with " << SpacePointCountsCont->size() << " elements");
      
      std::string s; char buff[128];
      std::vector<float> getVec;
      float sum;
      
      // Loop over container content
      for(uint i = 0; i < SpacePointCountsCont->size(); i++) {
        getVec = SpacePointCountsCont->at(i)->contentsPixelClusEndcapC();
        sum = 0.;
        for (uint j = 0; j < getVec.size(); ++j) sum += getVec[j];
        snprintf(buff, sizeof(buff), "REGTEST %s SUM of contentsPixelClusEndcapC() =         %10.2f ", s.c_str(), sum );
        ATH_MSG_INFO(buff);
        
        getVec = SpacePointCountsCont->at(i)->contentsPixelClusBarrel();
        sum = 0.;
        for (uint j = 0; j < getVec.size(); ++j) sum += getVec[j];
        snprintf(buff, sizeof(buff), "REGTEST %s SUM of contentsPixelClusBarrel() =         %10.2f ", s.c_str(), sum );
        ATH_MSG_INFO(buff);
        
        getVec = SpacePointCountsCont->at(i)->contentsPixelClusEndcapA();
        sum = 0.;
        for (uint j = 0; j < getVec.size(); ++j) sum += getVec[j];
        snprintf(buff, sizeof(buff), "REGTEST %s SUM of contentsPixelClusEndcapA() =         %10.2f ", s.c_str(), sum );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s pixelClusTotBins() =        %u ", s.c_str(), SpacePointCountsCont->at(i)->pixelClusTotBins() );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s pixelClusTotMin() =        %10.2f ", s.c_str(), SpacePointCountsCont->at(i)->pixelClusTotMin() );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s pixelClusTotMax() =        %10.2f ", s.c_str(), SpacePointCountsCont->at(i)->pixelClusTotMax() );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s pixelClusSizeBins() =        %u ", s.c_str(), SpacePointCountsCont->at(i)->pixelClusSizeBins() );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s pixelClusSizeMin()  =        %10.2f ", s.c_str(), SpacePointCountsCont->at(i)->pixelClusSizeMin() );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s pixelClusSizeMax() =        %10.2f ", s.c_str(), SpacePointCountsCont->at(i)->pixelClusSizeMax() );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s sctSpEndcapC() =        %u ", s.c_str(), SpacePointCountsCont->at(i)->sctSpEndcapC() );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s sctSpBarrel() =        %u ", s.c_str(), SpacePointCountsCont->at(i)->sctSpBarrel() );
        ATH_MSG_INFO(buff);
        
        snprintf(buff, sizeof(buff), "REGTEST %s sctSpEndcapA() =        %u ", s.c_str(), SpacePointCountsCont->at(i)->sctSpEndcapA() );
        ATH_MSG_INFO(buff);
      }
	}
}

void TrigEDMChecker::dumpTrigT2MBTSBits(){
  ATH_MSG_INFO("MinBias in dumpTrigT2MBTSBits()");

  std::string METTag="HLT_xAOD__TrigT2MbtsBitsContainer_T2Mbts";

  const xAOD::TrigT2MbtsBitsContainer* T2MbtsBitsCont=0;
  StatusCode sc = evtStore()->retrieve(T2MbtsBitsCont,METTag);

  if (sc.isFailure())
    ATH_MSG_INFO("failed to retrieve " << METTag);
  else {
    ATH_MSG_INFO("Accessing " << METTag << " with " << T2MbtsBitsCont->size() << " elements");

    std::string s; char buff[380];
    std::vector<float> getVec;
    float sum;

    // Loop over container content
    for(uint i = 0; i < T2MbtsBitsCont->size(); i++) {
      getVec = T2MbtsBitsCont->at(i)->triggerEnergies();
      sum = 0.;
      for (uint j = 0; j < getVec.size(); ++j) sum += getVec[j];
      snprintf(buff, sizeof(buff), "REGTEST %s SUM of triggerEnergies() =         %10.2f ", s.c_str(), sum );
      ATH_MSG_INFO(buff);

      getVec = T2MbtsBitsCont->at(i)->triggerTimes();
      sum = 0.;
      for (uint j = 0; j < getVec.size(); ++j) sum += getVec[j];
      snprintf(buff, sizeof(buff), "REGTEST %s SUM of triggerTimes() =         %10.2f ", s.c_str(), sum );
      ATH_MSG_INFO(buff);
    }
  }
}

void TrigEDMChecker::dumpTrigVertexCounts(){
  ATH_MSG_INFO("MinBias in dumpTrigVertexCounts()");

  std::string METTag="HLT_xAOD__TrigVertexCountsContainer_vertexcounts";

  const xAOD::TrigVertexCountsContainer* T2VertexCountsCont=0;
  StatusCode sc = evtStore()->retrieve(T2VertexCountsCont,METTag);

  if (sc.isFailure())
    ATH_MSG_INFO("failed to retrieve " << METTag);
  else {
    ATH_MSG_INFO("Accessing " << METTag << " with " << T2VertexCountsCont->size() << " elements");

    std::string s; char buff[380];
    std::vector<float> fgetVec;
    float fsum(0.);
    std::vector<unsigned int> ugetVec;
    unsigned int usum(0);

    // Loop over container content
    for(uint i = 0; i < T2VertexCountsCont->size(); i++) {
      ugetVec = T2VertexCountsCont->at(i)->vtxNtrks();
      for (uint j = 0; j < ugetVec.size(); ++j) usum += ugetVec[j];
      snprintf(buff, sizeof(buff), "REGTEST %s SUM of vtxNtrks() =         %u ", s.c_str(), usum );
      ATH_MSG_INFO(buff);

      fgetVec = T2VertexCountsCont->at(i)->vtxTrkPtSqSum();
      for (uint j = 0; j < fgetVec.size(); ++j) fsum += fgetVec[j];
      snprintf(buff, sizeof(buff), "REGTEST %s SUM of vtxTrkPtSqSum() =         %10.2f ", s.c_str(), fsum );
      ATH_MSG_INFO(buff);
    }
  }
}

void TrigEDMChecker::dumpTrigTrackCounts(){
  ATH_MSG_INFO("MinBias in dumpTrigTrackCounts()");

  std::string METTag="HLT_xAOD__TrigTrackCountsContainer_trackcounts";

  const xAOD::TrigTrackCountsContainer* T2TrackCountsCont=0;
  StatusCode sc = evtStore()->retrieve(T2TrackCountsCont,METTag);

  if (sc.isFailure())
    ATH_MSG_INFO("failed to retrieve " << METTag);
  else {
    ATH_MSG_INFO("Accessing " << METTag << " with " << T2TrackCountsCont->size() << " elements");

    std::string s; char buff[380];
    std::vector<float> getVec;
    float sum;

    // Loop over container content
    for(uint i = 0; i < T2TrackCountsCont->size(); i++) {
      getVec = T2TrackCountsCont->at(i)->z0_pt();
      sum = 0.;
      for (uint j = 0; j < getVec.size(); ++j) sum += getVec[j];
      snprintf(buff, sizeof(buff), "REGTEST %s SUM of z0_pt =         %10.2f ", s.c_str(), sum );
      ATH_MSG_INFO(buff);

      getVec = T2TrackCountsCont->at(i)->eta_phi();
      sum = 0.;
      for (uint j = 0; j < getVec.size(); ++j) sum += getVec[j];
      snprintf(buff, sizeof(buff), "REGTEST %s SUM of eta_phi() =         %10.2f ", s.c_str(), sum );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s z0Bins() =        %u ", s.c_str(), T2TrackCountsCont->at(i)->z0Bins() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s z0Min() =        %10.2f ", s.c_str(), T2TrackCountsCont->at(i)->z0Min() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s z0Max() =        %10.2f ", s.c_str(), T2TrackCountsCont->at(i)->z0Max() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s ptBins() =        %u ", s.c_str(), T2TrackCountsCont->at(i)->ptBins() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s ptMin() =        %10.2f ", s.c_str(), T2TrackCountsCont->at(i)->ptMin() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s ptMax() =        %10.2f ", s.c_str(), T2TrackCountsCont->at(i)->ptMax() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s etaBins() =        %u ", s.c_str(), T2TrackCountsCont->at(i)->etaBins() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s etaMin() =        %10.2f ", s.c_str(), T2TrackCountsCont->at(i)->etaMin() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s etaMax() =        %10.2f ", s.c_str(), T2TrackCountsCont->at(i)->etaMax() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s phiBins() =        %u ", s.c_str(), T2TrackCountsCont->at(i)->phiBins() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s phiMin() =        %10.2f ", s.c_str(), T2TrackCountsCont->at(i)->phiMin() );
      ATH_MSG_INFO(buff);

      snprintf(buff, sizeof(buff), "REGTEST %s phiMax() =        %10.2f ", s.c_str(), T2TrackCountsCont->at(i)->phiMax() );
      ATH_MSG_INFO(buff);
    }
  }
}

StatusCode TrigEDMChecker::dumpxAODTrigMinBias() {

	dumpTrigSpacePointCounts();
	dumpTrigT2MBTSBits();
	dumpTrigVertexCounts();
	dumpTrigTrackCounts();

	return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////



StatusCode TrigEDMChecker::dumpxAODTrigMissingET() {

  ATH_MSG_INFO("dumpxAODTrigMissingET()");

  int ntag=4;
  std::string METTags[]={"HLT_xAOD__TrigMissingETContainer_EFJetEtSum","HLT_xAOD__TrigMissingETContainer_TrigEFMissingET", "HLT_xAOD__TrigMissingETContainer_TrigL2MissingET_FEB","HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl"};

  for(int itag=0; itag <ntag; itag++) {

    const xAOD::TrigMissingETContainer* MissingETCont=0;
    StatusCode sc = evtStore()->retrieve(MissingETCont,METTags[itag]);
    if (sc.isFailure())
      ATH_MSG_INFO("failed to retrieve " << METTags[itag]);
    else {
      ATH_MSG_INFO("Accessing " << METTags[itag] << " with " << MissingETCont->size() << " elements");

      // Loop over container content
      for(uint i = 0; i < MissingETCont->size(); i++) {

        std::string s; char buff[3000];
             
        snprintf(buff, sizeof(buff), "REGTEST %s Ex =         %10.2f CLHEP::MeV", s.c_str(), MissingETCont->at(i)->ex() );
        ATH_MSG_INFO(buff);
        snprintf(buff, sizeof(buff), "REGTEST %s Ey =         %10.2f CLHEP::MeV", s.c_str(), MissingETCont->at(i)->ey() );
        ATH_MSG_INFO(buff);
        snprintf(buff, sizeof(buff), "REGTEST %s Ez =         %10.2f CLHEP::MeV", s.c_str(), MissingETCont->at(i)->ez() );
        ATH_MSG_INFO(buff);
        snprintf(buff, sizeof(buff), "REGTEST %s SumET =         %10.2f CLHEP::MeV", s.c_str(), MissingETCont->at(i)->sumEt() );
        ATH_MSG_INFO(buff);
        snprintf(buff, sizeof(buff), "REGTEST %s SumE =       %10.2f CLHEP::MeV", s.c_str(), MissingETCont->at(i)->sumE() );
        ATH_MSG_INFO(buff);
        snprintf(buff, sizeof(buff), "REGTEST %s Flag =       %d", s.c_str(), MissingETCont->at(i)->flag() );
        ATH_MSG_INFO(buff);
        snprintf(buff, sizeof(buff), "REGTEST %s Flag =       %d", s.c_str(), MissingETCont->at(i)->roiWord() );
        ATH_MSG_INFO(buff);
             
        unsigned int Nc = MissingETCont->at(i)->getNumberOfComponents();
        if (Nc > 0) { 
          s="REGTEST __name____status_usedChannels__sumOfSigns__calib1_calib0";
          s+="/MeV__ex/MeV_____ey/MeV_____ez/MeV___sumE/MeV__sumEt/CLHEP::MeV";
          ATH_MSG_INFO(s);
        }
             
        for(uint j = 0; j < Nc; j++) {
             
          std::string name =               MissingETCont->at(i)->nameOfComponent(j);
          const short status =             MissingETCont->at(i)->statusComponent(j);
          const unsigned short usedChan =  MissingETCont->at(i)->usedChannelsComponent(j);
          const short sumOfSigns =         MissingETCont->at(i)->sumOfSignsComponent(j);
          const float calib0 =             MissingETCont->at(i)->calib0Component(j);
          const float calib1 =             MissingETCont->at(i)->calib1Component(j);
          const float ex =                 MissingETCont->at(i)->exComponent(j);
          const float ey =                 MissingETCont->at(i)->eyComponent(j);
          const float ez =                 MissingETCont->at(i)->ezComponent(j);
          const float sumE =               MissingETCont->at(i)->sumEComponent(j);
          const float sumEt =              MissingETCont->at(i)->sumEtComponent(j);

          snprintf(buff, sizeof(buff),
                   "REGTEST   %s   %6d %12d %10d   %6.2f  %6.3f %10.2f %10.2f %10.2f %10.2f %10.2f",
                   name.c_str(), status, usedChan, sumOfSigns, calib1, calib0,
                   ex, ey, ez, sumE, sumEt);
          ATH_MSG_INFO(buff);		
        }              
      }
    }
  }

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////

StatusCode TrigEDMChecker::dumpLVL1_ROI() {

  ATH_MSG_DEBUG("in dumpLVL1_ROI()");

  ATH_MSG_INFO("REGTEST ==========START of LVL1_ROI DUMP===========");

  const LVL1_ROI * lvl1ROI;
  StatusCode sc = evtStore()->retrieve(lvl1ROI);
  if (sc.isFailure() ) {
    ATH_MSG_INFO("REGTEST No LVL1_ROI found");
    return  StatusCode::SUCCESS;
  }

  ATH_MSG_INFO("REGTEST LVL1_ROI retrieved");

  LVL1_ROI::emtaus_type::const_iterator itEMTau   =
    (lvl1ROI->getEmTauROIs()).begin();
  LVL1_ROI::emtaus_type::const_iterator itEMTau_e =
    (lvl1ROI->getEmTauROIs()).end();
  int j = 0;
  for( ; itEMTau != itEMTau_e; ++itEMTau, ++j) {
    ATH_MSG_INFO("REGTEST Looking at LVL1_ROI " << j);
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI Eta     is " << itEMTau->getEta());
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI Phi     is " << itEMTau->getPhi());
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI EMClus  is " << itEMTau->getEMClus());
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI TauClus is " << itEMTau->getTauClus());
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI EMIsol  is " << itEMTau->getEMIsol());
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI HadIsol is " << itEMTau->getHadIsol());
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI Core    is " << itEMTau->getCore());
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI HadCore is " << itEMTau->getHadCore());
    ATH_MSG_INFO("REGTEST LVL1 EmTauROI roiWord is " << itEMTau->getROIWord());
  }

  ATH_MSG_INFO("REGTEST ==========END of LVL1_ROI DUMP===========");
  ATH_MSG_DEBUG("dumpLVL1_ROI() succeeded");
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////

StatusCode TrigEDMChecker::dumpxAODMuonContainer() {

  ATH_MSG_DEBUG("In dumpxAODMuonContainer");

  ATH_MSG_INFO( "REGTEST ==========START of xAOD::MuonContainer DUMP===========" );

  const xAOD::MuonContainer* muonCont=0;
  StatusCode sc = evtStore()->retrieve(muonCont,"HLT_xAOD__MuonContainer_MuonEFInfo");
  if (sc.isFailure()) {
    ATH_MSG_INFO("REGTEST No muon container HLT_xAOD__MuonContainer_MuonEFInfo");
    return StatusCode::SUCCESS;
  }

  std::string output = m_muonPrinter->print( *muonCont );
  msg(MSG::INFO) << output << endmsg;

  ATH_MSG_INFO( "REGTEST ==========END of xAOD::MuonContainer DUMP===========" );

  return StatusCode::SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////

StatusCode TrigEDMChecker::dumpxAODTrigElectronContainer() {

  ATH_MSG_DEBUG("In dumpxAODElectronContainer");

  ATH_MSG_INFO( "REGTEST ==========START of xAOD::TrigElectronContainer DUMP===========" );

  const xAOD::TrigElectronContainer* elCont=0;
  StatusCode sc = evtStore()->retrieve(elCont,"HLT_xAOD__TrigElectronContainer_L2ElectronFex");
  if (sc.isFailure()) {
    ATH_MSG_INFO("REGTEST No TrigElectron container HLT_xAOD__TrigElectronContainer_L2ElectronFex");
    return StatusCode::SUCCESS;
  }

  for (const auto eg : *elCont){
      ATH_MSG_INFO("REGTEST TrigElectron->Phi() returns " << eg->phi());
      ATH_MSG_INFO("REGTEST TrigElectron->Eta() returns " << eg->eta());
      ATH_MSG_INFO("REGTEST TrigElectron->rEta returns " << eg->rcore());
      ATH_MSG_INFO("REGTEST TrigElectron->eratio() returns " << eg->eratio());
      ATH_MSG_INFO("REGTEST TrigElectron->pt() returns " << eg->pt());
      ATH_MSG_INFO("REGTEST TrigElectron->etHad() returns " << eg->etHad());
      ATH_MSG_INFO("REGTEST TrigElectron->f1() returns " << eg->f1());
      ATH_MSG_INFO("REGTEST TrigElectron caloEta = " << eg->caloEta());
      ATH_MSG_INFO("REGTEST TrigElectron dPhiCalo" << eg->trkClusDphi());
      ATH_MSG_INFO("REGTEST TrigElectron dEtaCalo" << eg->trkClusDeta());
      ATH_MSG_INFO("REGTEST TrigElectron pTcalo" << eg->pt());
      ATH_MSG_INFO("REGTEST TrigElectron eTOverPt" << eg->etOverPt());
      ATH_MSG_INFO("REGTEST TrigElectron nTRTHits" << eg->nTRTHits());
      ATH_MSG_INFO("REGTEST TrigElectron nStrawHits" << eg->nTRTHiThresholdHits());
      ATH_MSG_INFO("REGTEST TrigElectron Check EMCluster");
      if(eg->emCluster()){
          ATH_MSG_INFO("REGTEST TrigElectron EMCluster retrieved");
          ATH_MSG_INFO("REGTEST TrigElectron emCluster->energy() returns " << eg->emCluster()->energy());
          ATH_MSG_INFO("REGTEST TrigElectron emCluster->phi() returns " << eg->emCluster()->phi());
          ATH_MSG_INFO("REGTEST TrigElectron emCluster->eta() returns " << eg->emCluster()->eta());
          ATH_MSG_INFO("REGTEST TrigElectron emCluster check Element Link");
          ATH_MSG_INFO("REGTEST TrigElectron emCluster energy = " << eg->emCluster()->energy());
          ATH_MSG_INFO("REGTEST TrigElectron ElementLink emCluster energy = " << (*eg->emClusterLink())->energy());
      }
      else  ATH_MSG_INFO("REGTEST TrigElectron No EMCluster retrieved!");
      ATH_MSG_INFO("REGTEST TrigElectron Check TrackParticle");
      if(eg->trackParticle()){
          ATH_MSG_INFO("REGTEST TrigElectron TrackParticle retrieved");
          ATH_MSG_INFO("REGTEST TrigElectron trackParticle->pt() returns " << eg->trackParticle()->pt());
          ATH_MSG_INFO("REGTEST TrigElectron trackParticle->phi() returns " << eg->trackParticle()->phi());
          ATH_MSG_INFO("REGTEST TrigElectron trackParticle->eta() returns " << eg->trackParticle()->eta());
          ATH_MSG_INFO("REGTEST TrigElectron check TrackParticle Element Link");
          ATH_MSG_INFO("REGTEST TrigElectron TrackParticle pt = " << eg->trackParticle()->pt());
          ATH_MSG_INFO("REGTEST TrigElectron ElementLink TrackParticle pt = " << (*eg->trackParticleLink())->pt());
      }
      else  ATH_MSG_INFO("REGTEST TrigElectron No TrackParticle retrieved!");
  }
  ATH_MSG_INFO( "REGTEST ==========END of xAOD::TrigElectronContainer DUMP===========" );

  return StatusCode::SUCCESS;

}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
StatusCode TrigEDMChecker::dumpxAODTrigPhotonContainer() {

  ATH_MSG_DEBUG("In dumpxAODTrigPhotonContainer");

  ATH_MSG_INFO( "REGTEST ==========START of xAOD::TrigPhotonContainer DUMP===========" );

  const xAOD::TrigPhotonContainer* phCont=0;
  StatusCode sc = evtStore()->retrieve(phCont,"HLT_xAOD__TrigPhotonContainer_L2PhotonFex");
  if (sc.isFailure()) {
    ATH_MSG_INFO("REGTEST No TrigPhoton container HLT_xAOD__TrigPhotonContainer_L2PhotonFex");
    return StatusCode::SUCCESS;
  }

  for (const auto eg : *phCont){

      ATH_MSG_INFO("REGTEST TrigPhoton->Phi() returns " << eg->phi());
      ATH_MSG_INFO("REGTEST TrigPhoton->Eta() returns " << eg->eta());
      ATH_MSG_INFO("REGTEST TrigPhoton->dPhi() returns " << eg->dPhi());
      ATH_MSG_INFO("REGTEST TrigPhoton->dEta() returns " << eg->dEta());
      ATH_MSG_INFO("REGTEST TrigPhoton->rEta returns " << eg->rcore());
      ATH_MSG_INFO("REGTEST TrigPhoton->eratio() returns " << eg->eratio());
      ATH_MSG_INFO("REGTEST TrigPhoton->pt() returns " << eg->pt());
      ATH_MSG_INFO("REGTEST TrigPhoton->etHad() returns " << eg->etHad());
      ATH_MSG_INFO("REGTEST TrigPhoton->f1() returns " << eg->f1());
      ATH_MSG_INFO("REGTEST TrigPhoton Check EMCluster");
      if(eg->emCluster()){
          ATH_MSG_INFO("REGTEST TrigPhoton EMCluster retrieved");
          ATH_MSG_INFO("REGTEST TrigPhoton emCluster->energy() returns " << eg->emCluster()->energy());
          ATH_MSG_INFO("REGTEST TrigPhoton emCluster->phi() returns " << eg->emCluster()->phi());
          ATH_MSG_INFO("REGTEST TrigPhoton emCluster->eta() returns " << eg->emCluster()->eta());
          ATH_MSG_INFO("REGTEST TrigPhoton emCluster check Element Link");
          ATH_MSG_INFO("REGTEST TrigPhoton emCluster energy = " << eg->emCluster()->energy());
          ATH_MSG_INFO("REGTEST TrigPhoton ElementLink emCluster energy = " << (*eg->emClusterLink())->energy());
      }
      else  ATH_MSG_INFO("REGTEST TrigPhoton No EMCluster retrieved!");
  }
  ATH_MSG_INFO( "REGTEST ==========END of xAOD::TrigPhotonContainer DUMP===========" );

  return StatusCode::SUCCESS;

}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
StatusCode TrigEDMChecker::dumpxAODElectronContainer() {

  ATH_MSG_DEBUG("In dumpxAODElectronContainer");

  ATH_MSG_INFO( "REGTEST ==========START of xAOD::ElectronContainer DUMP===========" );

  const xAOD::ElectronContainer* elCont=0;
  StatusCode sc = evtStore()->retrieve(elCont,"HLT_xAOD__ElectronContainer_egamma_Electrons");
  if (sc.isFailure()) {
    ATH_MSG_INFO("REGTEST No Electron container HLT_xAOD__ElectronContainer_egamma_Electrons");
    return StatusCode::SUCCESS;
  }
  float val_float=-99;
  unsigned int isEMbit=0;
  bool pid=false;
  //DEBUG output for Egamma container
  ATH_MSG_INFO(" REGTEST: xAOD Reconstruction variables: ");
  //                //Cluster and ShowerShape info
  //
  static const SG::Accessor< float > accLH("LHValue");
  static const SG::Accessor< float > accLHCalo("LHCaloValue");
  static const SG::Accessor<ElementLink<xAOD::CaloClusterContainer> > orig ("originalCaloCluster");
  for (const auto eg : *elCont){
      //REGTEST printout
      if (eg) {
          ATH_MSG_INFO(" REGTEST: egamma energy: " << eg->e() );
          ATH_MSG_INFO(" REGTEST: egamma eta: " << eg->eta() );
          ATH_MSG_INFO(" REGTEST: egamma phi: " << eg->phi() );
          if(eg->selectionisEM(isEMbit,"isEMVLoose"))
              ATH_MSG_INFO(" REGTEST: isEMVLoose " << std::hex << isEMbit << std::dec);
          else ATH_MSG_WARNING(" REGTEST: Missing Aux info");
          if(eg->selectionisEM(isEMbit,"isEMLoose"))
              ATH_MSG_INFO(" REGTEST: isEMLoose " << std::hex << isEMbit << std::dec);
          else ATH_MSG_WARNING(" REGTEST: Missing Aux info");
          if(eg->selectionisEM(isEMbit,"isEMMedium"))
              ATH_MSG_INFO(" REGTEST: isEMMedium " << std::hex << isEMbit << std::dec);
          else ATH_MSG_WARNING(" REGTEST: Missing Aux info");
          if(eg->selectionisEM(isEMbit,"isEMTight"))
              ATH_MSG_INFO(" REGTEST: isEMTight " << std::hex << isEMbit << std::dec);
          else ATH_MSG_WARNING(" REGTEST: Missing Aux info");
          if(eg->selectionisEM(isEMbit,"isEMLHVLoose"))
              ATH_MSG_INFO(" REGTEST: isEMLHVLoose " << std::hex << isEMbit << std::dec);
          else ATH_MSG_WARNING(" REGTEST: Missing Aux info");
          if(eg->selectionisEM(isEMbit,"isEMLHLoose"))
              ATH_MSG_INFO(" REGTEST: isEMLHLoose " << std::hex << isEMbit << std::dec);
          else ATH_MSG_WARNING(" REGTEST: Missing Aux info");
          if(eg->selectionisEM(isEMbit,"isEMLHMedium"))
              ATH_MSG_INFO(" REGTEST: isEMLHMedium " << std::hex << isEMbit << std::dec);
          else ATH_MSG_WARNING(" REGTEST: Missing Aux info");
          if(eg->selectionisEM(isEMbit,"isEMLHTight"))
              ATH_MSG_INFO(" REGTEST: isEMLHTight " << std::hex << isEMbit << std::dec);
          else ATH_MSG_WARNING(" REGTEST: Missing Aux info");
          if(accLH.isAvailable(*eg))
              ATH_MSG_INFO(" REGTEST: LHValue " << accLH(*eg));
          else
              ATH_MSG_WARNING(" REGTEST: Missing Aux info ");
          if(accLHCalo.isAvailable(*eg))
              ATH_MSG_INFO(" REGTEST: LHValue " << accLHCalo(*eg));
          else
              ATH_MSG_WARNING(" REGTEST: Missing Aux info ");
          if(eg->passSelection(pid,"LHVLoose"))
              ATH_MSG_INFO(" REGTEST: LHVLoose " << pid); 
          else
              ATH_MSG_WARNING(" REGTEST: Missing Aux info ");
          if(eg->passSelection(pid,"LHLoose"))
              ATH_MSG_INFO(" REGTEST: LHLoose " << pid); 
          else
              ATH_MSG_WARNING(" REGTEST: Missing Aux info ");
          if(eg->passSelection(pid,"LHMedium"))
              ATH_MSG_INFO(" REGTEST: LHMedium " << pid); 
          else
              ATH_MSG_WARNING(" REGTEST: Missing Aux info ");
          if(eg->passSelection(pid,"LHTight"))
              ATH_MSG_INFO(" REGTEST: LHTight " << pid); 
          else
              ATH_MSG_WARNING(" REGTEST: Missing Aux info ");
      } else{
          ATH_MSG_INFO(" REGTEST: problems with egamma pointer" );
          return StatusCode::SUCCESS;
      }
      ATH_MSG_INFO(" REGTEST: caloCluster variables ");
      if (eg->caloCluster()) {
          ATH_MSG_INFO(" REGTEST: egamma cluster transverse energy: " << eg->caloCluster()->et() );
          ATH_MSG_INFO(" REGTEST: egamma cluster eta: " << eg->caloCluster()->eta() );
          ATH_MSG_INFO(" REGTEST: egamma cluster phi: " << eg->caloCluster()->phi() );
          double tmpeta = -999.;
          double tmpphi = -999.;
          eg->caloCluster()->retrieveMoment(xAOD::CaloCluster::ETACALOFRAME,tmpeta);
          eg->caloCluster()->retrieveMoment(xAOD::CaloCluster::PHICALOFRAME,tmpphi); 
          ATH_MSG_INFO(" REGTEST: egamma cluster calo-frame coords. etaCalo = " << tmpeta); 
          ATH_MSG_INFO(" REGTEST: egamma cluster calo-frame coords. phiCalo = " << tmpphi);
      } else{
          ATH_MSG_INFO(" REGTEST: problems with egamma cluster pointer" );
      }
      ATH_MSG_INFO("REGTEST: Check the original (uncalibrated)");
      if (!orig.isAvailable(*eg->caloCluster()) || !orig(*eg->caloCluster()).isValid()){
          ATH_MSG_INFO("Problem with original cluster link");
      }
      else {
          const xAOD::CaloCluster *origClus = *orig(*eg->caloCluster());
          ATH_MSG_INFO("REGTEST:: Compare new and old clusters");
          ATH_MSG_INFO("REGTEST:: Original Cluster e,eta,phi" << origClus->e() << " " <<  origClus->eta() << " " << origClus->phi());
          ATH_MSG_INFO("REGTEST:: MVA      Cluster e,eta,phi" << eg->caloCluster()->e() << " " <<  eg->caloCluster()->eta() << " " << eg->caloCluster()->phi());
      }
      ATH_MSG_INFO(" REGTEST: trackmatch variables ");
      if(eg->trackParticle()){
          ATH_MSG_INFO(" REGTEST: pt=  " << eg->trackParticle()->pt());
          ATH_MSG_INFO(" REGTEST: charge=  " << eg->trackParticle()->charge());
          ATH_MSG_INFO(" REGTEST: E/p=  " << eg->caloCluster()->et() / eg->trackParticle()->pt() );
          eg->trackCaloMatchValue(val_float,xAOD::EgammaParameters::deltaEta1);
          ATH_MSG_INFO(" REGTEST: Delta eta 1st sampling=  " << val_float);
          eg->trackCaloMatchValue(val_float,xAOD::EgammaParameters::deltaPhi2);
          ATH_MSG_INFO(" REGTEST: Delta phi 2nd sampling=  " << val_float);
      } else{
          ATH_MSG_INFO(" REGTEST: no electron eg->trackParticle() pointer");
      }

      //msg() << MSG::VERBOSE << " REGTEST: cluster variables " << endmsg;
      //clus = eg->caloCluster();
      ATH_MSG_INFO(" REGTEST: EMShower variables ");
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::ethad);
      ATH_MSG_INFO(" REGTEST: ethad    =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e011);
      ATH_MSG_INFO(" REGTEST: e011     =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e132);
      ATH_MSG_INFO(" REGTEST: e132     =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e237);
      ATH_MSG_INFO(" REGTEST: e237     =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e335);
      ATH_MSG_INFO(" REGTEST: e335     =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e2ts1);
      ATH_MSG_INFO(" REGTEST: e2ts1    =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e2tsts1);
      ATH_MSG_INFO(" REGTEST: e2tsts1  =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::ptcone20);
      ATH_MSG_INFO(" REGTEST: ptcone20   =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::ptcone30);
      ATH_MSG_INFO(" REGTEST: ptcone30   =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::ptcone40);
      ATH_MSG_INFO(" REGTEST: ptcone40   =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::etcone20);
      ATH_MSG_INFO(" REGTEST: etcone20   =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::etcone30);
      ATH_MSG_INFO(" REGTEST: etcone30   =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::etcone40);
      ATH_MSG_INFO(" REGTEST: etcone40   =  " << val_float);
      //DEBUG info for Electrons which by definition have a track match

  }
  ATH_MSG_INFO( "REGTEST ==========END of xAOD::ElectronContainer DUMP===========" );

  return StatusCode::SUCCESS;

}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
StatusCode TrigEDMChecker::dumpxAODPhotonContainer() {

  ATH_MSG_DEBUG("In dumpxAODPhotonContainer");

  ATH_MSG_INFO( "REGTEST ==========START of xAOD::PhotonContainer DUMP===========" );

  const xAOD::PhotonContainer* phCont=0;
  StatusCode sc = evtStore()->retrieve(phCont,"HLT_xAOD__PhotonContainer_egamma_Photons");
  if (sc.isFailure()) {
    ATH_MSG_INFO("REGTEST No Photon container HLT_xAOD__PhotonContainer_egamma_Photons");
    return StatusCode::SUCCESS;
  }

  float val_float=-99;
  unsigned int isEMbit=0;
  //DEBUG output for xAOD::PhotonContainer
  ATH_MSG_INFO(" REGTEST: xAOD Reconstruction variables: ");
  //Cluster and ShowerShape info
  static const SG::Accessor<ElementLink<xAOD::CaloClusterContainer> > orig ("originalCaloCluster");
  for (const auto eg : *phCont){
      //REGTEST printout
      if (eg) {
          ATH_MSG_INFO(" REGTEST: egamma energy: " << eg->e() );
          ATH_MSG_INFO(" REGTEST: egamma eta: " << eg->eta() );
          ATH_MSG_INFO(" REGTEST: egamma phi: " << eg->phi() );
          ATH_MSG_INFO(" REGTEST: isEMLoose " << eg->selectionisEM(isEMbit,"isEMLoose"));
          ATH_MSG_INFO(" REGTEST: isEMLoose bit " << std::hex << isEMbit << std::dec);
          ATH_MSG_INFO(" REGTEST: isEMMedium " << eg->selectionisEM(isEMbit,"isEMMedium"));
          ATH_MSG_INFO(" REGTEST: isEMMedium bit " << std::hex << isEMbit << std::dec);
          ATH_MSG_INFO(" REGTEST: isEMTight " << eg->selectionisEM(isEMbit,"isEMTight"));
          ATH_MSG_INFO(" REGTEST: isEMTight bit " << std::hex << isEMbit << std::dec);
      } else{
          ATH_MSG_INFO(" REGTEST: problems with egamma pointer" );
          return StatusCode::SUCCESS;
      }
      ATH_MSG_INFO(" REGTEST: caloCluster variables ");
      if (eg->caloCluster()) {
          ATH_MSG_INFO(" REGTEST: egamma cluster transverse energy: " << eg->caloCluster()->et() );
          ATH_MSG_INFO(" REGTEST: egamma cluster eta: " << eg->caloCluster()->eta() );
          ATH_MSG_INFO(" REGTEST: egamma cluster phi: " << eg->caloCluster()->phi() );
          double tmpeta = -999.;
          double tmpphi = -999.;
          eg->caloCluster()->retrieveMoment(xAOD::CaloCluster::ETACALOFRAME,tmpeta);
          eg->caloCluster()->retrieveMoment(xAOD::CaloCluster::PHICALOFRAME,tmpphi); 
          ATH_MSG_INFO(" REGTEST: egamma cluster calo-frame coords. etaCalo = " << tmpeta); 
          ATH_MSG_INFO(" REGTEST: egamma cluster calo-frame coords. phiCalo = " << tmpphi);
      } else{
          ATH_MSG_INFO(" REGTEST: problems with egamma cluster pointer" );
      }
      ATH_MSG_INFO("REGTEST: Check the original (uncalibrated)");
      if (!orig.isAvailable(*eg->caloCluster()) || !orig(*eg->caloCluster()).isValid()){
          ATH_MSG_INFO("Problem with original cluster link");
      }
      else {
          const xAOD::CaloCluster *origClus = *orig(*eg->caloCluster());
          ATH_MSG_INFO("REGTEST:: Compare new and old clusters");
          ATH_MSG_INFO("REGTEST:: Original Cluster e,eta,phi" << origClus->e() << " " <<  origClus->eta() << " " << origClus->phi());
          ATH_MSG_INFO("REGTEST:: MVA      Cluster e,eta,phi" << eg->caloCluster()->e() << " " <<  eg->caloCluster()->eta() << " " << eg->caloCluster()->phi());
      }
      //msg() << MSG::VERBOSE << " REGTEST: cluster variables " << endmsg;
      //clus = eg->caloCluster();
      ATH_MSG_INFO(" REGTEST: EMShower variables ");
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::ethad);
      ATH_MSG_INFO(" REGTEST: ethad    =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e011);
      ATH_MSG_INFO(" REGTEST: e011     =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e132);
      ATH_MSG_INFO(" REGTEST: e132     =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e237);
      ATH_MSG_INFO(" REGTEST: e237     =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e335);
      ATH_MSG_INFO(" REGTEST: e335     =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e2ts1);
      ATH_MSG_INFO(" REGTEST: e2ts1    =  " << val_float);
      eg->showerShapeValue(val_float,xAOD::EgammaParameters::e2tsts1);
      ATH_MSG_INFO(" REGTEST: e2tsts1  =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::etcone20);
      ATH_MSG_INFO(" REGTEST: etcone20   =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::etcone30);
      ATH_MSG_INFO(" REGTEST: etcone30   =  " << val_float);
      eg->isolationValue(val_float,xAOD::Iso::etcone40);
      ATH_MSG_INFO(" REGTEST: etcone40   =  " << val_float);
      //DEBUG info for Electrons which by definition have a track match

  }
  ATH_MSG_INFO( "REGTEST ==========END of xAOD::PhotonContainer DUMP===========" );

  return StatusCode::SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////

StatusCode TrigEDMChecker::dumpTrigEFBphysContainer() {

  ATH_MSG_DEBUG("in dumpTrigEFBphysContainer()");

  ATH_MSG_INFO("REGTEST ==========START of TrigEFBphysContainer DUMP===========");

  std::string EFBphysTags[]={"HLT_xAOD__TrigBphysContainer_EFBMuMuFex",
                             "HLT_xAOD__TrigBphysContainer_EFBMuMuXFex",
                             "HLT_xAOD__TrigBphysContainer_EFDsPhiPiFex",
                             "HLT_xAOD__TrigBphysContainer_EFMuPairs",
                             "HLT_xAOD__TrigBphysContainer_EFMultiMuFex",
                             "HLT_xAOD__TrigBphysContainer_EFTrackMass"
  };

  int ntag= (int) sizeof(EFBphysTags) / sizeof(EFBphysTags[0]);


  for (int itag=0; itag<ntag; itag++){
    const xAOD::TrigBphysContainer*  trigEFBphys;
    StatusCode sc = evtStore()->retrieve(trigEFBphys, EFBphysTags[itag]);
    if (sc.isFailure()) {
      ATH_MSG_INFO("REGTEST No TrigEFBphysContainer found with tag " << EFBphysTags[itag]);
      continue;
    }

    ATH_MSG_INFO("REGTEST TrigEFBphysContainer found with tag " << EFBphysTags[itag]
                 << " and size " << trigEFBphys->size());

    //  for (int i=0; trigEFBphys != lastTrigEFBphys; ++trigEFBphys, ++i) {

    //mLog << MSG::INFO << "REGTEST Looking at TrigEFBphysContainer " << i << endmsg;

    xAOD::TrigBphysContainer::const_iterator EFBphysItr  = trigEFBphys->begin();
    xAOD::TrigBphysContainer::const_iterator EFBphysItrE = trigEFBphys->end();

    for (int j=0; EFBphysItr != EFBphysItrE; ++EFBphysItr, ++j ) {

      ATH_MSG_INFO("REGTEST Looking at TrigEFBphys " << j);

      ATH_MSG_INFO("REGTEST TrigEFBphys->eta() returns " << (*EFBphysItr)->eta());
      ATH_MSG_INFO("REGTEST TrigEFBphys->phi() returns " << (*EFBphysItr)->phi());
      ATH_MSG_INFO("REGTEST TrigEFBphys->mass() returns " << (*EFBphysItr)->mass());
      ATH_MSG_INFO("REGTEST TrigEFBphys->fitmass() returns " << (*EFBphysItr)->fitmass());
      // ATH_MSG_INFO("REGTEST TrigEFBphys->isValid() returns " << (*EFBphysItr)->isValid());
      ATH_MSG_INFO("REGTEST TrigEFBphys->roiId() returns " << (*EFBphysItr)->roiId());
      ATH_MSG_INFO("REGTEST TrigEFBphys->particleType() returns " << (*EFBphysItr)->particleType());

      if( (*EFBphysItr)->secondaryDecay() != NULL){
        const xAOD::TrigBphys * psecond =(*EFBphysItr)->secondaryDecay();
        ATH_MSG_INFO("REGTEST Secondary decay info: ");
        ATH_MSG_INFO("REGTEST pSecondDecay->eta() returns " << psecond->eta());
        ATH_MSG_INFO("REGTEST pSecondDecay->phi() returns " << psecond->phi());
        ATH_MSG_INFO("REGTEST pSecondDecay->mass() returns " << psecond->mass());
        ATH_MSG_INFO("REGTEST pSecondDecay->fitmass() returns " << psecond->fitmass());
        // ATH_MSG_INFO("REGTEST pSecondDecay->isValid() returns " << (*EFBphysItr)->secondaryDecayLink()->isValid());
        ATH_MSG_INFO("REGTEST pSecondDecay->roiId() returns " << psecond->roiId());
        ATH_MSG_INFO("REGTEST pSecondDecay->particleType() returns " << psecond->particleType());

      } // end if secondary exists



      const std::vector<ElementLink<xAOD::TrackParticleContainer> > trackVector = (*EFBphysItr)->trackParticleLinks();
      if (trackVector.size() != 0) {
        ATH_MSG_INFO(" REGTEST got track vector size: " << trackVector.size());
      } else {
        ATH_MSG_INFO(" REGTEST no track vector!!! " );
      }
      std::vector<ElementLink<xAOD::TrackParticleContainer> >::const_iterator trkIt=trackVector.begin();
      for (int itrk=0 ; trkIt!= trackVector.end(); ++itrk, ++trkIt) {
        if (!(trkIt->isValid())) {
          ATH_MSG_WARNING("TrackParticleContainer::Invalid ElementLink to track ");
          continue;
        }
        //const Trk::Perigee* trackPerigee=(*(*trkIt))->measuredPerigee();
        const Trk::Perigee* trackPerigee=&((*(*trkIt))->perigeeParameters());

        //      msg() << MSG::VERBOSE << "track, iterator, pointer " << itrk << " " << *trkIt << " " << *(*trkIt) << endmsg;
        double phi = trackPerigee->parameters()[Trk::phi];
        double theta = trackPerigee->parameters()[Trk::theta];
        double px = trackPerigee->momentum()[Trk::px];
        double py = trackPerigee->momentum()[Trk::py];
        double pt = sqrt(px*px + py*py);
        double eta = -std::log(tan(theta/2));

        ATH_MSG_INFO("track " << itrk << " pt phi eta " << pt << " " <<
                     phi << " " << eta);
      }

    }
  }
  ATH_MSG_INFO("REGTEST ==========END of TrigEFBphysContainer DUMP===========");
  ATH_MSG_DEBUG("dumpTrigEFBphysContainer() succeeded");

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
StatusCode TrigEDMChecker::dumpTrigL2BphysContainer() {

  ATH_MSG_DEBUG("in dumpTrigL2BphysContainer()");
  ATH_MSG_INFO("REGTEST ==========START of TrigL2BphysContainer DUMP===========");

  std::string L2BphysTags[]={"HLT_xAOD__TrigBphysContainer_L2BMuMuFex",
                             "HLT_xAOD__TrigBphysContainer_L2BMuMuXFex",
                             "HLT_xAOD__TrigBphysContainer_L2DsPhiPiFexDs",
                             "HLT_xAOD__TrigBphysContainer_L2DsPhiPiFexPhi",
                             "HLT_xAOD__TrigBphysContainer_L2JpsieeFex",
                             "HLT_xAOD__TrigBphysContainer_L2MultiMuFex",
                             "HLT_xAOD__TrigBphysContainer_L2TrackMass",
  };
  const int ntag = (int) sizeof(L2BphysTags) / sizeof(L2BphysTags[0]);


  for (int itag=0; itag<ntag; itag++){
    const xAOD::TrigBphysContainer*  trigL2Bphys;
    StatusCode sc = evtStore()->retrieve(trigL2Bphys, L2BphysTags[itag]);
    if (sc.isFailure()) {
      ATH_MSG_INFO("REGTEST No TrigL2BphysContainer found with tag " << L2BphysTags[itag]);
      continue;
    }

    ATH_MSG_INFO("REGTEST TrigL2BphysContainer found with tag " << L2BphysTags[itag]
                 << " and size " << trigL2Bphys->size());

    //  for (int i=0; trigL2Bphys != lastTrigL2Bphys; ++trigL2Bphys, ++i) {

    // mLog << MSG::INFO << "REGTEST Looking at TrigL2BphysContainer " << i << endmsg;

    xAOD::TrigBphysContainer::const_iterator L2BphysItr  = trigL2Bphys->begin();
    xAOD::TrigBphysContainer::const_iterator L2BphysItrE = trigL2Bphys->end();

    for (int j=0; L2BphysItr != L2BphysItrE; ++L2BphysItr, ++j ) {

      ATH_MSG_INFO("REGTEST Looking at TrigL2Bphys " << j);

      ATH_MSG_INFO("REGTEST TrigL2Bphys->eta() returns " << (*L2BphysItr)->eta());
      ATH_MSG_INFO("REGTEST TrigL2Bphys->phi() returns " << (*L2BphysItr)->phi());
      ATH_MSG_INFO("REGTEST TrigL2Bphys->mass() returns " << (*L2BphysItr)->mass());
      ATH_MSG_INFO("REGTEST TrigL2Bphys->fitmass() returns " << (*L2BphysItr)->fitmass());
      // ATH_MSG_INFO("REGTEST TrigL2Bphys->isValid() returns " << (*L2BphysItr)->isValid());
      ATH_MSG_INFO("REGTEST TrigL2Bphys->roiId() returns " << (*L2BphysItr)->roiId());
      ATH_MSG_INFO("REGTEST TrigL2Bphys->particleType() returns " << (*L2BphysItr)->particleType());

      if( (*L2BphysItr)->secondaryDecay() != NULL){
        const xAOD::TrigBphys * psecond =(*L2BphysItr)->secondaryDecay();
        ATH_MSG_INFO("REGTEST Secondary decay info: ");
        ATH_MSG_INFO("REGTEST pSecondDecay->eta() returns " << psecond->eta());
        ATH_MSG_INFO("REGTEST pSecondDecay->phi() returns " << psecond->phi());
        ATH_MSG_INFO("REGTEST pSecondDecay->mass() returns " << psecond->mass());
        ATH_MSG_INFO("REGTEST pSecondDecay->fitmass() returns " << psecond->fitmass());
        // ATH_MSG_INFO("REGTEST pSecondDecay->isValid() returns " << (*L2BphysItr)->secondaryDecayLink()->isValid());
        ATH_MSG_INFO("REGTEST pSecondDecay->roiId() returns " << psecond->roiId());
        ATH_MSG_INFO("REGTEST pSecondDecay->particleType() returns " << psecond->particleType());
      } // end if secondary exists

      const std::vector<ElementLink<xAOD::TrackParticleContainer> > trackVector = (*L2BphysItr)->trackParticleLinks();
      if (trackVector.size() != 0) {
        ATH_MSG_INFO(" REGTEST got track vector size: " << trackVector.size());
      } else {
        ATH_MSG_INFO(" REGTEST no track vector!!! " );
      }
      std::vector<ElementLink<xAOD::TrackParticleContainer> >::const_iterator trkIt=trackVector.begin();
      for (int itrk=0 ; trkIt!= trackVector.end(); ++itrk, ++trkIt) {
        if (!(trkIt->isValid())) {
          ATH_MSG_WARNING("TrackParticleContainer::Invalid ElementLink to track ");
          continue;
        }
        //const Trk::Perigee* trackPerigee=(*(*trkIt))->measuredPerigee();
        const Trk::Perigee* trackPerigee=&((*(*trkIt))->perigeeParameters());

        //      msg() << MSG::VERBOSE << "track, iterator, pointer " << itrk << " " << *trkIt << " " << *(*trkIt) << endmsg;
        double phi = trackPerigee->parameters()[Trk::phi];
        double theta = trackPerigee->parameters()[Trk::theta];
        double px = trackPerigee->momentum()[Trk::px];
        double py = trackPerigee->momentum()[Trk::py];
        double pt = sqrt(px*px + py*py);
        double eta = -std::log(tan(theta/2));
        
        ATH_MSG_INFO("track " << itrk << " pt phi eta " << pt << " " <<
                     phi << " " << eta);
      }
    }
  }

  ATH_MSG_INFO("REGTEST ==========END of TrigL2BphysContainer DUMP===========");
  ATH_MSG_DEBUG("dumpTrigL2BphysContainer() succeeded");

  return StatusCode::SUCCESS;
}



//////////////////////////////////////////////////////////////////////////////////

StatusCode TrigEDMChecker::dumpxAODJetContainer() {
    
    ATH_MSG_DEBUG("in dumpxAODJetContainer()");
    
    ATH_MSG_INFO("REGTEST ==========START of xAOD::JetContainer DUMP===========");
    
    std::string containerName[30] = {"HLT_xAOD__JetContainer_a4tcemjesFS",
                                     "HLT_xAOD__JetContainer_a4tcemsubFS",
                                     "HLT_xAOD__JetContainer_a4tcemsubjesFS",
                                     "HLT_xAOD__JetContainer_a4tcemnojcalibFS",
                                     "HLT_xAOD__JetContainer_a4tcemjesPS",
                                     "HLT_xAOD__JetContainer_a4tcemnojcalibPS",
                                     "HLT_xAOD__JetContainer_a4tclcwjesFS",
                                     "HLT_xAOD__JetContainer_a4tclcwsubFS",
                                     "HLT_xAOD__JetContainer_a4tclcwsubjesFS",
                                     "HLT_xAOD__JetContainer_a4tclcwnojcalibFS",
                                     "HLT_xAOD__JetContainer_a4tclcwjesPS",
                                     "HLT_xAOD__JetContainer_a4tclcwnojcalibPS",
                                     "HLT_xAOD__JetContainer_a4TTemnojcalibFS",
                                     "HLT_xAOD__JetContainer_a4TThadnojcalibFS",
                                     "HLT_xAOD__JetContainer_a10tcemjesFS",
                                     "HLT_xAOD__JetContainer_a10tcemsubFS",
                                     "HLT_xAOD__JetContainer_a10tcemsubjesFS",
                                     "HLT_xAOD__JetContainer_a10tcemnojcalibFS",
                                     "HLT_xAOD__JetContainer_a10tcemjesPS",
                                     "HLT_xAOD__JetContainer_a10tcemnojcalibPS",
                                     "HLT_xAOD__JetContainer_a10tclcwjesFS",
                                     "HLT_xAOD__JetContainer_a10tclcwsubFS",
                                     "HLT_xAOD__JetContainer_a10tclcwsubjesFS",
                                     "HLT_xAOD__JetContainer_a10tclcwnojcalibFS",
                                     "HLT_xAOD__JetContainer_a10tclcwjesPS",
                                     "HLT_xAOD__JetContainer_a10tclcwnojcalibPS",
                                     "HLT_xAOD__JetContainer_a10TTemnojcalibFS",
                                     "HLT_xAOD__JetContainer_a10TThadnojcalibFS",
                                     "HLT_xAOD__JetContainer_a10r_tcemsubjesFS",
                                     "HLT_xAOD__JetContainer_TrigHLTJetDSSelectorCollection"};
    float containerSizeParameter[30] = {0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.4};
    int containerInputType[30] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 2, 2,
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1};
    int containerSignalState[30] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0};
    bool onefilled = false;
    
    for (int icont=0; icont<30; ++icont) {
        //retrieve jet collection
        const xAOD::JetContainer* jetCont = 0;
        StatusCode sc = evtStore()->retrieve(jetCont, containerName[icont]) ;
        
        if (sc.isFailure()) {
            ATH_MSG_WARNING("REGTEST Cannot retrieve jet container");
            continue;
        }
        
        int jetContsize = jetCont->size();
        ATH_MSG_INFO("REGTEST Got jet container " << containerName[icont] << ", size: " << jetContsize);
        if (jetContsize != 0) {
            onefilled = true;
            int i = 0;
            for(const auto thisjet : *jetCont) {
                ++i;
                ATH_MSG_INFO( "REGTEST Looking at jet " << i);
                if (thisjet) {
                    //checks jet variables
                    ATH_MSG_DEBUG("REGTEST    Checking jet variables");
                    ATH_MSG_INFO( "REGTEST    pt: " << thisjet->pt() );
                    ATH_MSG_INFO( "REGTEST    eta: " << thisjet->eta() );
                    ATH_MSG_INFO( "REGTEST    phi: " << thisjet->phi() );
                    ATH_MSG_INFO( "REGTEST    m: " << thisjet->m() );
                    ATH_MSG_INFO( "REGTEST    e: " << thisjet->e() );
                    ATH_MSG_INFO( "REGTEST    rapidity: " << thisjet->rapidity() );
                    ATH_MSG_INFO( "REGTEST    px: " << thisjet->px() );
                    ATH_MSG_INFO( "REGTEST    py: " << thisjet->py() );
                    ATH_MSG_INFO( "REGTEST    pz: " << thisjet->pz() );
                    ATH_MSG_INFO( "REGTEST    type: " << thisjet->type() );
                    ATH_MSG_INFO( "REGTEST    algorithm (kt: 0, cam: 1, antikt: 2, ...): " << thisjet->getAlgorithmType() << "; should be 2");
                    if(thisjet->getAlgorithmType() != 2) ATH_MSG_WARNING("Jet algorithm different from container");
                    ATH_MSG_INFO( "REGTEST    size parameter: " << thisjet->getSizeParameter() << "; should be " << containerSizeParameter[icont]);
                    if(thisjet->getSizeParameter() != containerSizeParameter[icont]) ATH_MSG_WARNING("Jet size different from container");
                    ATH_MSG_INFO( "REGTEST    input (LCTopo: 0, EMTopo: 1, TopoTower: 2, ...): " << thisjet->getInputType() << "; should be " << containerInputType[icont]);
                    if(thisjet->getInputType() != containerInputType[icont]) ATH_MSG_WARNING("Jet input different from container");
                    ATH_MSG_INFO( "REGTEST    constituents signal state (uncalibrated: 0, calibrated: 1): " << thisjet->getConstituentsSignalState() << "; should be " << containerSignalState[icont]);
                    if(thisjet->getConstituentsSignalState() != containerSignalState[icont]) ATH_MSG_WARNING("Jet constituents' signal state different from container");
                    ATH_MSG_INFO( "REGTEST    number of constituents: " << thisjet->numConstituents() );
                    
                    
                    //checks the constituents
                    ATH_MSG_DEBUG("REGTEST    Checking jet constituents");
                    
                    xAOD::JetConstituentVector constitCont = thisjet->getConstituents();
                    unsigned int constitContsize = constitCont.size();
                    ATH_MSG_INFO("REGTEST    Got constituent vector, size: " << constitContsize << "; should be " << thisjet->numConstituents());
                    if(constitContsize != thisjet->numConstituents()) ATH_MSG_WARNING("Constituents container size different from number of constituents");
                    
                    if (constitContsize != 0) {
//                        int j = 0;
                        //                        for (const auto thisconstit : constitCont) {
                        //                            ++j;
                        //                            ATH_MSG_INFO( "REGTEST    Looking at constituent " << j);
                        //                            if(thisconstit){
                        //                                ATH_MSG_INFO( "REGTEST        constituent pt: " << thisconstit->pt() );
                        //                                ATH_MSG_INFO( "REGTEST        constituent eta: " << thisconstit->eta() );
                        //                                ATH_MSG_INFO( "REGTEST        constituent phi: " << thisconstit->phi() );
                        //                                ATH_MSG_INFO( "REGTEST        constituent m: " << thisconstit->m() );
                        //                                ATH_MSG_INFO( "REGTEST        constituent e: " << thisconstit->e() );
                        //                                ATH_MSG_INFO( "REGTEST        constituent type (CaloCluster: 1, Jet: 2, ...): " << thisconstit->type() );
                        //                            }
                        //                            else{
                        //                                ATH_MSG_WARNING("REGTEST Problem with constituent pointer");
                        //                                return StatusCode::SUCCESS;
                        //                            }
                        //                        }
                        //                        ATH_MSG_INFO("REGTEST        size of constituent vector == number of displayed constituents: "<< (constitContsize == j) );
                        //                        if (constitContsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                    }
                    //                    else ATH_MSG_WARNING("REGTEST    This jet has 0 constituents!");
                    
                    //checks int attributes
                    ATH_MSG_DEBUG("REGTEST    Checking int attributes");
                    int valueint;
                    if( thisjet->getAttribute(xAOD::JetAttribute::GhostMuonSegmentCount, valueint))    ATH_MSG_INFO("REGTEST    GhostMuonSegmentCount: " << valueint);
                    if( thisjet->getAttribute(xAOD::JetAttribute::GhostTrackCount, valueint))          ATH_MSG_INFO("REGTEST    GhostTrackCount: " << valueint);
                    if( thisjet->getAttribute(xAOD::JetAttribute::GhostTruthParticleCount, valueint))  ATH_MSG_INFO("REGTEST    GhostTruthParticleCount: " << valueint);
                    if( thisjet->getAttribute(xAOD::JetAttribute::FracSamplingMaxIndex, valueint))   ATH_MSG_INFO( "REGTEST    FracSamplingMaxIndex: " << valueint);
                    
                    
                    //checks float attributes
                    ATH_MSG_DEBUG("REGTEST    Checking float attributes");
                    float value;
                    if( thisjet->getAttribute(xAOD::JetAttribute::ActiveArea, value))        ATH_MSG_INFO( "REGTEST    ActiveArea: " << value );
                    // Four-vector type                   if( thisjet->getAttribute(xAOD::JetAttribute::ActiveArea4vec, value))        ATH_MSG_INFO( "REGTEST    ActiveArea4vec: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::AverageLArQF, value))      ATH_MSG_INFO( "REGTEST    AverageLArQF: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::BchCorrCell, value))       ATH_MSG_INFO( "REGTEST    BchCorrCell: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::BchCorrDotx, value))       ATH_MSG_INFO( "REGTEST    BchCorrDotx: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::BchCorrJet, value))        ATH_MSG_INFO( "REGTEST    BchCorrJet: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::BchCorrJetForCell, value)) ATH_MSG_INFO( "REGTEST    BchCorrJetForCell: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::CentroidR, value))         ATH_MSG_INFO( "REGTEST    CentroidR: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::HECQuality, value))        ATH_MSG_INFO( "REGTEST    HECQuality: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::IsoKR20Par, value))        ATH_MSG_INFO( "REGTEST    IsoKR20Par: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::IsoKR20Perp, value))       ATH_MSG_INFO( "REGTEST    IsoKR20Perp: " << value );
                    // ElementLink<DataVector<xAOD::Vertex> > type                    if( thisjet->getAttribute(xAOD::JetAttribute::HighestJVFVtx, value))       ATH_MSG_INFO( "REGTEST    HighestJVFVtx: " << value );
                    // ??? type                    if( thisjet->getAttribute(xAOD::JetAttribute::JetLabel, value))       ATH_MSG_INFO( "REGTEST    JetLabel: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::KtDR, value))              ATH_MSG_INFO( "REGTEST    KtDR: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::LArBadHVEnergy, value))    ATH_MSG_INFO( "REGTEST    LArBadHVEnergy: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::LArBadHVRatio, value))     ATH_MSG_INFO( "REGTEST    LArBadHVRatio: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::LArQuality, value))        ATH_MSG_INFO( "REGTEST    LArQuality: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::NegativeE, value))         ATH_MSG_INFO( "REGTEST    NegativeE: " << value );
                    // no tools available yet                    if( thisjet->getAttribute(xAOD::JetAttribute::NumTowers, value))         ATH_MSG_INFO( "REGTEST    NumTowers: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::SamplingMax, value))       ATH_MSG_INFO( "REGTEST    SamplingMax: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Timing, value))            ATH_MSG_INFO( "REGTEST    Timing: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::VoronoiArea, value))       ATH_MSG_INFO( "REGTEST    VoronoiArea: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::VoronoiAreaE, value))      ATH_MSG_INFO( "REGTEST    VoronoiAreaE: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::VoronoiAreaPx, value))     ATH_MSG_INFO( "REGTEST    VoronoiAreaPx: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::VoronoiAreaPy, value))     ATH_MSG_INFO( "REGTEST    VoronoiAreaPy: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::VoronoiAreaPz, value))     ATH_MSG_INFO( "REGTEST    VoronoiAreaPz: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Width, value))             ATH_MSG_INFO( "REGTEST    WIDTH: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::FracSamplingMax, value))   ATH_MSG_INFO( "REGTEST    FracSamplingMax: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::EMFrac, value))            ATH_MSG_INFO( "REGTEST    EMFrac: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::HECFrac, value))           ATH_MSG_INFO( "REGTEST    HECFrac: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::isBadLoose, value))        ATH_MSG_INFO( "REGTEST    isBadLoose: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::isBadMedium, value))       ATH_MSG_INFO( "REGTEST    isBadMedium: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::isBadTight, value))        ATH_MSG_INFO( "REGTEST    isBadTight: " << value );
                    // unknown attribute                   if( thisjet->getAttribute(xAOD::JetAttribute::isUgly, value))            ATH_MSG_INFO( "REGTEST    isUgly: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::N90Constituents, value))   ATH_MSG_INFO( "REGTEST    N90Constituents: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::N90Cells, value))          ATH_MSG_INFO( "REGTEST    N90Cells: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::OotFracClusters10, value)) ATH_MSG_INFO( "REGTEST    OotFracClusters10: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::OotFracClusters5, value))  ATH_MSG_INFO( "REGTEST    OotFracClusters5: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::OotFracCells5, value))     ATH_MSG_INFO( "REGTEST    OotFracCells5: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::OotFracCells10, value))    ATH_MSG_INFO( "REGTEST    OotFracCells10: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::PtTruth, value))    ATH_MSG_INFO( "REGTEST    PtTruth: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Tau1, value))              ATH_MSG_INFO( "REGTEST    Tau1: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Tau2, value))              ATH_MSG_INFO( "REGTEST    Tau2: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Tau3, value))              ATH_MSG_INFO( "REGTEST    Tau3: " << value );
                    // unknown attribute                  if( thisjet->getAttribute(xAOD::JetAttribute::Split12, value))           ATH_MSG_INFO( "REGTEST    Split12: " << value );
                    // unknown attribute                   if( thisjet->getAttribute(xAOD::JetAttribute::Split23, value))           ATH_MSG_INFO( "REGTEST    Split23: " << value );
                    // unknown attribute                   if( thisjet->getAttribute(xAOD::JetAttribute::Split34, value))           ATH_MSG_INFO( "REGTEST    Split34: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Dip12, value))             ATH_MSG_INFO( "REGTEST    Dip12: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Dip13, value))             ATH_MSG_INFO( "REGTEST    Dip13: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Dip23, value))             ATH_MSG_INFO( "REGTEST    Dip23: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::DipExcl12, value))         ATH_MSG_INFO( "REGTEST    DipExcl12: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::ThrustMin, value))         ATH_MSG_INFO( "REGTEST    ThrustMin: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::ThrustMaj, value))         ATH_MSG_INFO( "REGTEST    ThrustMaj: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::FoxWolfram0, value))       ATH_MSG_INFO( "REGTEST    FoxWolfram0: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::FoxWolfram1, value))       ATH_MSG_INFO( "REGTEST    FoxWolfram1: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::FoxWolfram2, value))       ATH_MSG_INFO( "REGTEST    FoxWolfram2: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::FoxWolfram3, value))       ATH_MSG_INFO( "REGTEST    FoxWolfram3: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::FoxWolfram4, value))       ATH_MSG_INFO( "REGTEST    FoxWolfram4: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Sphericity, value))        ATH_MSG_INFO( "REGTEST    Sphericity: " << value );
                    if( thisjet->getAttribute(xAOD::JetAttribute::Aplanarity, value))        ATH_MSG_INFO( "REGTEST    Aplanarity: " << value );
                    
                    //checks vector<int> attributes
                    ATH_MSG_DEBUG("REGTEST    Checking vector<int> attributes");
                    std::vector<int> vecvalueint;
                    if (thisjet->getAttribute(xAOD::JetAttribute::NumTrkPt1000, vecvalueint)) {
                        int vecsize = vecvalueint.size();
                        ATH_MSG_INFO("REGTEST    Got NumTrkPt1000 vector, size: " << vecsize);
                        if (vecsize != 0) {
                            int j = 0;
                            for(const auto & thisvalue : vecvalueint){
                                ++j;
                                ATH_MSG_INFO("REGTEST        NumTrkPt1000 #" << j << ": " << thisvalue);
                            }
                            ATH_MSG_INFO("REGTEST        size of attribute vector == number of displayed attributes: " << (vecsize == j) );
                            if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                        }
                    }
                    if (thisjet->getAttribute(xAOD::JetAttribute::NumTrkPt500, vecvalueint)) {
                        int vecsize = vecvalueint.size();
                        ATH_MSG_INFO("REGTEST    Got Got NumTrkPt500 vector, size: " << vecsize);
                        if (vecsize != 0) {
                            int j = 0;
                            for(const auto & thisvalue : vecvalueint){
                                ++j;
                                ATH_MSG_INFO("REGTEST        NumTrkPt500 #" << j << ": " << thisvalue);
                            }
                            ATH_MSG_INFO("REGTEST        size of attribute vector == number of displayed attributes: " << (vecsize == j) );
                            if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                        }
                    }
                    
                    //checks vector<float> attributes
                    ATH_MSG_DEBUG("REGTEST    Checking vector<float> attributes");
                    std::vector<float> vecvalue;
                    if (thisjet->getAttribute(xAOD::JetAttribute::JVF, vecvalue)) {
                        int vecsize = vecvalue.size();
                        ATH_MSG_INFO("REGTEST    Got JVF vector, size: " << vecsize);
                        if (vecsize != 0) {
                            int j = 0;
                            for(const auto & thisvalue : vecvalue){
                                ++j;
                                ATH_MSG_INFO("REGTEST        JVF #" << j << ": " << thisvalue);
                            }
                            ATH_MSG_INFO("REGTEST        size of attribute vector == number of displayed attributes: " << (vecsize == j) );
                            if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                        }
                    }
                    if (thisjet->getAttribute(xAOD::JetAttribute::SumPtTrkPt1000, vecvalue)) {
                        int vecsize = vecvalue.size();
                        ATH_MSG_INFO("REGTEST    Got SumPtTrkPt1000 vector, size: " << vecsize);
                        if (vecsize != 0) {
                            int j = 0;
                            for(const auto & thisvalue : vecvalue){
                                ++j;
                                ATH_MSG_INFO("REGTEST        SumPtTrkPt1000 #" << j << ": " << thisvalue);
                            }
                            ATH_MSG_INFO("REGTEST        size of attribute vector == number of displayed attributes: " << (vecsize == j) );
                            if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                        }
                    }
                    if (thisjet->getAttribute(xAOD::JetAttribute::SumPtTrkPt500, vecvalue)) {
                        int vecsize = vecvalue.size();
                        ATH_MSG_INFO("REGTEST    Got SumPtTrkPt500 vector, size: " << vecsize);
                        if (vecsize != 0) {
                            int j = 0;
                            for(const auto & thisvalue : vecvalue){
                                ++j;
                                ATH_MSG_INFO("REGTEST        SumPtTrkPt500 #" << j << ": " << thisvalue);
                            }
                            ATH_MSG_INFO("REGTEST        size of attribute vector == number of displayed attributes: " << (vecsize == j) );
                            if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                        }
                    }
                    if (thisjet->getAttribute(xAOD::JetAttribute::TrackWidthPt1000, vecvalue)) {
                        int vecsize = vecvalue.size();
                        ATH_MSG_INFO("REGTEST    Got TrackWidthPt1000 vector, size: " << vecsize);
                        if (vecsize != 0) {
                            int j = 0;
                            for(const auto & thisvalue : vecvalue){
                                ++j;
                                ATH_MSG_INFO("REGTEST        TrackWidthPt1000 #" << j << ": " << thisvalue);
                            }
                            ATH_MSG_INFO("REGTEST        size of attribute vector == number of displayed attributes: " << (vecsize == j) );
                            if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                        }
                    }
                    if (thisjet->getAttribute(xAOD::JetAttribute::TrackWidthPt500, vecvalue)) {
                        int vecsize = vecvalue.size();
                        ATH_MSG_INFO("REGTEST    Got TrackWidthPt500 vector, size: " << vecsize);
                        if (vecsize != 0) {
                            int j = 0;
                            for(const auto & thisvalue : vecvalue){
                                ++j;
                                ATH_MSG_INFO("REGTEST        TrackWidthPt500 #" << j << ": " << thisvalue);
                            }
                            ATH_MSG_INFO("REGTEST        size of attribute vector == number of displayed attributes: " << (vecsize == j) );
                            if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                        }
                    }
                    if (thisjet->getAttribute(xAOD::JetAttribute::EnergyPerSampling, vecvalue)) {
                        int vecsize = vecvalue.size();
                        ATH_MSG_INFO("REGTEST    Got EnergyPerSampling vector, size: " << vecsize);
                        if (vecsize != 0) {
                            int j = 0;
                            for(const auto & thisvalue : vecvalue){
                                ++j;
                                ATH_MSG_INFO("REGTEST        EnergyPerSampling #" << j << ": " << thisvalue);
                            }
                            ATH_MSG_INFO("REGTEST        size of attribute vector == number of displayed attributes: " << (vecsize == j) );
                            if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this attribute");
                        }
                    }
                    
                    
                    //checks associated objects
                    //                    ATH_MSG_DEBUG("REGTEST    Checking associated objects");
                    //                                std::vector<const xAOD::TrackParticle*> track;
                    //                                if( thisjet->getAssociatedObjects(xAOD::JetAttribute::GhostTrack, track) ){
                    //                                    int vecsize = track.size();
                    //                                    ATH_MSG_INFO("REGTEST    Got GhostTrack vector, size: " << vecsize);
                    //                                    if (vecsize != 0) {
                    //                                        int j = 0;
                    //                                        for(const auto & thistrack : track){
                    //                                            ++j;
                    //                                            //checks only one associated variable, just making sure getting the object worked
                    //                                            if (thistrack) ATH_MSG_INFO("REGTEST        z0 for GhostTrack #" << j << ": " << thistrack->z0());
                    //                                            else{
                    //                                                ATH_MSG_WARNING("REGTEST Problem with attribute pointer");
                    //                                                return StatusCode::SUCCESS;
                    //                                            }
                    //                                        }
                    //                                        ATH_MSG_INFO("REGTEST        size of associated object vector == number of displayed attributes: " << (vecsize == j) );
                    //                                        if (vecsize != j) ATH_MSG_WARNING("REGTEST Problem with displaying this associated object");
                    //                                    }
                    //                                }
                }
                else{
                    ATH_MSG_WARNING("REGTEST Problem with jet pointer");
                    return StatusCode::SUCCESS;
                }
            }
            
            if (jetContsize == i) ATH_MSG_INFO("REGTEST size of jet container == number of displayed jets: " << (jetContsize == i) );
            else ATH_MSG_WARNING("REGTEST Problem with displaying jets");
        }
    }
    
    if (!onefilled) ATH_MSG_DEBUG("There was no filled jet containers");
    
    ATH_MSG_INFO("REGTEST ==========END of xAOD::JetContainer DUMP===========");
    
    ATH_MSG_DEBUG("leaving dumpxAODJetContainer()");
    
    return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////

StatusCode TrigEDMChecker::dumpxAODTrigEMCluster() {

  ATH_MSG_DEBUG("in dumpxAODTrigEMCluster()");

  ATH_MSG_INFO("REGTEST ==========START of TrigEMCluster DUMP===========");

  SG::ConstIterator< xAOD::TrigEMCluster > EMCluster;
  SG::ConstIterator< xAOD::TrigEMCluster > lastEMCluster;

  StatusCode sc = evtStore()->retrieve(EMCluster,lastEMCluster);
  if (sc.isFailure()) {
    ATH_MSG_INFO("REGTEST No xAOD::TrigEMCluster found");
    return  StatusCode::SUCCESS;
  }
  ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster retrieved");


  for (int i=0; EMCluster != lastEMCluster; ++EMCluster, ++i) {

    const xAOD::TrigEMCluster* thisEMCluster = &(*EMCluster);

    ATH_MSG_INFO("REGTEST Looking at xAOD::TrigEMCluster " << i);
    
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->energy() returns " << thisEMCluster->energy());
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->e() returns " << thisEMCluster->energy());
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->phi() returns " << thisEMCluster->phi());

    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->eta() returns " << thisEMCluster->eta());
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->e237() returns " << thisEMCluster->e237());
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->e277() returns " << thisEMCluster->e277());
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->fracs1() returns " << thisEMCluster->fracs1());
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->weta2() returns " << thisEMCluster->weta2());
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->ehad1() returns " << thisEMCluster->ehad1());
    ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->Eta1() returns " << thisEMCluster->eta1());
  }

  ATH_MSG_INFO("REGTEST ==========END of xAOD::TrigEMCluster DUMP===========");
  ATH_MSG_DEBUG("dumpxAODTrigEMCluster() succeeded");

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////

StatusCode TrigEDMChecker::dumpxAODTrigEMClusterContainer() {

  ATH_MSG_DEBUG("in dumpxAODTrigEMClusterContainer()");

  ATH_MSG_INFO("REGTEST ==========START of xAODTrigEMClusterContainer DUMP===========");

  SG::ConstIterator< xAOD::TrigEMClusterContainer > EMCluster;
  SG::ConstIterator< xAOD::TrigEMClusterContainer > lastEMCluster;

  StatusCode sc = evtStore()->retrieve(EMCluster,lastEMCluster);
  if (sc.isFailure()) {
    ATH_MSG_INFO("REGTEST No xAOD::TrigEMClusterContainer found");
    return  StatusCode::SUCCESS;
  }
  ATH_MSG_INFO("REGTEST xAOD::TrigEMClusterContainer retrieved");


  for (int i=0; EMCluster != lastEMCluster; ++EMCluster, ++i) {

    ATH_MSG_INFO("REGTEST Looking at xAOD::TrigEMClusterContainer " << i);

    xAOD::TrigEMClusterContainer::const_iterator EMClusterItr  = EMCluster->begin();
    xAOD::TrigEMClusterContainer::const_iterator EMClusterItrE = EMCluster->end();

    for (int j=0; EMClusterItr != EMClusterItrE; ++EMClusterItr, ++j ) {

      ATH_MSG_INFO("REGTEST Looking at xAOD::TrigEMCluster " << j);
      ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->energy() returns " << (*EMClusterItr)->energy());
      ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->et() returns " << (*EMClusterItr)->et());
      ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->eta() returns " << (*EMClusterItr)->eta());
      ATH_MSG_INFO("REGTEST xAOD::TrigEMCluster->phi() returns " << (*EMClusterItr)->phi());
      //msg() <<MSG::INFO << "REGTEST xAOD::TrigEMCluster->print() gives" << endmsg;
      //int level = msg().level();
      // little trick to print out stuff
      //msg().setLevel(MSG::DEBUG);
      //(*EMClusterItr)->print(msg());
      //msg().setLevel(level);
    }
  }

  ATH_MSG_INFO("REGTEST ==========END of TrigEMClusterContainer DUMP===========");
  ATH_MSG_DEBUG("dumpTrigEMClusterContainer() succeeded");

  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////
StatusCode TrigEDMChecker::dumpxAODTauJetContainer() {

  ATH_MSG_DEBUG("In dumpxAODTauJetContainer");
  ATH_MSG_INFO( "REGTEST ==========START of xAOD::TauJetContainer DUMP===========" );
  const xAOD::TauJetContainer * TauJetcont = 0;
  StatusCode sc = evtStore() -> retrieve (TauJetcont, "HLT_xAOD__TauJetContainer_TrigTauRecMerged");

  if (sc.isFailure()) {

    ATH_MSG_INFO("REGTEST No Tau container HLT_xAOD__TauJetContainer_TrigTauRecMerged");


    return StatusCode::SUCCESS;
  }


  for(xAOD::TauJetContainer::const_iterator tauIt = TauJetcont->begin(); tauIt != TauJetcont->end();++tauIt){

    ATH_MSG_INFO( "REGTEST (*tauIt)->eta() returns  " << (*tauIt)->eta() );
    ATH_MSG_INFO( "REGTEST (*tauIt)->phi() returns  " << (*tauIt)->phi() );
    ATH_MSG_INFO( "REGTEST (*tauIt)->pt() returns   " << (*tauIt)->pt() );

    // for numTracks()
    int EFnTracks = -1;
    #ifndef XAODTAU_VERSIONS_TAUJET_V3_H
    EFnTracks = (*tauIt)->nTracks();
    #else
    (*tauIt)->detail(xAOD::TauJetParameters::nChargedTracks, EFnTracks);
    #endif

    ATH_MSG_INFO( "REGTEST (*tauIt)->nTracks() returns " << EFnTracks );

    // for nTracksIsolation()
    int EFWidenTrack = -1;
    #ifndef XAODTAU_VERSIONS_TAUJET_V3_H
    EFWidenTrack = (*tauIt)->nWideTracks();
    #else
    (*tauIt)->detail(xAOD::TauJetParameters::nIsolatedTracks, EFWidenTrack);
    #endif

    ATH_MSG_INFO( "REGTEST (*tauIt)->nWideTracks() returns " << EFWidenTrack );

    //bool test = false;
    float trkAvgDist=0;
    float etOvPtLead=0;
    float emRadius=0;
    float hadRadius=0;
    float IsoFrac=0;
    float centFrac=0;
    float ipSigLeadTrk=0;
    float trFlightPathSig=0;
    float dRmax=0;
    float massTrkSys=0;
    float PSSFraction=0;
    float EMPOverTrkSysP=0;
    float ChPiEMEOverCaloEME=0;
    float EtEm=0;
    float EtHad=0;

    if ( (*tauIt)->detail(xAOD::TauJetParameters::trkAvgDist,trkAvgDist))
      ATH_MSG_INFO( "REGTEST TauDetails->trkAvgDist() returns " << trkAvgDist);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::etOverPtLeadTrk,etOvPtLead))
      ATH_MSG_INFO( "REGTEST TauDetails->etOverPtLeadTrk() returns " << etOvPtLead);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::EMRadius,emRadius))
      ATH_MSG_INFO( "REGTEST TauDetails->EMRadius() returns " << emRadius);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::hadRadius,hadRadius))
      ATH_MSG_INFO( "REGTEST TauDetails->hadRadius() returns " << hadRadius);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::isolFrac,IsoFrac))
      ATH_MSG_INFO( "REGTEST TauDetails->isolFrac() returns " << IsoFrac);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::centFrac,centFrac))
      ATH_MSG_INFO( "REGTEST TauDetails->centFrac() returns " << centFrac);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::ipSigLeadTrk,ipSigLeadTrk))
      ATH_MSG_INFO( "REGTEST TauDetails->ipSigLeadTrk() returns " << ipSigLeadTrk);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::trFlightPathSig,trFlightPathSig))
      ATH_MSG_INFO( "REGTEST TauDetails->trFlightPathSig() returns " << trFlightPathSig);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::dRmax,dRmax))
      ATH_MSG_INFO( "REGTEST TauDetails->dRmax() returns " << dRmax);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::massTrkSys,massTrkSys)){
      massTrkSys /=1000;
      ATH_MSG_INFO( "REGTEST TauDetails->massTrkSys() returns " << massTrkSys);}

    if ( (*tauIt)->detail(xAOD::TauJetParameters::PSSFraction,PSSFraction))
      ATH_MSG_INFO( "REGTEST TauDetails->PSSFraction() returns " << PSSFraction);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::EMPOverTrkSysP,EMPOverTrkSysP))
      ATH_MSG_INFO( "REGTEST TauDetails->EMPOverTrkSysP() returns " << EMPOverTrkSysP);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::ChPiEMEOverCaloEME,ChPiEMEOverCaloEME))
      ATH_MSG_INFO( "REGTEST TauDetails->ChPiEMEOverCaloEME() returns " << ChPiEMEOverCaloEME);

    if ( (*tauIt)->detail(xAOD::TauJetParameters::etEMAtEMScale,EtEm)){
      EtEm /=1000;
      ATH_MSG_INFO( "REGTEST TauDetails->etEMAtEMScale() returns " << EtEm);}

    if ( (*tauIt)->detail(xAOD::TauJetParameters::etHadAtEMScale,EtHad)){
       EtHad /=1000;
       ATH_MSG_INFO( "REGTEST TauDetails->etHadAtEMScale() returns " << EtHad);}


    if( !(*tauIt)->jetLink().isValid() ) {
      ATH_MSG_WARNING("tau does not have jet seed");
      return StatusCode::SUCCESS;
    }

    const xAOD::Jet* pJetSeed = *((*tauIt)->jetLink());

    xAOD::JetConstituentVector::const_iterator clusItr  = pJetSeed->getConstituents().begin();
    xAOD::JetConstituentVector::const_iterator clusItrE = pJetSeed->getConstituents().end();

    for (int clusCount = 0; clusItr != clusItrE; ++clusItr, ++clusCount) {

      ATH_MSG_INFO( "REGTEST Tau Cluster " << clusCount << " pt = " << (*clusItr)->pt()
		    << " eta = " << (*clusItr)->eta()
		    << " phi = " << (*clusItr)->phi() );

    }


    for (unsigned int trackNum = 0;  trackNum < (*tauIt)->nTracks(); ++trackNum) {

      const xAOD::TrackParticle *linkTrack = (*tauIt)->track(trackNum)->track();
      if (!linkTrack) {
     	ATH_MSG_WARNING("can't get tau linked track");
     	return StatusCode::SUCCESS;
      } else {
     	ATH_MSG_DEBUG("Got the tau linked track");
      }

      ATH_MSG_INFO( "REGTEST Tau linked track " << trackNum << " pt = " << linkTrack->pt()
      		    << " eta = " << linkTrack->eta()
      		    << " phi = " << linkTrack->phi() );

    }


  }// end for

  return StatusCode::SUCCESS;

}

/////////////////////////////////////////////////////////////////////////////////

StatusCode TrigEDMChecker::dumpxAODTrackParticle() {

  ATH_MSG_DEBUG("In dumpxAODTrackParticle()");

  ATH_MSG_INFO("REGTEST ==========START of xAOD::TrackParticle DUMP===========");

	std::vector<std::string> SGkeys;
	SGkeys.push_back("HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bjet_EFID");
	SGkeys.push_back("HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bphysics_EFID");
	SGkeys.push_back("HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_EFID");
	SGkeys.push_back("HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_FullScan_EFID");
	SGkeys.push_back("HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_EFID");
	SGkeys.push_back("HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Photon_EFID");
	SGkeys.push_back("HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_EFID");

	StatusCode returnsc = StatusCode::SUCCESS;

	for (unsigned int SGkey = 0; SGkey < SGkeys.size(); ++SGkey) {
		const xAOD::TrackParticleContainer* trackParticleContainer=0;
		StatusCode sc = evtStore()->retrieve(trackParticleContainer,SGkeys.at(SGkey));
		if (sc.isFailure()) {
          ATH_MSG_INFO("REGTEST No track particle container found with key " << SGkeys.at(SGkey));
			continue;
		}
		ATH_MSG_INFO("REGTEST TrackParticleContainer retrieved with key " << SGkeys.at(SGkey)
                     << " and size " << trackParticleContainer->size());
        
		xAOD::TrackParticleContainer::const_iterator trackParticleItr = trackParticleContainer->begin();
		xAOD::TrackParticleContainer::const_iterator trackParticleLast = trackParticleContainer->end();

		for (int index = 0; trackParticleItr != trackParticleLast; ++trackParticleItr, ++index) {
          ATH_MSG_INFO("REGTEST Looking at Track Particle " << index);

          ATH_MSG_INFO("REGTEST IParticle functions:");
		  ATH_MSG_INFO("REGTEST pt: " << (*trackParticleItr)->pt()
                       << "/eta: " << (*trackParticleItr)->eta()
                       << "/phi: " << (*trackParticleItr)->phi()
                       << "/m: " << (*trackParticleItr)->m()
                       << "/e: " << (*trackParticleItr)->e()
                       << "/rapidity: " << (*trackParticleItr)->rapidity());

          ATH_MSG_INFO("REGTEST Defining parameters functions:");
		  ATH_MSG_INFO("REGTEST charge: " << (*trackParticleItr)->charge()
                       << "/d0: " << (*trackParticleItr)->d0()
                       << "/z0: " << (*trackParticleItr)->z0()
                       << "/phi0: " << (*trackParticleItr)->phi0()
                       << "/theta: " << (*trackParticleItr)->theta()
                       << "/qOverP: " << (*trackParticleItr)->qOverP()
                       << "/vx: " << (*trackParticleItr)->vx()
                       << "/vy: " << (*trackParticleItr)->vy()
                       << "/vz: " << (*trackParticleItr)->vz());

			// Curvilinear functions skipped

          ATH_MSG_INFO("REGTEST Fit quality functions:");
		  ATH_MSG_INFO("REGTEST chiSquared: " << (*trackParticleItr)->chiSquared()
                       << "/numberDoF: " << (*trackParticleItr)->numberDoF());

			// TrackInfo functions skipped

          ATH_MSG_INFO("REGTEST summaryValue variables:");
          msg() << MSG::INFO << "REGTEST ";
			uint8_t numberOfBLayerHits = 0;
			if ( (*trackParticleItr)->summaryValue(numberOfBLayerHits, xAOD::numberOfBLayerHits) ) {
				msg() << "/numberOfBLayerHits: " << static_cast<int>(numberOfBLayerHits);
			} else {
				msg() << "/numberOfBLayerHits not found";
			}

			uint8_t numberOfPixelHits = 0;
			if ( (*trackParticleItr)->summaryValue(numberOfPixelHits, xAOD::numberOfPixelHits) ) {
				msg() << "/numberOfPixelHits: " << static_cast<int>(numberOfPixelHits);
			} else {
				msg() << "/numberOfPixelHits not found";
			}

			uint8_t numberOfPixelHoles = 0;
			if ( (*trackParticleItr)->summaryValue(numberOfPixelHoles, xAOD::numberOfPixelHoles) ) {
				msg() << "/numberOfPixelHoles: " << static_cast<int>(numberOfPixelHoles);
			} else {
				msg() << "/numberOfPixelHoles not found";
			}

			uint8_t numberOfSCTHits = 0;
			if ( (*trackParticleItr)->summaryValue(numberOfSCTHits, xAOD::numberOfSCTHits) ) {
				msg() << "/numberOfSCTHits: " << static_cast<int>(numberOfSCTHits);
			} else {
				msg() << "/numberOfSCTHits not found";
			}

			uint8_t numberOfSCTHoles = 0;
			if ( (*trackParticleItr)->summaryValue(numberOfSCTHoles, xAOD::numberOfSCTHoles) ) {
				msg() << "/numberOfSCTHoles: " << static_cast<int>(numberOfSCTHoles);
			} else {
				msg() << "/numberOfSCTHoles not found";
			}

			uint8_t numberOfTRTHits = 0;
			if ( (*trackParticleItr)->summaryValue(numberOfTRTHits, xAOD::numberOfTRTHits) ) {
				msg() << "/numberOfTRTHits: " << static_cast<int>(numberOfTRTHits);
			} else {
				msg() << "/numberOfTRTHits not found";
			}

			uint8_t numberOfTRTHoles = 0;
			if ( (*trackParticleItr)->summaryValue(numberOfTRTHoles, xAOD::numberOfTRTHoles) ) {
				msg() << "/numberOfTRTHoles: " << static_cast<int>(numberOfTRTHoles);
			} else {
				msg() << "/numberOfTRTHoles not found";
			}
			msg() << endmsg;
		}
	}

	ATH_MSG_INFO("REGTEST ==========END of xAOD::TrackParticle DUMP===========");
	ATH_MSG_DEBUG("dumpxAODTrackParticles() succeeded");

	return returnsc;
}

StatusCode TrigEDMChecker::dumpxAODVertex() {

  ATH_MSG_DEBUG("In dumpxAODVertex()");

  ATH_MSG_INFO("REGTEST ==========START of xAOD::Vertex DUMP===========");

	const xAOD::VertexContainer* vertexContainer=0;
	StatusCode sc = evtStore()->retrieve(vertexContainer,"HLT_xAOD__VertexContainer_xPrimVx");
	if (sc.isFailure()) {
      ATH_MSG_INFO("REGTEST No vertex container");
		return StatusCode::FAILURE;
	}
	ATH_MSG_INFO("REGTEST VertexContainer retrieved");

	xAOD::VertexContainer::const_iterator vertexItr = vertexContainer->begin();
	xAOD::VertexContainer::const_iterator vertexLast = vertexContainer->end();

	for (int index = 0; vertexItr != vertexLast; ++vertexItr, ++index) {
      ATH_MSG_INFO("REGTEST Looking at Vertex " << index);

      ATH_MSG_INFO("REGTEST Public Member Functions:");
      ATH_MSG_INFO("REGTEST x: " << (*vertexItr)->x()
                   << "/y: " << (*vertexItr)->y()
                   << "/z: " << (*vertexItr)->z());

      ATH_MSG_INFO("REGTEST Public Member Functions:");
      ATH_MSG_INFO("REGTEST chiSquared: " << (*vertexItr)->chiSquared()
                   << "/numberDoF: " << (*vertexItr)->numberDoF());
	}

	ATH_MSG_INFO("REGTEST ==========END of xAOD::Vertex DUMP===========");
	ATH_MSG_DEBUG("dumpxAODVertex() succeeded");

	return StatusCode::SUCCESS;
}

StatusCode TrigEDMChecker::dumpTDT(const EventContext& ctx) {
  using namespace TrigCompositeUtils; // LinkInfo
  ATH_MSG_INFO( "REGTEST ==========START of TDT DUMP===========" );
  // Note: This minimal TDT dumper is for use during run-3 dev
  std::string chain = m_dumpNavForChain;
  if (chain.empty()) {
    chain = "HLT_.*";
  }
  std::vector<std::string> confChains = m_trigDec->getListOfTriggers(chain);
  const std::string IdentifierStr{"Identifier"};
  for (const auto& item : confChains) {
    bool passed = m_trigDec->isPassed(item);
    ATH_MSG_INFO("  HLT Item " << item << " (numeric ID " << TrigConf::HLTUtils::string2hash(item, IdentifierStr) << ") passed raw? " << passed);
    if (m_trigDec->getNavigationFormat() == "TriggerElement") {
      ATH_MSG_DEBUG("    Skipping Run 2 features in this dumper");
      continue;
    }
    std::vector< LinkInfo<xAOD::IParticleContainer> > passFeatures = m_trigDec->features<xAOD::IParticleContainer>(item);
    if (passFeatures.size()) {
      ATH_MSG_INFO("    " << item << " Passed Final IParticle features size: " << passFeatures.size());
      for (const LinkInfo<xAOD::IParticleContainer>& li : passFeatures) {
        if (!li.isValid()) {
          ATH_MSG_WARNING("      Unable to access feature - link invalid.");
        } else {
          try {
            std::string state = "ACTIVE";
            if (li.state == ActiveState::INACTIVE) state = "INACTIVE";
            else if (li.state == ActiveState::UNSET) state = "UNSET";
            ATH_MSG_INFO("      IParticle Feature from " << li.link.dataID() << " index:" << li.link.index() << " pt:" << (*li.link)->pt() << " eta:" << (*li.link)->eta() << " phi:" << (*li.link)->phi() << " state:" << state);
          } catch (const std::exception& e) {
            ATH_MSG_WARNING("      Unable to dereference feature {" << e.what() << "}");
          }
        }
      }
    }
    std::vector< LinkInfo<xAOD::IParticleContainer> > passAndFailFeatures = m_trigDec->features<xAOD::IParticleContainer>(item, TrigDefs::includeFailedDecisions);
    if (passAndFailFeatures.size()) {
      ATH_MSG_INFO("    " << item << " Passed+Failed Final IParticle features size: " << passAndFailFeatures.size());
      for (const LinkInfo<xAOD::IParticleContainer>& li : passAndFailFeatures) {
        if (!li.isValid()) {
          ATH_MSG_WARNING("      Unable to access feature - link invalid.");
        } else {
          try {
            std::string state = "ACTIVE";
            if (li.state == ActiveState::INACTIVE) state = "INACTIVE";
            else if (li.state == ActiveState::UNSET) state = "UNSET";
            ATH_MSG_INFO("      IParticle Feature from " << li.link.dataID() << " index:" << li.link.index() << " pt:" << (*li.link)->pt() << " eta:" << (*li.link)->eta() << " phi:" << (*li.link)->phi() << " state:" << state);
          } catch (const std::exception& e) {
            ATH_MSG_WARNING("      Unable to dereference feature {" << e.what() << "}");
          }
        }
      }
    }
    std::vector< LinkInfo<xAOD::IParticleContainer> > allFeatures = m_trigDec->features<xAOD::IParticleContainer>(item, TrigDefs::includeFailedDecisions, "", TrigDefs::allFeaturesOfType);
    if (allFeatures.size()) {
      ATH_MSG_INFO("    " << item << " Passed+Failed ALL IParticle features size: " << allFeatures.size());
      for (const LinkInfo<xAOD::IParticleContainer>& li : allFeatures) {
        if (!li.isValid()) {
          ATH_MSG_WARNING("      Unable to access feature - link invalid.");
        } else {
          try {
            std::string state = "ACTIVE";
            if (li.state == ActiveState::INACTIVE) state = "INACTIVE";
            else if (li.state == ActiveState::UNSET) state = "UNSET";
            ATH_MSG_INFO("      IParticle Feature from " << li.link.dataID() << " index:" << li.link.index() << " pt:" << (*li.link)->pt() << " eta:" << (*li.link)->eta() << " phi:" << (*li.link)->phi() << " state:" << state);
          } catch (const std::exception& e) {
            ATH_MSG_WARNING("      Unable to dereference feature {" << e.what() << "}");
          }
        }
      }
    }
  }

  if (m_trigDec->getNavigationFormat() == "TrigComposite") {
    // Check associateToEventView helper function
    std::vector< LinkInfo<xAOD::IParticleContainer> > muons = m_trigDec->features<xAOD::IParticleContainer>("HLT_mu24_idperf_L1MU20", TrigDefs::Physics, "HLT_MuonL2CBInfo");
    SG::ReadHandle<xAOD::TrackParticleContainer> muonTracksReadHandle(m_muonTracksKey, ctx);
    for (const LinkInfo<xAOD::IParticleContainer>& mu : muons) {
      // Note: auto here refers to type std::pair< xAOD::TrackParticleContainer::const_iterator, xAOD::TrackParticleContainer::const_iterator>
      const auto roiTrackItPair = m_trigDec->associateToEventView<xAOD::TrackParticleContainer>(muonTracksReadHandle, mu, "roi");
      const xAOD::TrackParticleContainer::const_iterator startIt = roiTrackItPair.first;
      const xAOD::TrackParticleContainer::const_iterator stopIt  = roiTrackItPair.second;
      ATH_MSG_INFO("Muon pT: " << (*mu.link)->pt() << " is from the same ROI as tracks with index " 
        << std::distance(muonTracksReadHandle->begin(), startIt) << "-" << std::distance(muonTracksReadHandle->begin(), stopIt) 
        << ", which is " << std::distance(startIt, stopIt) << " tracks, out of " << muonTracksReadHandle->size() << " total tracks.");
      for (xAOD::TrackParticleContainer::const_iterator it = startIt; it != stopIt; ++it) {
        ATH_MSG_VERBOSE(" -- Track " << std::distance(startIt, it) << " in this ROI, pT: " << (*it)->pt() );
      }
    }
  }

  ATH_MSG_INFO( "REGTEST ==========END of TDT DUMP===========" );
  return StatusCode::SUCCESS;
}

StatusCode TrigEDMChecker::dumpTrigComposite() {
  ATH_MSG_INFO( "REGTEST ==========START of xAOD::TrigCompositeContainer DUMP===========" );

  if (m_doDumpAllTrigComposite) {
    m_dumpTrigCompositeContainers.clear();
    const CLID TrigCompositeCLID = static_cast<CLID>( ClassID_traits< xAOD::TrigCompositeContainer >::ID() );
    evtStore()->keys(TrigCompositeCLID, m_dumpTrigCompositeContainers.value());
    std::string typeNameTC;
    ATH_CHECK(m_clidSvc->getTypeNameOfID(TrigCompositeCLID, typeNameTC));
    ATH_MSG_DEBUG("dumpTrigComposite got " <<  m_dumpTrigCompositeContainers.size() << " keys for " << typeNameTC);
  } else {
    ATH_MSG_DEBUG("Using supplied " <<  m_dumpTrigCompositeContainers.size() << " keys");
  }

  for ( const std::string & key: m_dumpTrigCompositeContainers ) {
    // get the collection
    if ( not evtStore()->contains<xAOD::TrigCompositeContainer>(key) ) {    
      ATH_MSG_WARNING("Absent TrigCompositeContainer: " << key );
      continue;
    }
    ATH_MSG_DEBUG( "#################### Dumping container of : " << key );
    const xAOD::TrigCompositeContainer* cont= nullptr;
    ATH_CHECK( evtStore()->retrieve( cont, key ) );
    
    size_t count = 0;
    for ( auto tc: *cont ) {
      ATH_MSG_DEBUG("########## ELEMENT " << count++);
      ATH_MSG_DEBUG(*tc);
      // Get the objects we know of
      for (size_t i = 0; i < tc->linkColNames().size(); ++i) ATH_CHECK(checkTrigCompositeElementLink(tc, i));
    }
  }
  ATH_MSG_INFO( "REGTEST ==========END of xAOD::TrigCompositeContainer DUMP===========" );
  return StatusCode::SUCCESS;
}



StatusCode TrigEDMChecker::checkTrigCompositeElementLink(const xAOD::TrigComposite* tc, size_t element) { 

  const std::string name = tc->linkColNames().at(element);
  const CLID clid = static_cast<CLID>(tc->linkColClids().at(element));

  if (clid == ClassID_traits< TrigRoiDescriptorCollection >::ID()) { 

    const ElementLink<TrigRoiDescriptorCollection> elementLink = tc->objectLink<TrigRoiDescriptorCollection>(name);
    if (!elementLink.isValid()) ATH_MSG_WARNING("  Invalid element link to TrigRoiDescriptorCollection, link name:'" << name << "'");
    else ATH_MSG_DEBUG("  Dereferenced link '" << name << "'' to TrigRoiDescriptor:" << *elementLink);

  } else if (clid == ClassID_traits< DataVector< LVL1::RecEmTauRoI > >::ID()) { // There could be a few ROI types....
    // CLASS_DEF( DataVector< LVL1::RecEmTauRoI >, 6256, 1 )

    const ElementLink<DataVector< LVL1::RecEmTauRoI >> elementLink = tc->objectLink<DataVector< LVL1::RecEmTauRoI >>(name);
    if (!elementLink.isValid()) ATH_MSG_WARNING("  Invalid element link to LVL1::RecEmTauRoI, link name:'" << name << "'");
    else ATH_MSG_DEBUG("  Dereferenced link '" << name << "' to LVL1::RecEmTauRoI:" << *elementLink);

   } else if (clid == ClassID_traits< xAOD::TrigCompositeContainer >::ID()) {
    
    const ElementLink<xAOD::TrigCompositeContainer> elementLink = tc->objectLink<xAOD::TrigCompositeContainer>(name);
    if (!elementLink.isValid()) ATH_MSG_WARNING("  Invalid element link to TrigComposite, link name:'" << name << "'");
    else ATH_MSG_DEBUG("  Dereferenced link '" << name << "' to TrigComposite, TC name:'" << (*elementLink)->name() << "'");

   } else if (clid == ClassID_traits< ViewContainer >::ID()) {
    
    const ElementLink<ViewContainer> elementLink = tc->objectLink<ViewContainer>(name);
    if (!elementLink.isValid()) ATH_MSG_WARNING("  Invalid element link to View, link name:'" << name << "'");
    else ATH_MSG_DEBUG("  Dereferenced link '" << name << "' to View:'" << *elementLink);

   } else if (name == "feature") {

    if (clid == ClassID_traits< xAOD::TrigEMClusterContainer >::ID()) {

      const ElementLink<xAOD::TrigEMClusterContainer> elementLink = tc->objectLink<xAOD::TrigEMClusterContainer>(name);
      if (!elementLink.isValid()) ATH_MSG_WARNING("  Invalid element link to xAOD::TrigEMClusterContainer 'feature'");
      else ATH_MSG_DEBUG("  Dereferenced xAOD::TrigEMClusterContainer link 'feature', Energy:" << (*elementLink)->energy());

    } else if (clid == ClassID_traits< xAOD::TrigMissingETContainer >::ID()) {

      const ElementLink<xAOD::TrigMissingETContainer> elementLink = tc->objectLink<xAOD::TrigMissingETContainer>(name);
      if (!elementLink.isValid()) ATH_MSG_WARNING("  Invalid element link to xAOD::TrigMissingETContainer 'feature'");
      else ATH_MSG_DEBUG("  Dereferenced xAOD::TrigMissingETContainer link 'feature', ex:" << (*elementLink)->ex() << " ey:" << (*elementLink)->ey());

    } else {

      try {
        const ElementLink<xAOD::IParticleContainer> elementLink = tc->objectLink<xAOD::IParticleContainer>(name);
        if (!elementLink.isValid()) ATH_MSG_WARNING("  Invalid element link to 'feature'");
        else ATH_MSG_DEBUG("  Dereferenced IParticle link 'feature', pt:" << (*elementLink)->pt() << " eta:" << (*elementLink)->eta() << " phi:" <<  (*elementLink)->phi());
      } catch(std::runtime_error& e) {
        ATH_MSG_WARNING("  Cannot dereference 'feature' as IParticle: '" << e.what() << "'");
      }

    }

  } else {
    ATH_MSG_DEBUG("  Ignoring link to '" << name << "' with link CLID " << clid);
  }

  return StatusCode::SUCCESS;

}


StatusCode TrigEDMChecker::TrigCompositeNavigationToDot(std::string& returnValue, bool& pass) {

  using namespace TrigCompositeUtils;

  // This constexpr is evaluated at compile time
  const CLID TrigCompositeCLID = static_cast<CLID>( ClassID_traits< xAOD::TrigCompositeContainer >::ID() );
  std::vector<std::string> keys;
  if ( m_dumpTrigCompositeContainers.size() == 0 ) {
    evtStore()->keys(TrigCompositeCLID, keys);
  }
  else {
    keys = m_dumpTrigCompositeContainers;
  }
  std::string typeNameTC;
  ATH_CHECK(m_clidSvc->getTypeNameOfID(TrigCompositeCLID, typeNameTC));
  ATH_MSG_DEBUG("Got " <<  keys.size() << " keys for " << typeNameTC);

  HLT::Identifier chainID(m_dumpNavForChain);
  DecisionIDContainer chainIDs;
  chainIDs.insert( chainID.numeric() );

  std::set<int> converted;

  const Trig::ChainGroup* cg = m_trigDec->getChainGroup(m_dumpNavForChain);
  pass = cg->isPassed(TrigDefs::requireDecision);
  std::vector<std::string> chains = cg->getListOfTriggers();
  for (const std::string& chain : chains) {
    const TrigConf::HLTChain* hltChain = m_trigDec->ExperimentalAndExpertMethods().getChainConfigurationDetails(chain);
    const HLT::Identifier chainID_tmp( hltChain->chain_name() );
    chainIDs.insert( chainID_tmp.numeric() );
    const std::vector<size_t> legMultiplicites = hltChain->leg_multiplicities();
    if (legMultiplicites.size() == 0) {
      ATH_MSG_ERROR("chain " << chainID_tmp << " has invalid configuration, no multiplicity data.");
    } else if (legMultiplicites.size() > 1) {
      // For multi-leg chains, the DecisionIDs are handled per leg.
      // We don't care here exactly how many objects are required per leg, just that there are two-or-more legs
      for (size_t legNumeral = 0; legNumeral < legMultiplicites.size(); ++legNumeral) {
        const HLT::Identifier legID = TrigCompositeUtils::createLegName(chainID_tmp, legNumeral);
        chainIDs.insert( legID.numeric() );
      }
    }
  }

  // First retrieve them all (this should not be needed in future)
  const DecisionContainer* container = nullptr;
  for (const std::string& key : keys) ATH_CHECK( evtStore()->retrieve( container, key ) );

  std::stringstream ss;
  ss << "digraph {" << std::endl;
  ss << "  node [shape=rectangle]" << std::endl;
  ss << "  rankdir = BT" << std::endl;
  const std::string seedStr{"seed"};
  // Now process them
  for (const std::string& key : keys) {
    if ( not m_doDumpAllTrigComposite ) {
      if ( not key.starts_with( "HLTNav_") ) { // Nav containers should always start with HLTNav_
        continue;
      }
    }
    ATH_CHECK( evtStore()->retrieve( container, key ) );
    ATH_MSG_DEBUG("Processing collection " << key << " to be added to the navigation graph");
    bool writtenHeader = false;

    for (const Decision* tc : *container ) {
      // Output my ID in the graph. 
      const DecisionContainer* container = static_cast<const DecisionContainer*>( tc->container() );
      const ElementLink<DecisionContainer> selfEL = ElementLink<DecisionContainer>(*container, tc->index());
      std::vector<ElementLink<DecisionContainer>> seedELs = tc->objectCollectionLinks<DecisionContainer>(seedStr);
      const bool isHypoAlgNode = tc->name() == "H";
      const bool isComboHypoAlgNode = tc->name() == "CH";
      const std::vector<DecisionID>& decisions = tc->decisions();
      const uint32_t selfKey = selfEL.key();
      const uint32_t selfIndex = selfEL.index();
      if (m_dumpNavForChain != "") {
        bool doDump = false;
        // Check me
        for (DecisionID id : decisions) {
          if (chainIDs.count(id) == 1) {
            doDump = true;
            break;
          }
        }
        // Check my seeds
        if (!doDump and (isHypoAlgNode or isComboHypoAlgNode) and not m_excludeFailedHypoNodes) {
          for (const ElementLink<DecisionContainer>& s : seedELs) {
            const std::vector<DecisionID>& seedDecisions = (*s)->decisions();
            for (DecisionID id : seedDecisions) {
              if (chainIDs.count(id) == 1) {
                doDump = true;
                break;
              }
            }
          }
        }
        if (!doDump) {
          continue;
        }
      }
      if (!writtenHeader) {
        writtenHeader = true;
        ss << "  subgraph " << key << " {" << std::endl;
        ss << "    label=\"" << key << "\"" << std::endl;
      }
      static const std::string scheme = "rdpu9";
      std::string color = "1";
      if      (tc->name() == "L1") { color = "1"; }
      else if (tc->name() == "F")  { color = "2"; }
      else if (tc->name() == "IM") { color = "3"; }
      else if (tc->name() == "H")  { color = "4"; }
      else if (tc->name() == "CH") { color = "5"; }
      else if (tc->name() == "SF") { color = "6"; }
      else if (tc->name() == "HLTPassRaw") { color = "7"; }
      ss << "    \"" << selfKey << "_" << selfIndex << "\" [colorscheme="<<scheme<<",style=filled,fillcolor="<<color<<",label=<<B>Container</B>=" << typeNameTC; 
      if (tc->name() != "") ss << " <B>Name</B>=" << tc->name();
      ss << "<BR/><B>Key</B>=" << key << "<BR/><B>Index</B>=" << selfIndex;
      const bool isRemapped = tc->isRemapped();
      if (isHypoAlgNode) ss << " <B>linksRemapped</B>=" << (isRemapped ? "Y" : "N");
      if (decisions.size() > 0) {
        ss << "<BR/><B>Pass</B>=";
        size_t c = 0;
        for (unsigned decisionID : decisions) {
          HLT::Identifier dID(decisionID);
          std::string highlight = (dID.numeric() == chainID.numeric() ? "<B>[CHAIN:" : "");
          if (highlight == "" and chainIDs.count(dID.numeric()) == 1 and TrigCompositeUtils::isLegId(dID)) {
            highlight = "<B>[LEG" + std::to_string(TrigCompositeUtils::getIndexFromLeg(dID)) + ":";
          }
          ss << std::hex << highlight << decisionID << (!highlight.empty() ? "]</B>" : "") << std::dec << ",";
          if (c++ == 5) {
            ss << "<BR/>";
            c = 0;
          }
        }
      }
      ss << ">]" << std::endl;
      // Output all the things I link to
      size_t seedCount = 0;
      for (size_t i = 0; i < tc->linkColNames().size(); ++i) {
        const std::string link = tc->linkColNames().at(i);
        if (link == "seed" || link == "seed__COLL") {
          ElementLink<DecisionContainer> seedEL = seedELs.at(seedCount++);
          const uint32_t seedKey = tc->linkColKeys().at(i);
          const uint32_t seedIndex = tc->linkColIndices().at(i);
          ATH_CHECK( seedKey == seedEL.key() );
          ATH_CHECK( seedIndex == seedEL.index() );
          if (m_dumpNavForChain != "") { // Only print "seed" link to nodes we include in our search
            const std::vector<DecisionID> seedDecisions = (*seedEL)->decisions();
            bool doSeedLink = false;
            for (DecisionID id : seedDecisions) {
              if (chainIDs.count(id) == 1) {
                doSeedLink = true;
                break;
              }
            }            
            if (!doSeedLink) {
              continue;
            }
          }
          ss << "    \"" << selfKey << "_" << selfIndex << "\" -> \"" << seedKey << "_" << seedIndex << "\" [colorscheme="<<scheme<<",color=9,fontcolor=8,label=\"seed\"]" << std::endl;
        } else {
          // Start with my class ID
          std::string linkColour = "12";
          std::string linkBackground = "11";
          const std::string extScheme =  "paired12";
          if      (link == "roi") { linkColour="2"; linkBackground="1"; }
          else if (link == "initialRoI") { linkColour="2"; linkBackground="1"; }
          else if (link == "initialRecRoI") { linkColour="8"; linkBackground="7"; }
          else if (link == "feature") { linkColour="4"; linkBackground="3"; }
          else if (link == "view") { linkColour="10"; linkBackground="9"; }
          const CLID linkCLID = static_cast<CLID>( tc->linkColClids().at(i) );
          // Use it to get my class name
          std::string tname;
          ATH_CHECK(m_clidSvc->getTypeNameOfID(linkCLID, tname));
          // Now get the sgkey I'm linking to & the index
          const SG::sgkey_t key = (isRemapped ? static_cast<SG::sgkey_t>( tc->linkColKeysRemap().at(i) ) : static_cast<SG::sgkey_t>( tc->linkColKeys().at(i) ));
          const unsigned index = (isRemapped ? tc->linkColIndicesRemap().at(i) : tc->linkColIndices().at(i));
          // Look it up
          CLID checkCLID;
          const std::string* keyStr = evtStore()->keyToString(key, checkCLID); // I don't own this str
          if (keyStr != nullptr && checkCLID != linkCLID) {
            std::string tnameOfCheck;
            m_clidSvc->getTypeNameOfID(checkCLID, tnameOfCheck).ignore(); // Might be invalid. But we don't care.
            ATH_MSG_ERROR("Inconsistent CLID " << checkCLID << " [" << tnameOfCheck << "] stored in storegate for key " << key
              << ". We were expecting " << linkCLID << " [" << tname << "]");
          }

          std::string tnameEscape;
          for (std::string::const_iterator i = tname.begin(); i != tname.end(); ++i) {
            unsigned char c = *i;
            if (c == '<') {
              tnameEscape += "&lt;";
            } else if (c == '>') {
              tnameEscape += "&gt;";
            } else {
              tnameEscape += c;
            }
          }

          // Print
          ss << "    \"" << selfKey << "_" << selfIndex << "\" -> \"" << key << "_" << index << "\" ";
          ss << "[colorscheme="<<extScheme<<",color="<<linkColour<<",fontcolor="<<linkColour<<",arrowhead=empty,label=\"" << link << "\"]" << std::endl; 

          // Check if we are linking to self (e.g. a dummy-feature), don't output a new box for this
          const bool linkToSelf = (selfKey == key and selfIndex == index);

          if (converted.count(key + index) == 0 and not linkToSelf) {
            ss << "    \"" << key << "_" << index << "\" [colorscheme="<<extScheme<<",style=filled,fillcolor="<<linkBackground<<",label=<<B>Container</B>=" << tnameEscape << "<BR/><B>Key</B>=";
            if (keyStr != nullptr) ss << *keyStr;
            else ss << "[<I>KEY "<< key <<" NOT IN STORE</I>] "; 
            ss << "<BR/><B>Index</B>=" << index << ">]";
          }

          converted.insert(key + index);
        }
      }
    }
    if (writtenHeader) {
      ss << "  }" << std::endl;
    }
  }

  ss << "}" << std::endl;

  returnValue.assign( ss.str() );
  return StatusCode::SUCCESS;
}

StatusCode TrigEDMChecker::dumpNavigation(const EventContext& ctx)
{
  // Get object from store
  const xAOD::TrigNavigation * navigationHandle = nullptr;
  ATH_CHECK( evtStore()->retrieve( navigationHandle, m_navigationHandleKey.key() ) );
  // Proper version doesn't work - conversion issue?
  //SG::ReadHandle< xAOD::TrigNavigation > navigationHandle = SG::ReadHandle< xAOD::TrigNavigation >( m_navigationHandleKey );
  //if ( !navigationHandle.isValid() ) ATH_MSG_FATAL( "Could not retrieve navigation" );

  // Get serialised navigation info
  const std::vector< unsigned int > serialisedNavigation = navigationHandle->serialized();
  ATH_MSG_INFO( "Serialised navigation size: " << serialisedNavigation.size() );

  // Convert the input
  HLT::Navigation* testNav = m_navigationTool.get();
  testNav->deserialize( serialisedNavigation );

  // Make a map of TE name hashes
  const xAOD::TriggerMenuContainer * testMenu = nullptr;
  ATH_CHECK( inputMetaStore()->retrieve( testMenu, "TriggerMenu" ) );
  std::map< int, std::string > hash2string;
  for ( auto const& sequence : testMenu->front()->sequenceInputTEs() ) {
    for ( auto const& name : sequence ) {
      int hash = TrigConf::HLTUtils::string2hash( name );
      hash2string[ hash ] = name;
    }
  }

  // Map TE names to chain names
  unsigned int chainCounter = 0;
  std::map< int, std::string > hash2chain;
  for ( auto const& chain : testMenu->front()->chainSignatureOutputTEs() ) {

    // Find the chain name
    std::string chainName = testMenu->front()->chainNames()[ chainCounter ];
    ++chainCounter;

    // Find all associated TEs
    for ( auto const& signature : chain ) {
      for ( auto const& name : signature ) {
        int hash = TrigConf::HLTUtils::string2hash( name );
        hash2string[ hash ] = name; // for decoding
        hash2chain[ hash ] = chainName;
      }
    }
  }

  // Define a map of TE features, to the TEs that use them. Needs a custom sort lambda
  auto cmpLambda = []( const HLT::TriggerElement::FeatureAccessHelper &lhs, const HLT::TriggerElement::FeatureAccessHelper &rhs) { 

    // Compare indices if CLID matches
    if ( lhs.getCLID() == rhs.getCLID() ) return ( lhs.getIndex() < rhs.getIndex() );

    // Compare CLIDs
    else return ( lhs.getCLID() < rhs.getCLID() );
  };
  std::map< HLT::TriggerElement::FeatureAccessHelper, std::vector< HLT::TriggerElement* >, decltype(cmpLambda) > feature2element(cmpLambda);

  // Retrieve all TE features and add them to the map
  std::vector< HLT::TriggerElement* > allTEs;
  testNav->getAll( allTEs, false );
  for ( auto element : allTEs ) {

    // Add TE features to the big map 
    for ( auto helper : element->getFeatureAccessHelpers() ) {
      feature2element[ helper ].push_back( element );
    }
  }

  // Debug - output all TEs and their ancestors
  // No duplication - only print terminal nodes
  for ( auto element : allTEs ) {
    if ( testNav->isTerminalNode( element ) ) {
      ATH_MSG_INFO( "+++++++++++ " << hash2string[ element->getId() ] << " is terminal node" );
      ATH_MSG_INFO( "ptr: " << element );
      std::queue< HLT::TriggerElement* > allAncestors;
      allAncestors.push( element );
      while ( allAncestors.size() ) {

        HLT::TriggerElement * thisElement = allAncestors.front();
        allAncestors.pop();
        auto theseAncestors = thisElement->getRelated( HLT::TriggerElement::Relation::seededByRelation );

        // Dump TE
        ATH_MSG_INFO( "te: " << thisElement->getId() << " " << hash2string[ thisElement->getId() ] );
        ATH_MSG_INFO( "  chain: " << hash2chain[ thisElement->getId() ] );
        for ( const auto& helper : thisElement->getFeatureAccessHelpers() ) {
          ATH_MSG_INFO( "   feat: " << helper );
        }
        ATH_MSG_INFO( theseAncestors.size() << " ancestors" );

        // Examine ancestors
        for ( auto ancestor : theseAncestors ) {
          allAncestors.push( ancestor );
        }
      }
    }
  }

  // Make the decision container
  SG::WriteHandle< TrigCompositeUtils::DecisionContainer > outputNavigation = TrigCompositeUtils::createAndStore( m_decisionsKey, ctx );
  auto decisionOutput = outputNavigation.ptr();

  // Find unique chains associated with a feature
  std::map< HLT::TriggerElement const*, std::vector< int > > element2decisions;
  for ( const auto& pair : feature2element ) {

    // Get the feature info
    std::string featureName = testNav->label( pair.first.getCLID(), pair.first.getIndex().subTypeIndex() );
    auto sgKey = evtStore()->stringToKey( featureName, pair.first.getCLID() );

    // Store RoIs with appropriate label ?
    std::string storeFeatureName = "feature";
/*    if ( pair.first.getCLID() == ClassID_traits< TrigRoiDescriptor >::ID() ) {
      storeFeatureName = "roi";
    }*/

    // Make a decision object for the feature
    auto decision = TrigCompositeUtils::newDecisionIn( decisionOutput );
    decision->typelessSetObjectLink( storeFeatureName, sgKey, pair.first.getCLID(), pair.first.getIndex().objectsBegin(), pair.first.getIndex().objectsEnd() );

    // Examine associated TEs, look for chains
    std::set< std::string > passedChains;
    for ( HLT::TriggerElement const* element : pair.second ) {

      // TODO - find out what chains actually passed!
      passedChains.insert( hash2chain[ element->getId() ] );

      // Index the TE
      int decisionNumber = decisionOutput->size() - 1;
      element2decisions[ element ].push_back( decisionNumber );
    }

    // Store unique chains in the decision
    for ( auto& chain : passedChains ) {
      TrigCompositeUtils::addDecisionID( TrigConf::HLTUtils::string2hash( chain ), decision );
    }
  }

  // Store decision ancestry (had to go through once before to ensure indices populated)
  unsigned int decisionCounter = 0;
  for ( const auto& pair : feature2element ) {

    // Get current decision
    auto decision = decisionOutput->at( decisionCounter );
    ++decisionCounter;

    // Find ancestor TEs
    for ( auto element : pair.second ) {
      auto theseAncestors = element->getRelated( HLT::TriggerElement::Relation::seededByRelation );
      for ( auto ancestor : theseAncestors ) {
        for ( int decisionIndex : element2decisions[ ancestor ] ) {
          TrigCompositeUtils::linkToPrevious( decision, m_decisionsKey.key(), decisionIndex );
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}
