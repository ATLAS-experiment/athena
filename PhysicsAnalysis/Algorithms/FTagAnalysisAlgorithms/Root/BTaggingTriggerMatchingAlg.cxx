/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler



//
// includes
//

#include <FTagAnalysisAlgorithms/BTaggingTriggerMatchingAlg.h>
#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigAnalysisHelpers/FeatureRequestDescriptor.h"
#include "xAODBTagging/BTagging.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace
{
  /// leg part with an online b-tagging requirement, e.g. "bmv2c2060" but not "boffperf"
  bool isOnlineBTagPart(const std::string& part)
  {
    return part.starts_with('b') && part != "boffperf" && part != "bperf";
  }

  /// Run 2 GSC threshold leg part, e.g. "gsc55"
  bool isGSCPart(const std::string& part)
  {
    return part.size() > 3 && part.starts_with("gsc") &&
      std::all_of(part.begin() + 3, part.end(),
                  [](unsigned char c) { return std::isdigit(c); });
  }

  /// maximum distance between two copies of the same HLT jet, as used by
  /// the b-jet trigger emulation to associate the Run 2 HLT jet collections
  constexpr float sameHLTJetDR = 0.05;
}

//
// method implementations
//

namespace CP
{
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
        ANA_CHECK(m_emulationTool.retrieve());
    } else {
        ANA_MSG_INFO("Using Run-3 trigger EDM for b-tagging trigger matching");

#ifndef XAOD_STANDALONE
        ANA_MSG_DEBUG("Disabling unused emulation tool");
        ATH_CHECK(m_emulationTool.retrieve( DisableTool{true} ));
#endif

        m_ftagRun3TriggerDecorAccessors.clear();
        m_ftagRun3TriggerDecorAccessors.reserve(m_ftagRun3TriggerDecoNames.value().size());
        for (const auto& decoName : m_ftagRun3TriggerDecoNames.value()) {
            m_ftagRun3TriggerDecorAccessors.emplace_back(decoName);
        }

    }

    // the jet legs of the chain do not change from event to event
    int ileg = 0;
    for (const ChainNameParser::LegInfo& legInfo :
           ChainNameParser::HLTChainInfo(m_trigger.value())) {
      if (legInfo.signature == "j") {
        JetLegInfo leg;
        leg.index = ileg;
        leg.name = legInfo.legName();
        leg.threshold = legInfo.threshold;
        for (const std::string& part : legInfo.legParts) {
          if (isOnlineBTagPart(part))
            leg.isBTagged = true;
          // the threshold of Run 2 GSC legs applies to the GSC-calibrated jets
          else if (!m_useRun3TriggerEDM.value() && isGSCPart(part))
            leg.threshold = std::stoi(part.substr(3));
        }
        ANA_MSG_DEBUG("Jet leg " << leg.index << ": " << leg.name
                      << " threshold: " << leg.threshold << " GeV"
                      << " b-tagged: " << leg.isBTagged);
        m_jetLegs.push_back(std::move(leg));
      }
      ileg++;
    }

    if (!m_useRun3TriggerEDM.value() &&
        std::none_of(m_jetLegs.begin(), m_jetLegs.end(),
                     [](const JetLegInfo& leg) { return leg.isBTagged; })) {
      ANA_MSG_WARNING("No b-tagged jet leg found in " << m_trigger.value()
                      << ": no jet will pass the online b-tagging");
    }

    return StatusCode::SUCCESS;
  }



  StatusCode BTaggingTriggerMatchingAlg ::
  execute (const EventContext& ctx)
  {

    // the trigger decision and the HLT jets do not depend on the offline jets;
    // no jet is matched if the chain did not fire
    const bool isTrigPassed = m_trigDecTool->isPassed(m_trigger.value());

    // Run 3: HLT jets of each leg from the trigger navigation
    std::vector<HLTJetLeg> jetLegs;
    if (m_useRun3TriggerEDM && isTrigPassed) {
      Trig::FeatureRequestDescriptor frd;
      frd.setChainGroup(m_trigger.value());

      for (const JetLegInfo& leg : m_jetLegs) {
        ATH_MSG_VERBOSE(" Leg" << leg.index << ": " << leg.name
                        << " " << leg.threshold);

        frd.setRestrictRequestToLeg(leg.index);
        jetLegs.push_back({leg.threshold,
                           m_trigDecTool->features<xAOD::IParticleContainer>(frd)});
      }
    }

    // Run 2: the trigger navigation is not available, the b-tagged legs are emulated
    const xAOD::JetContainer* hltJetsRun2 = nullptr;
    std::vector<const xAOD::Jet*> hltBTaggedJets;
    if (!m_useRun3TriggerEDM && isTrigPassed) {
      SG::ReadHandle<xAOD::JetContainer> hltJets(m_bjetInput, ctx);
      if (!hltJets.isValid()) {
        ANA_MSG_ERROR ("Failed to retrieve HLT jet container " << m_bjetInput.key());
        return StatusCode::FAILURE;
      }
      hltJetsRun2 = hltJets.cptr();

      const auto emulatedJets = m_emulationTool->getEmulatedJets(m_trigger.value());
      for (const JetLegInfo& leg : m_jetLegs) {
        if (!leg.isBTagged)
          continue;
        const auto emulated = emulatedJets.find(leg.name);
        if (emulated == emulatedJets.end()) {
          ANA_MSG_DEBUG(" No emulated jets for " << leg.name);
          continue;
        }
        ANA_MSG_DEBUG(" Emulated jets for " << leg.name << ": " << emulated->second.size());
        // the emulated jets of a b-tagged leg passed its online b-tagging requirement
        for (const auto& [hlt_jet, passBtag] : emulated->second) {
          ATH_MSG_VERBOSE("  pt: " << hlt_jet->pt()
                          << " eta: " << hlt_jet->eta()
                          << " phi: " << hlt_jet->phi()
                          << " passBtag: " << passBtag);
          if (passBtag)
            hltBTaggedJets.push_back(hlt_jet);
        }
      }
      ATH_MSG_VERBOSE("Num HLT-jets: " << hltJetsRun2->size()
                      << " passing the online b-tagging: " << hltBTaggedJets.size());
    }

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys, ctx));

      if(m_useRun3TriggerEDM){
        std::unordered_map<const xAOD::Jet*, const xAOD::Jet*> matchedOfflineOnlineJets;
        SG::ReadHandle<xAOD::JetContainer> hlt_bjets(m_bjetInput, ctx);
        if (!hlt_bjets.isValid()) {
          ANA_MSG_ERROR ("Failed to retrieve HLT b-jet container " << m_bjetInput.key());
          return StatusCode::FAILURE;
        }

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
            ATH_CHECK(passTriggerBtag(jet, matchedOfflineOnlineJets, jetLegs, passTrigger, matched));

          m_matchingDecoration.set (*jet, matched, sys);
          m_bTagMatchingDecoration.set (*jet, passTrigger, sys);
        }
      } else {
        for (const xAOD::Jet* jet : *jets) {
          bool bestHLTJetBTagPass = false;
          bool bestHLTJetMatched = false;
          float minDR = 0.4;
          const xAOD::Jet* bestHLTJet = nullptr;
          if (hltJetsRun2 && m_preselection.getBool(*jet, sys)) {
            // closest HLT jet on which the online b-tagging ran
            for (const xAOD::Jet* hlt_jet : *hltJetsRun2) {
              float dR = jet->p4().DeltaR(hlt_jet->p4());
              if (dR < minDR) {
                minDR = dR;
                bestHLTJet = hlt_jet;
              }
            }
          }
          if (bestHLTJet) {
            // the trigger looked at this jet if it passes the cuts of one of the jet legs;
            // the Run 2 leg thresholds from the chain name are on the transverse energy, in GeV
            if (std::abs(bestHLTJet->eta()) < m_etamax.value()) {
              for (const JetLegInfo& leg : m_jetLegs) {
                if (bestHLTJet->p4().Et() > 1e3 * leg.threshold) {
                  bestHLTJetMatched = true;
                  break;
                }
              }
            }
            // the jet passed the online b-tagging if it passed one of the b-tagged legs;
            // compare kinematics, as the container may hold several copies of the same HLT jet
            for (const xAOD::Jet* hlt_bjet : hltBTaggedJets) {
              if (bestHLTJet->p4().DeltaR(hlt_bjet->p4()) < sameHLTJetDR) {
                bestHLTJetBTagPass = true;
                break;
              }
            }
          }
          ATH_MSG_VERBOSE(" =dRHLT: " << minDR << " bestHLT pT: "
              << (bestHLTJet ? bestHLTJet->pt() : -99.)
              << " btag: " << bestHLTJetBTagPass << " matched: " << bestHLTJetMatched);

          m_matchingDecoration.set (*jet, bestHLTJetMatched, sys);
          m_bTagMatchingDecoration.set (*jet, bestHLTJetBTagPass, sys);

        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode BTaggingTriggerMatchingAlg::passTriggerBtag
  (const xAOD::Jet* jet,
   const std::unordered_map<const xAOD::Jet*, const xAOD::Jet*>& matchedOfflineOnlineJets,
   const std::vector<HLTJetLeg>& jetLegs,
   bool& btag, bool& matched) const{
    btag = false;
    matched = false;

    const xAOD::IParticle* bestHLT = nullptr;
    float minDRHLT = 0.4; // hard-coded matching distance

    for (const HLTJetLeg& jetLeg : jetLegs){
      auto mapjet = matchedOfflineOnlineJets.find(jet);
      if (mapjet != matchedOfflineOnlineJets.end() && mapjet->second){
        auto hlt_bjet =  mapjet->second;
        // the leg threshold from the chain name is in GeV
        if (hlt_bjet->pt() > 1e3 * jetLeg.threshold && std::abs( hlt_bjet->eta()) < m_etamax.value()){
          matched = true;
        }
      }

      for (const auto& hlt_jet_link : jetLeg.features){
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

      ATH_MSG_VERBOSE(" =dRHLT: " << minDRHLT << " bestHLT pT: "
		      << (bestHLT ? bestHLT->pt() : -99.)
		      << " btag: " << btag << " matched: " << matched);

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
    if(!acc.isAvailable(*jet) || !acc(*jet)){
      ATH_MSG_ERROR("HLTBTag object not accessible on HLT jet");
      return StatusCode::FAILURE;
    }
    const xAOD::BTagging* tagInfo = acc(*jet);

    if(m_trigger.value().contains("mv2c20")){
      if(!tagInfo->MVx_discriminant("MV2c20", hlt_bscore)){
	ATH_MSG_ERROR("MV2c20 discriminant not accessible");
	return StatusCode::FAILURE;
      }
    }
    else if(m_trigger.value().contains("mv2c10")){
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
