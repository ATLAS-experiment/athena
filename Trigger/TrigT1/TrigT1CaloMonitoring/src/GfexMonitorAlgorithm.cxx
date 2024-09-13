/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
   */

#include "GfexMonitorAlgorithm.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloIdentifier/CaloGain.h"
#include "Identifier/Identifier.h"
#include "Identifier/HWIdentifier.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include <istream>

GfexMonitorAlgorithm::GfexMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
	: AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode GfexMonitorAlgorithm::initialize() {
	ATH_MSG_DEBUG("GfexMonitorAlgorithm::initialize");
	ATH_MSG_DEBUG("Package Name "<< m_packageName);

	ATH_CHECK( m_gFexJetTobKeyList.initialize() ) ;
	ATH_CHECK( m_gFexRhoTobKeyList.initialize() ) ;
	ATH_CHECK( m_gFexGlobalTobKeyList.initialize() ) ;

	// TOBs may come from trigger bytestream - renounce from scheduler
	renounceArray(m_gFexJetTobKeyList);
	renounceArray(m_gFexRhoTobKeyList);
	renounceArray(m_gFexGlobalTobKeyList);

	// Fill variable name map for global TOBs
	m_globTobVarMap.insert({"gScalarEJwoj", {"gFexMet", "gFexSumEt"}});
	m_globTobVarMap.insert({"gMETComponentsJwoj", {"METx", "METy"}});
	m_globTobVarMap.insert({"gMHTComponentsJwoj", {"MHTx", "MHTy"}});
	m_globTobVarMap.insert({"gMSTComponentsJwoj", {"MSTx", "MSTy"}});
	// NC and Rho commented out for future
	//  m_globTobVarMap.insert({"gMETComponentsNoiseCut", {"METx_NoiseCut", "METy_NoiseCut"}});
	//  m_globTobVarMap.insert({"gMETComponentsRms", {"METx_Rms", "METy_Rms"}});
	//  m_globTobVarMap.insert({"gScalarENoiseCut", {"gFexMet_NoiseCut", "gFexSumEt_NoiseCut"}});
	//  m_globTobVarMap.insert({"gScalarERms", {"gFexMet_Rms", "gFexSumEt_Rms"}});

	return AthMonitorAlgorithm::initialize();
}

StatusCode GfexMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
	ATH_MSG_DEBUG("GfexMonitorAlgorithm::fillHistograms");

	// Small-R and large-R jets container loop
	for (const auto& key : m_gFexJetTobKeyList){
		SG::ReadHandle<xAOD::gFexJetRoIContainer> jetContainer (key, ctx);
		auto lumi = GetEventInfo(ctx)->lumiBlock();
	
		// Check that this container is present
		if ( !jetContainer.isValid() ) {
			ATH_MSG_WARNING("No gFex jet container found in storegate: "<< key.key());
		}

		else {
			if (key.key() == "L1_gFexLRJetRoI"){
				const xAOD::gFexJetRoIContainer* jetContainerPtr = jetContainer.cptr();
			// Loop over all required pt cut values - LRJets
				for(auto ptCut : m_ptCutValuesgLJ){
					ATH_CHECK(fillJetHistograms(key.key(), jetContainerPtr, ptCut, lumi));
				}
			}
			if (key.key() == "L1_gFexSRJetRoI"){
				const xAOD::gFexJetRoIContainer* jetContainerPtr = jetContainer.cptr();
				// Loop over all required pt cut values - SRJets
				for(auto ptCut : m_ptCutValuesgJ){
					ATH_CHECK(fillJetHistograms(key.key(), jetContainerPtr, ptCut, lumi));
				}
			}
		}
	}  // end jet loop

	// Rho container loop
	for (const auto& key : m_gFexRhoTobKeyList){
		SG::ReadHandle<xAOD::gFexJetRoIContainer> rhoContainer (key, ctx);
		// Check that this container is present
		if ( !rhoContainer.isValid() ) {
			ATH_MSG_WARNING("No gFex rho container found in storegate: "<< key.key());
		}
		else {
			const xAOD::gFexJetRoIContainer* rhoContainerPtr = rhoContainer.cptr();
			ATH_CHECK(fillRhoHistograms(key.key(), rhoContainerPtr));
		}
	} // end rho container loop

	// Global TOB container loop
	for (const auto& key : m_gFexGlobalTobKeyList){
		SG::ReadHandle<xAOD::gFexGlobalRoIContainer> globalTobContainer (key, ctx);
		// Check that this container is present
		if ( !globalTobContainer.isValid() ) {
			ATH_MSG_WARNING("No gFex global TOB container found in storegate: "<< key.key());
		}
		else {
			const xAOD::gFexGlobalRoIContainer* globalTobContainerPtr = globalTobContainer.cptr();
			ATH_CHECK(fillGlobalTobHistograms(key.key(), globalTobContainerPtr));
		}
	} // end global TOBs container loop
	return StatusCode::SUCCESS;
}

StatusCode GfexMonitorAlgorithm::fillJetHistograms(const std::string& handleKey, const xAOD::gFexJetRoIContainer* container, const float& ptCutValue ,const auto& lbn) const {

	// Define name extension based on pT cut value
	std::string histNameExt = ptCutValue != -1. ? (std::string("_CutPt") + std::to_string(int(ptCutValue))) : "";
	auto gtype = Monitored::Scalar<int>(handleKey + "gFexType", 0.0);
	auto jetEta = Monitored::Scalar<float>(handleKey + "Eta" + histNameExt, 0.0);
	auto jetPhi = Monitored::Scalar<float>(handleKey + "Phi" + histNameExt, 0.0);
	auto jetPt = Monitored::Scalar<float>(handleKey + "Pt" + histNameExt, 0.0);
	auto binNumber = Monitored::Scalar<int>(handleKey+"binNumber"+histNameExt,0);
	auto lumiNumber = Monitored::Scalar<int>(handleKey+"LBN"+histNameExt, lbn );



	for(const xAOD::gFexJetRoI* gFexJetRoI : *container){
		jetEta = gFexJetRoI->eta();
		auto phi = gFexJetRoI->phi();
		jetPt  = gFexJetRoI->gFexTobEt();
		
		uint8_t etaidx = gFexJetRoI->iEta();
		uint8_t phiidx = gFexJetRoI->iPhi();

		int x = etaidx+1;
		int y = phiidx+1;
		binNumber = 40*(y-1)+x;


		if(jetPt > ptCutValue){
			if (handleKey == "L1_gFexSRJetRoI"){
				if (std::abs(jetEta)>=3.3){
					jetPhi = phi - M_PI/32;
					fill(m_packageName,jetEta,jetPhi);
					jetPhi = phi + M_PI/32;
					fill(m_packageName,jetEta,jetPhi,jetPt);
					fill(m_packageName,lumiNumber,binNumber);
				} else {
					jetPhi = phi;
					fill(m_packageName,jetEta,jetPhi,jetPt);
					fill(m_packageName,lumiNumber,binNumber);
				}	
			}
			jetPhi = phi;
			fill(m_packageName, jetEta, jetPhi, jetPt);
			fill(m_packageName,lumiNumber,binNumber);
		}
	}
	return StatusCode::SUCCESS;
}

StatusCode GfexMonitorAlgorithm::fillRhoHistograms(const std::string& handleKey, const xAOD::gFexJetRoIContainer* container) const {
	auto gFexRhoeT = Monitored::Scalar<float>(handleKey, 0.0);
	for(const xAOD::gFexJetRoI* gFexRhoRoI : *container){
		gFexRhoeT=gFexRhoRoI->gFexTobEt();
		fill(m_packageName, gFexRhoeT);
	}
	return StatusCode::SUCCESS;
}

StatusCode GfexMonitorAlgorithm::fillGlobalTobHistograms(const std::string& handleKey, const xAOD::gFexGlobalRoIContainer* container) const {
	// Find the variable names corresponding to the current key handle
	std::pair<std::string, std::string> varNames;
	for (const auto& [key, value] : m_globTobVarMap) {
		if (handleKey.find(key) != std::string::npos) {
			varNames = value;
			break;
		}
	}
	auto varOne = Monitored::Scalar<float>(varNames.first,0.0);
	auto varTwo = Monitored::Scalar<float>(varNames.second,0.0);

	for (const xAOD::gFexGlobalRoI* globRoI : *container) {
		varOne = globRoI->METquantityOne();
		varTwo = globRoI->METquantityTwo();
		fill(m_packageName, varOne, varTwo);
	}
	return StatusCode::SUCCESS;
}

GfexMonitorAlgorithm::FPGAType GfexMonitorAlgorithm::getFPGAType(const float& eta) const {
	if(eta < 0 && eta > -2.5) return FPGAType::FPGAa;
	if(eta > 0 && eta <  2.5) return FPGAType::FPGAb;
	if(std::abs(eta) > 2.5 && std::abs(eta)) return FPGAType::FPGAc;
	return FPGAType::None;
}
