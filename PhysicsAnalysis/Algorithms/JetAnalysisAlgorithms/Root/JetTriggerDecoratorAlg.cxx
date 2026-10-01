/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetAnalysisAlgorithms/JetTriggerDecoratorAlg.h"

#include <algorithm>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigConfHLTData/HLTChain.h"

namespace
{

// Returns the L1 RoI ET for Run-2 and Run-3 EDMs respectively.
inline float
getL1JetEt(const xAOD::JetRoI* roi)
{
  return roi->et8x8();
}

inline float
getL1JetEt(const xAOD::jFexSRJetRoI* roi)
{
  return static_cast<float>(roi->et());
}

// Returns the list of fired threshold names for a Run-2 L1 RoI.
inline const std::vector<std::string>&
getL1JetThresholds(const xAOD::JetRoI* roi)
{
  return roi->thrNames();
}

// Parsed L1 leg token: the threshold-name (without leg multiplicity, e.g.
// "jJ50") and its integer threshold. The chain L1 lower-chain name is
// event-constant, so it is parsed once per event (see parseL1LegTokens)
// rather than re-parsed for every candidate RoI.
struct L1LegToken {
  std::string name;  // legName_noMultiplicity, e.g. "jJ50"
  int threshold = 0;
};

// Parse the L1 lower-chain name into (name, threshold) leg tokens. Splits
// on '_' (after mapping '-' -> '_') and applies the fixed leg-name regex.
// Constructed once per event; the regex itself is a function-local static
// const so it is compiled exactly once for the whole job.
inline std::vector<L1LegToken> parseL1LegTokens(const std::string& l1Name) {
  static const std::regex l1NameParser(
      "(\\d*)(j?J)(\\d*)((p|\\.)(\\d*)ETA(\\d*))?");
  static const std::regex dashToUnderscore("-");

  std::vector<L1LegToken> tokens;
  std::stringstream ss(std::regex_replace(l1Name, dashToUnderscore, "_"));
  std::string legName;
  std::smatch match;
  while (getline(ss, legName, '_')) {
    if (std::regex_match(legName, match, l1NameParser)) {
      std::string legName_noMultiplicity =
          match[2].str() + match[3].str() + match[4].str();
      int threshold = match[3].str().empty() ? 1 : std::stoi(match[3].str());
      tokens.push_back({std::move(legName_noMultiplicity), threshold});
    }
  }
  return tokens;
}

template <typename RoI, typename Container, typename ThrAccessor>
std::tuple<float, float, float, float, std::vector<int>> matchL1Container(
    const TLorentzVector& jetP4, const Container& container,
    ThrAccessor thrAccessor, const std::vector<L1LegToken>& l1LegTokens,
    float drMax) {
  const RoI* bestL1 = nullptr;
  float minDRL1 = drMax;
  std::set<int> L1Thresholds;

  for (const RoI* l1_jet : container) {
    TLorentzVector l1_jet_p4;
    l1_jet_p4.SetPtEtaPhiM(
        getL1JetEt(l1_jet), l1_jet->eta(), l1_jet->phi(), 0.);
    const float dR = static_cast<float>(jetP4.DeltaR(l1_jet_p4));
    if (dR < minDRL1) {
      minDRL1 = dR;
      bestL1 = l1_jet;
    }
  }

  // Only the finally-selected RoI's thresholds are reported.
  if (bestL1) {
    const auto& thrNames = thrAccessor(bestL1);
    for (const L1LegToken& tok : l1LegTokens) {
      for (const auto& thr : thrNames) {
        if (thr == tok.name) {
          L1Thresholds.insert(tok.threshold);
        }
      }
    }
  }

  return {
    bestL1 ? getL1JetEt(bestL1) : -99.f,
    bestL1 ? bestL1->eta() : -99.f,
    bestL1 ? bestL1->phi() : -99.f,
    minDRL1,
    bestL1 ? std::vector<int>(L1Thresholds.begin(), L1Thresholds.end())
           : std::vector<int>()
  };
}

}  // anonymous namespace


namespace CP
{
  StatusCode JetTriggerDecoratorAlg::initialize() {
    ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));

    ANA_CHECK(m_L1JetsInKey.initialize(m_doL1Matching && !m_usePhaseIL1));
    ANA_CHECK(m_L1JetsPhaseIInKey.initialize(m_doL1Matching && m_usePhaseIL1));
    ANA_CHECK(m_HLTJetsInKey.initialize(
        m_doHLTMatching && !m_useEmulationTool));

    ANA_CHECK(m_trigDecisionTool.retrieve());
    if(m_useEmulationTool) ANA_CHECK(m_emulationTool.retrieve());

    if (m_doL1Matching && m_usePhaseIL1) {
      ANA_CHECK(m_trigConfigTool.retrieve());
    }

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

      // The chain is job-constant: parse its "j" legs once.
      int ileg = 0;
      for (const ChainNameParser::LegInfo& legInfo :
           ChainNameParser::HLTChainInfo(m_trigger.value())) {
        if (legInfo.signature == "j") {
          ANA_MSG_VERBOSE(" Leg" << ileg << ": "
                                 << " " << legInfo.legName() << " "
                                 << legInfo.type() << " "
                                 << legInfo.signature << " "
                                 << legInfo.threshold);

          int legThreshold = legInfo.threshold;

          if (legInfo.legName().find("gsc") != std::string::npos) {
            for (const std::string& part : legInfo.legParts) {
              if (part.find("gsc") != std::string::npos) {
                legThreshold = std::stoi(part.substr(3));
                ATH_MSG_DEBUG("GSC leg found. Using threshold "
                              << legThreshold);
                break;
              }
            }
          }

          m_jetLegs.push_back({ileg, legThreshold, legInfo.legName()});
        }
        ileg++;
      }

      m_isNavBugTrigger =
          std::find(m_triggerNavBug.begin(), m_triggerNavBug.end(),
                    m_trigger.value()) != m_triggerNavBug.end();
    }

    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode JetTriggerDecoratorAlg::execute(const EventContext& ctx) {
    SG::ReadHandle<xAOD::JetRoIContainer> l1Jets;
    SG::ReadHandle<xAOD::jFexSRJetRoIContainer> l1JetsPhaseI;
    if (m_doL1Matching) {
      if (m_usePhaseIL1) {
        ANA_CHECK(m_jfexThresholdTable.update(m_trigConfigTool,
                                              m_l1ThresholdType.value(), ctx,
                                              msg()));
        l1JetsPhaseI = SG::makeHandle(m_L1JetsPhaseIInKey, ctx);
        ANA_CHECK(l1JetsPhaseI.isValid());
      } else {
        l1Jets = SG::makeHandle(m_L1JetsInKey, ctx);
        ANA_CHECK(l1Jets.isValid());
      }
    }
    SG::ReadHandle<xAOD::JetContainer> hltJetsFromCont;
    if (m_doHLTMatching) {
      if (m_useEmulationTool)
        ANA_MSG_DEBUG(m_trigger << " isPassed "
                                << m_emulationTool->isPassed(m_trigger.value()));
      else {
        hltJetsFromCont = SG::makeHandle(m_HLTJetsInKey, ctx);
        ANA_CHECK(hltJetsFromCont.isValid());
      }
    }

    Trig::FeatureRequestDescriptor frd;
    frd.setChainGroup(m_trigger.value());
    // prepare Run2 emulation results
    std::unordered_map<std::string,
                       std::vector<std::pair<const xAOD::Jet*, bool>>>
        emulatedJets = {};
    if (m_doHLTMatching && m_useEmulationTool)
      emulatedJets = m_emulationTool->getEmulatedJets(m_trigger);
    bool isTrigPassed = m_trigDecisionTool->isPassed(m_trigger.value());
    const TrigConf::HLTChain* hltChain =
        m_trigDecisionTool->ExperimentalAndExpertMethods()
            .getChainConfigurationDetails(m_trigger);
    std::vector<L1LegToken> l1LegTokens;
    if (hltChain) {
      l1LegTokens = parseL1LegTokens(hltChain->lower_chain_name());
    } else {
      // Chain not in this file's menu (e.g. a chain of another year):
      // it cannot have fired, so only the default decorations are written.
      if (!m_warnedMissingChain) {
        ANA_MSG_WARNING("Trigger chain " << m_trigger.value()
                        << " not found in the trigger menu; writing default "
                           "matching decorations (warning printed once)");
        m_warnedMissingChain = true;
      }
      isTrigPassed = false;
    }

    // HLT candidates of each "j" leg (aligned with m_jetLegs): they depend
    // only on the event, so are collected once before the jet loops. The
    // first nNavCandidates[i] of them come from the trigger navigation (or
    // the Run 2 emulation), the rest from the container (nav-bug triggers).
    std::vector<std::vector<const xAOD::IParticle*>> hltCandidates(
        m_jetLegs.size());
    std::vector<std::size_t> nNavCandidates(m_jetLegs.size(), 0);
    if (m_doHLTMatching && isTrigPassed) {
      for (std::size_t i = 0; i < m_jetLegs.size(); ++i) {
        const JetLeg& leg = m_jetLegs[i];
        std::vector<const xAOD::IParticle*>& candidates = hltCandidates[i];

        ////////////////////////////
        ////  Run 2 emulation  /////
        ////////////////////////////

        if (m_useEmulationTool) {
          const auto emulated = emulatedJets.find(leg.name);
          if (emulated != emulatedJets.end()) {
            candidates.reserve(emulated->second.size());
            for (const auto& jetAndBtag : emulated->second)
              candidates.push_back(jetAndBtag.first);
          }
          ANA_MSG_DEBUG(" Emulated jets for " << leg.name << ": "
                        << candidates.size());
          nNavCandidates[i] = candidates.size();
        }

        ////////////////////////////
        ////  Run 3 access  /////
        ////////////////////////////

        else {
          frd.setRestrictRequestToLeg(leg.index);
          const auto hlt_jetsFromtrigDec =
              m_trigDecisionTool->features<xAOD::IParticleContainer>(frd);

          for (const auto& hlt_jet_link : hlt_jetsFromtrigDec) {
            const xAOD::IParticle* hlt_jetFromtrigDec = *hlt_jet_link.link;
            if (!hlt_jetFromtrigDec)
              continue;
            candidates.push_back(hlt_jetFromtrigDec);
          }
          nNavCandidates[i] = candidates.size();

          // Start adding missing HLT jets -- only for buggy triggers
          if (m_isNavBugTrigger) {
            for (const xAOD::Jet* jetFromCont : *hltJetsFromCont) {
              bool alreadyIn = false;
              for (const xAOD::IParticle* seenJet : candidates) {
                if (isSameJet(seenJet, jetFromCont)) {
                  alreadyIn = true;
                  break;
                }
              }
              if (alreadyIn)
                continue;

              candidates.push_back(jetFromCont);
              ANA_MSG_DEBUG("Added missing HLT jet from container: pt="
                            << jetFromCont->pt()
                            << " eta=" << jetFromCont->eta()
                            << " phi=" << jetFromCont->phi());
            }
          }
        }  // end Run 3 access
      }
    }

    for (const auto& sys : m_systematicsList.systematicsVector()) {
      const xAOD::JetContainer* jets = nullptr;
      ANA_CHECK(m_jetsHandle.retrieve(jets, sys, ctx));

      for (const xAOD::Jet* jet : *jets) {
        const TLorentzVector jetP4 = jet->p4();

        ///////////////////////////
        //////  L1 matching  //////
        ///////////////////////////

        if (m_doL1Matching) {
          float l1Et = -99.f;
          float l1Eta = -99.f;
          float l1Phi = -99.f;
          float minDRL1 = m_l1dR.value();
          std::vector<int> l1ThresholdsVec;

          if (isTrigPassed) {
            if (m_usePhaseIL1) {
              const JfexThresholdTable& jfexTable = m_jfexThresholdTable;
              std::tie(l1Et, l1Eta, l1Phi, minDRL1, l1ThresholdsVec) =
                  matchL1Container<xAOD::jFexSRJetRoI>(
                      jetP4, *l1JetsPhaseI,
                      [&jfexTable](const xAOD::jFexSRJetRoI* r) {
                        return jfexTable.decode(*r);
                      },
                      l1LegTokens, m_l1dR.value());
            } else {
              // Legacy L1Calo path: use JetRoI::thrNames().
              std::tie(l1Et, l1Eta, l1Phi, minDRL1, l1ThresholdsVec) =
                  matchL1Container<xAOD::JetRoI>(
                      jetP4, *l1Jets,
                      [](const xAOD::JetRoI* r)
                          -> const std::vector<std::string>& {
                        return getL1JetThresholds(r);
                      },
                      l1LegTokens, m_l1dR.value());
            }
          }

          m_L1Et_decor.set(*jet, l1Et, sys);
          m_L1Eta_decor.set(*jet, l1Eta, sys);
          m_L1Phi_decor.set(*jet, l1Phi, sys);
          m_L1DR_decor.set(*jet, minDRL1, sys);
          m_L1Threshold_decor.set(*jet, l1ThresholdsVec, sys);
        }  // end L1 matching

        ///////////////////////////
        //////  HLT matching  /////
        ///////////////////////////

        if (m_doHLTMatching) {
          const xAOD::IParticle* bestHLT = nullptr;
          float minDRHLT = m_hltDR.value();
          std::set<int> HLTThresholds = {};

          for (std::size_t i = 0; i < m_jetLegs.size(); ++i) {
            const int legThreshold = m_jetLegs[i].threshold;
            const std::vector<const xAOD::IParticle*>& candidates =
                hltCandidates[i];

            for (std::size_t j = 0; j < candidates.size(); ++j) {
              const xAOD::IParticle* hlt_jet = candidates[j];
              float dR = jetP4.DeltaR(hlt_jet->p4());

              ANA_MSG_VERBOSE(
                  "  pt: " << hlt_jet->pt() << " eta: " << hlt_jet->eta()
                           << " phi: " << hlt_jet->phi() << " dR: " << dR
                           << " (fromContainer=" << (j >= nNavCandidates[i])
                           << ")");

              if (bestHLT && isSameJet(bestHLT, hlt_jet))
                HLTThresholds.insert(legThreshold);
              else if (dR < minDRHLT) {
                minDRHLT = dR;
                bestHLT = hlt_jet;
                HLTThresholds.clear();
                HLTThresholds.insert(legThreshold);
              }
            }  // Loop over the leg's HLT candidates
          }

	  m_HLTPt_decor.set(*jet, bestHLT ? bestHLT->pt() : -99., sys);
	  m_HLTEta_decor.set(*jet, bestHLT ? bestHLT->eta() : -99., sys);
	  m_HLTPhi_decor.set(*jet, bestHLT ? bestHLT->phi() : -99., sys);
	  m_HLTDR_decor.set(*jet, minDRHLT, sys);

	  std::vector<int> hltThresh;
	  if (bestHLT)
	    hltThresh = std::vector<int>(HLTThresholds.begin(),
					 HLTThresholds.end());
	  m_HLTThreshold_decor.set(*jet, hltThresh, sys);

	  ANA_MSG_VERBOSE("Summary "
			  << " Trigger: " << m_trigger << " bestHLT pT: "
			  << (bestHLT ? bestHLT->pt() : -99.));
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
