/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler



//
// includes
//

#include <FTagAnalysisAlgorithms/BTaggingTriggerMatchingAlg.h>
#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigAnalysisHelpers/FeatureRequestDescriptor.h"
#include "xAODBTagging/BTagging.h"

//
// method implementations
//

namespace CP
{
  BTaggingTriggerMatchingAlg::BTaggingTriggerMatchingAlg
  (const std::string &name, ISvcLocator *svcLoc) :
    EL::AnaAlgorithm(name, svcLoc)
  {
  }

  StatusCode BTaggingTriggerMatchingAlg ::
  initialize ()
  {
    ANA_CHECK (m_trigDecTool.retrieve());
    ANA_CHECK(m_bjetInput.initialize());
    
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_matchingDecoration.initialize (m_systematicsList, m_jetHandle));
    ANA_CHECK (m_bTagMatchingDecoration.initialize (m_systematicsList, m_jetHandle));
    ANA_CHECK (m_systematicsList.initialize());
    if (!m_useRun3TriggerEDM.value()) {
        ANA_MSG_INFO("Using Run-2 trigger EDM for b-tagging trigger matching");
    } else {
        ANA_MSG_INFO("Using Run-3 trigger EDM for b-tagging trigger matching");
        m_ftagRun3TriggerDecorAccessors.clear();
        m_ftagRun3TriggerDecorAccessors.reserve(m_ftagRun3TriggerDecoNames.value().size());
        for (const auto& decoName : m_ftagRun3TriggerDecoNames.value()) {
            m_ftagRun3TriggerDecorAccessors.emplace_back(decoName);
        }
    }

    return StatusCode::SUCCESS;
  }



  StatusCode BTaggingTriggerMatchingAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    { 
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      std::map<const xAOD::Jet*, const xAOD::Jet*> matchedOfflineOnlineJets;
      SG::ReadHandle<xAOD::JetContainer> hlt_bjets(m_bjetInput);

      for (const xAOD::Jet* jet : *jets) {
        if (m_preselection.getBool(*jet, sys)) {
          float minDR = 0.4;
          const xAOD::Jet* bestHLTJet = nullptr;
          for (const xAOD::Jet* hlt_bjet : *hlt_bjets) {
            float dR = jet->p4().DeltaR(hlt_bjet->p4());
            if (dR < minDR) {
                minDR = dR;
                bestHLTJet = hlt_bjet;
            }
          }
          matchedOfflineOnlineJets[jet] = bestHLTJet;
        }
      }

      for (const xAOD::Jet *jet : *jets)
      {
        bool matched = false;
        bool passTrigger = false;

        if (m_preselection.getBool (*jet, sys))
          ATH_CHECK(passTriggerBtag(jet, matchedOfflineOnlineJets, passTrigger, matched));
        
        m_matchingDecoration.set (*jet, matched, sys);
        m_bTagMatchingDecoration.set (*jet, passTrigger, sys);
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode BTaggingTriggerMatchingAlg::passTriggerBtag
  (const xAOD::Jet* jet,
   const std::map<const xAOD::Jet*, const xAOD::Jet*>& matchedOfflineOnlineJets,
   bool& btag, bool& matched) const{
    btag = false;
    matched = false;
    if(!m_trigDecTool->isPassed(m_trigger)){
      // No further check, btag will be false
      return StatusCode::SUCCESS;
    }

    Trig::FeatureRequestDescriptor frd;
    frd.setChainGroup(m_trigger);

    int ileg = 0;
    const xAOD::IParticle* bestHLT = nullptr;
    float minDRHLT = 0.4; // hard-coded matching distance

    for (const ChainNameParser::LegInfo& legInfo :
	   ChainNameParser::HLTChainInfo(m_trigger)){
      if (legInfo.signature == "j"){
        ATH_MSG_VERBOSE(" Leg" << ileg << ": "
          << " " << legInfo.legName() << " "
          << legInfo.type() << " " << legInfo.signature
          << " " << legInfo.threshold);
          
        frd.setRestrictRequestToLeg(ileg);
        auto hlt_jets = m_trigDecTool->features<xAOD::IParticleContainer>(frd);
        auto mapjet = matchedOfflineOnlineJets.find(jet);
        if (mapjet != matchedOfflineOnlineJets.end() && mapjet->second){
          auto hlt_bjet =  mapjet->second;
          if (hlt_bjet->pt() > legInfo.threshold &&  abs( hlt_bjet->eta()) < m_etamax.value()){
            matched = true;
          }
        }

	for (const auto& hlt_jet_link : hlt_jets){
	  const xAOD::IParticle *hlt_jet = *hlt_jet_link.link;
	  float dR = jet->p4().DeltaR(hlt_jet->p4());
	  bool hasBtag = false;
	  if(m_useRun3TriggerEDM){
      // we need to access via the trigger decision tool
      
	    bool hasBtagLink = hlt_jet_link.source->hasObjectLink("btag");
        // in later Run-3 releases, the btag link may not be present if the online btagging decorations
        // were added to the jets directly
        bool hasBtagDeco = false;
        if(!hasBtagLink){
            ATH_MSG_VERBOSE("No btag' link found on HLT jet, checking for Run-3 trigger decorations on jet");
            ATH_CHECK(hasBTagDeco(hlt_jet, hasBtagDeco));
        }
        hasBtag = hasBtagLink || hasBtagDeco;
	  }
	  else{
	    double hlt_bscore = -1.;
	    ATH_CHECK(getBtagScore(hlt_jet, hlt_bscore));
	    hasBtag = hlt_bscore > m_btagThreshold;
	  }

	  ATH_MSG_VERBOSE("  pt: "
			  << hlt_jet->pt() << " eta: " << hlt_jet->eta()
			  << " phi: " << hlt_jet->phi() << " dR: " << dR
			  << " btag: " << hasBtag);

	  if (bestHLT && isSameJet(bestHLT, hlt_jet))
	    btag = btag || hasBtag; // if any leg claims b-tag, then the jet is b-tagged
	  else if (dR < minDRHLT) {
	    minDRHLT = dR;
	    bestHLT = hlt_jet;
	    btag = hasBtag;
	  }
	}
      }

      ATH_MSG_VERBOSE(" =dRHLT: " << minDRHLT << " bestHLT pT: "
		      << (bestHLT ? bestHLT->pt() : -99.)
		      << " btag: " << btag);

      ileg++;
    }
    return StatusCode::SUCCESS;
  }

  bool BTaggingTriggerMatchingAlg::isSameJet(const xAOD::IParticle *jet1,
					       const xAOD::IParticle *jet2) const{
    // Need this function because jet1 == jet2 would return false
    // when comparing b-jet to untagged jet
    return (jet1->p4().DeltaR(jet2->p4()) < 0.01) &&
      (std::abs(jet1->pt() - jet2->pt()) < 100);
  }

  StatusCode BTaggingTriggerMatchingAlg::getBtagScore(const xAOD::IParticle *jet, double& hlt_bscore) const {
    SG::ConstAccessor<const xAOD::BTagging*> acc("HLTBTag");
    const xAOD::BTagging* tagInfo = acc(*jet);

    if(m_trigger.value().find("mv2c20") != std::string::npos){
      if(!tagInfo->MVx_discriminant("MV2c20", hlt_bscore)){
	ATH_MSG_ERROR("MV2c20 discriminant not accessible");
	return StatusCode::FAILURE;
      }
    }
    else if(m_trigger.value().find("mv2c10") != std::string::npos){
      if(!tagInfo->MVx_discriminant("MV2c10", hlt_bscore)){
	ATH_MSG_ERROR("MV2c10 discriminant not accessible");
	return StatusCode::FAILURE;
      }
    }
    else{
      double w1 = tagInfo->IP3D_pb() / tagInfo->IP3D_pu();
      double w2 = tagInfo->SV1_pb() / tagInfo->SV1_pu();
      double W = w1 * w2;
      if ( W/(1.0+W) < 1.0 )
	hlt_bscore = -1.0 * std::log10(1.0 - ( W / ( 1.0 + W ) ) );
      else hlt_bscore = 50;
    }
    return StatusCode::SUCCESS;
  }

  StatusCode BTaggingTriggerMatchingAlg::hasBTagDeco(const xAOD::IParticle* jet, bool& hasBtagDeco) const {
    hasBtagDeco = false;
    for (const auto& accessor : m_ftagRun3TriggerDecorAccessors) {
        if (accessor.isAvailable(*jet)) {
            hasBtagDeco = true;
            break;
        }
    }
    return StatusCode::SUCCESS;
  }

}
