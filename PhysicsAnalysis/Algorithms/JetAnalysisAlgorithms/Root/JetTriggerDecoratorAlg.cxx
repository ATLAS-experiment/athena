/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetAnalysisAlgorithms/JetTriggerDecoratorAlg.h"
#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigConfHLTData/HLTChain.h"

#include <algorithm>

namespace CP
{
  JetTriggerDecoratorAlg::JetTriggerDecoratorAlg(const std::string &name,
						 ISvcLocator *svcLoc)
    : EL::AnaAlgorithm(name, svcLoc),
      m_trigDecisionTool("Trig::TrigDecisionTool/TrigDecisionTool")
  {
    declareProperty("TrigDecisionTool", m_trigDecisionTool, "trigger decision tool");
  }
  
  StatusCode JetTriggerDecoratorAlg::initialize() {
    ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));

    ANA_CHECK(m_L1JetsInKey.initialize(m_doL1Matching));
    ANA_CHECK(m_HLTJetsInKey.initialize(m_doHLTMatching && !m_useEmulationTool));

    ANA_CHECK(m_trigDecisionTool.retrieve());
    if(m_useEmulationTool) ANA_CHECK(m_emulationTool.retrieve());
    
    if(m_doL1Matching){      
      ANA_CHECK(m_L1Et_decor.initialize(m_systematicsList, m_jetsHandle));
      ANA_CHECK(m_L1Eta_decor.initialize(m_systematicsList, m_jetsHandle));
      ANA_CHECK(m_L1Phi_decor.initialize(m_systematicsList, m_jetsHandle));
      ANA_CHECK(m_L1DR_decor.initialize(m_systematicsList, m_jetsHandle));
      ANA_CHECK(m_L1Threshold_decor.initialize(m_systematicsList, m_jetsHandle));
    }

    if(m_doHLTMatching){
      ANA_CHECK(m_HLTPt_decor.initialize(m_systematicsList, m_jetsHandle));
      ANA_CHECK(m_HLTEta_decor.initialize(m_systematicsList, m_jetsHandle));
      ANA_CHECK(m_HLTPhi_decor.initialize(m_systematicsList, m_jetsHandle));
      ANA_CHECK(m_HLTDR_decor.initialize(m_systematicsList, m_jetsHandle));
      ANA_CHECK(m_HLTThreshold_decor.initialize(m_systematicsList, m_jetsHandle));
    }

    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode JetTriggerDecoratorAlg::execute() {
    SG::ReadHandle<xAOD::JetRoIContainer> l1Jets;
    if(m_doL1Matching){
      l1Jets = SG::makeHandle(m_L1JetsInKey);
      ANA_CHECK(l1Jets.isValid());
    }
    std::regex l1NameParser("(\\d*)(J)(\\d*)((p|\\.)(\\d*)ETA(\\d*))?");

    SG::ReadHandle<xAOD::JetContainer> hltJetsFromCont;
    if (m_doHLTMatching) {
      if(m_useEmulationTool)
        ANA_MSG_DEBUG(m_trigger << " isPassed "<<m_emulationTool->isPassed(m_trigger));
      else {
        hltJetsFromCont = SG::makeHandle(m_HLTJetsInKey);
        ANA_CHECK(hltJetsFromCont.isValid());
      }
    }

    Trig::FeatureRequestDescriptor frd;
    frd.setChainGroup(m_trigger);
    // prepare Run2 emulation results
    std::unordered_map<std::string, std::vector<std::pair<const xAOD::Jet*, bool>>> emulatedJets = {};
    if (m_doHLTMatching && m_useEmulationTool)
      emulatedJets = m_emulationTool->getEmulatedJets(m_trigger);
    bool isTrigPassed = m_trigDecisionTool->isPassed(m_trigger);
    const TrigConf::HLTChain* hltChain = m_trigDecisionTool->ExperimentalAndExpertMethods().getChainConfigurationDetails(m_trigger);
    const std::string& l1Name = hltChain->lower_chain_name();
    
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK(m_jetsHandle.retrieve(jets, sys));

      for (const xAOD::Jet *jet : *jets)
      {
	///////////////////////////
	//////  L1 matching  //////
	///////////////////////////

	if (m_doL1Matching){
	  const xAOD::JetRoI* bestL1 = nullptr;
	  float minDRL1 = m_dR.value();
	  std::set<int> L1Thresholds = {};

	  if (isTrigPassed) {

	    for (const auto l1_jet : *l1Jets) {
	      TLorentzVector l1_jet_p4;
              l1_jet_p4.SetPtEtaPhiM(l1_jet->et8x8(), l1_jet->eta(), l1_jet->phi(), 0.);
              float dR = jet->p4().DeltaR(l1_jet_p4);
	      if (dR < minDRL1) {
		minDRL1 = dR;
                bestL1 = l1_jet;

		std::vector<std::string> thrNames = l1_jet->thrNames();
                std::stringstream ss(std::regex_replace(l1Name, std::regex("-"), "_"));
                std::string legName;
                std::smatch match;
		// loop over legs
                while (getline(ss, legName, '_')) {
                  if (std::regex_match(legName, match, l1NameParser)) {
                    std::string legName_noMultiplicity = match[2].str() + match[3].str() + match[4].str();
                    int threshold = match[3].str()=="" ? 1 : std::stoi(match[3].str());
		    // compare with passed thresholds
                    for (const auto &thr : thrNames) {
                      if (thr == legName_noMultiplicity)
                        L1Thresholds.insert(threshold);
                    }
                  } // end of regex match
                } // loop over legs
	      }
	    } // loop over L1 jets
	  }

	  m_L1Et_decor.set(*jet, bestL1 ? bestL1->et8x8() : -99., sys);
	  m_L1Eta_decor.set(*jet, bestL1 ? bestL1->eta() : -99., sys);
	  m_L1Phi_decor.set(*jet, bestL1 ? bestL1->phi() : -99., sys);
	  m_L1DR_decor.set(*jet, minDRL1, sys);

	  std::vector<int> l1Thresh;
	  if(bestL1) l1Thresh = std::vector<int>(L1Thresholds.begin(), L1Thresholds.end());
	  m_L1Threshold_decor.set(*jet, l1Thresh, sys);
	} // end L1 matching

	///////////////////////////
	//////  HLT matching  /////
	///////////////////////////

	if (m_doHLTMatching){
	  const xAOD::IParticle* bestHLT = nullptr;
	  float minDRHLT = m_dR.value();
	  std::set<int> HLTThresholds = {};

	  if (isTrigPassed) {
	    int ileg = 0;
            for (const ChainNameParser::LegInfo &legInfo :
                 ChainNameParser::HLTChainInfo(m_trigger)) {
	      if (legInfo.signature == "j") {
                ANA_MSG_VERBOSE(" Leg" << ileg << ": "
				<< " " << legInfo.legName() << " "
				<< legInfo.type() << " " << legInfo.signature
				<< " " << legInfo.threshold);

		////////////////////////////
		////  Run 2 emulation  /////
		////////////////////////////
		
		if (m_useEmulationTool) {
                  auto hlt_emulated_jets = emulatedJets[legInfo.legName()]; // use pre-fetched emulation results
                  ANA_MSG_DEBUG(" Emulated jets for " << legInfo.legName() << ": " << hlt_emulated_jets.size());

                  for (const auto& [hlt_jet, passBtag]: hlt_emulated_jets) {
                    float dR = jet->p4().DeltaR(hlt_jet->p4());
                    ANA_MSG_VERBOSE("  pt: " << hlt_jet->pt()
				    << " eta: " << hlt_jet->eta()
				    << " phi: " << hlt_jet->phi()
				    << " dR: " << dR);

                    if (bestHLT && isSameJet(bestHLT, hlt_jet))
                      HLTThresholds.insert(legInfo.threshold);
                    else if (dR < minDRHLT) {
                      minDRHLT = dR;
                      bestHLT = hlt_jet;
                      HLTThresholds.clear();
                      HLTThresholds.insert(legInfo.threshold);
                    }
                  }
		}

		////////////////////////////
		////  Run 3 access  /////
		////////////////////////////

		else {
		  frd.setRestrictRequestToLeg(ileg);
                  auto hlt_jetsFromtrigDec = m_trigDecisionTool->features<xAOD::IParticleContainer>(frd);
                  std::vector<const xAOD::IParticle*> allHLTJets;

                  for (const auto& hlt_jet_link : hlt_jetsFromtrigDec) {
                    const xAOD::IParticle *hlt_jetFromtrigDec = *hlt_jet_link.link;
                    if (!hlt_jetFromtrigDec) continue;
                    allHLTJets.push_back(hlt_jetFromtrigDec);
                  }

                  // Start adding missing HLT jets -- only for buggy triggers
		  if (std::find(m_triggerNavBug.begin(),
				m_triggerNavBug.end(),
				m_trigger) != m_triggerNavBug.end()) {
                    for (const xAOD::Jet* jetFromCont : *hltJetsFromCont) {
                      bool alreadyIn = false;
                      for (const xAOD::IParticle* seenJet : allHLTJets) {
                        if (isSameJet(seenJet, jetFromCont)) {
                          alreadyIn = true;
                          break;
                        }
                      }
                      if (alreadyIn) continue;

                      allHLTJets.push_back(jetFromCont);
                      ANA_MSG_DEBUG("Added missing HLT jet from container: pt="
                                    << jetFromCont->pt() << " eta=" << jetFromCont->eta()
                                    << " phi=" << jetFromCont->phi());
                    }
		  }

                  for (const xAOD::IParticle* hlt_jet : allHLTJets) {
                    float dR = jet->p4().DeltaR(hlt_jet->p4());
                    bool fromtrigDec = false;
                    for (const auto& hlt_jet_link : hlt_jetsFromtrigDec) {
                      if (*hlt_jet_link.link == hlt_jet) {
                        fromtrigDec = true;
                        break;
                      }
                    }

                    ANA_MSG_VERBOSE("  pt: "
				    << hlt_jet->pt() << " eta: " << hlt_jet->eta()
				    << " phi: " << hlt_jet->phi() << " dR: " << dR
				    << " (fromContainer=" << !fromtrigDec << ")");

                    if (bestHLT && isSameJet(bestHLT, hlt_jet))
                      HLTThresholds.insert(legInfo.threshold);
                    else if (dR < minDRHLT)
                    {
                      minDRHLT = dR;
                      bestHLT = hlt_jet;
                      HLTThresholds.clear();
                      HLTThresholds.insert(legInfo.threshold);
                    }
                  } // Loop over allHLTJets
		} // end Run 3 access

		m_HLTPt_decor.set(*jet, bestHLT ? bestHLT->pt() : -99., sys);
		m_HLTEta_decor.set(*jet, bestHLT ? bestHLT->eta() : -99., sys);
		m_HLTPhi_decor.set(*jet, bestHLT ? bestHLT->phi() : -99., sys);
		m_HLTDR_decor.set(*jet, minDRHLT, sys);

		std::vector<int> hltThresh;
		if(bestHLT) hltThresh = std::vector<int>(HLTThresholds.begin(), HLTThresholds.end());
		m_HLTThreshold_decor.set(*jet, hltThresh, sys);

		ANA_MSG_VERBOSE("Summary " << " Trigger: " << m_trigger << " bestHLT pT: "
				<< (bestHLT ? bestHLT->pt() : -99.));
	      } // end HLT matching

	      ileg++;
	    }
	  }
	  
	}
      }
    };
    return StatusCode::SUCCESS;
  }


  bool JetTriggerDecoratorAlg::isSameJet(const xAOD::IParticle *jet1, const xAOD::IParticle *jet2) const
  {
    // Need this function because jet1 == jet2 would return false when comparing b-jet to untagged jet
    return (jet1->p4().DeltaR(jet2->p4()) < 0.01) && (std::abs(jet1->pt() - jet2->pt()) < 100);
  }

}
