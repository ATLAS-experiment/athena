/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetAnalysisAlgorithms/JetTriggerDecoratorAlg.h"

#include <AthContainers/ConstAccessor.h>
#include <xAODTrigger/jFexSRJetRoIContainer.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigConfData/L1Menu.h"
#include "TrigConfData/L1Threshold.h"
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
inline std::vector<std::string>
getL1JetThresholds(const xAOD::JetRoI* roi)
{
  return roi->thrNames();
}

inline std::vector<std::string>
getL1JetThresholds(const xAOD::jFexSRJetRoI* roi,
                   const std::vector<std::string>& bitToName)
{
  std::vector<std::string> passed;
  static const SG::AuxElement::ConstAccessor<uint64_t>
      thrPatternsAcc("thresholdPatterns");
  if (!thrPatternsAcc.isAvailable(*roi)) return passed;
  const uint64_t pat = thrPatternsAcc(*roi);
  passed.reserve(bitToName.size());
  for (size_t b = 0; b < bitToName.size(); ++b) {
    if (((pat >> b) & 1ULL) && !bitToName[b].empty()) {
      passed.push_back(bitToName[b]);
    }
  }
  return passed;
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
      tokens.push_back({legName_noMultiplicity, threshold});
    }
  }
  return tokens;
}

template <typename RoI, typename Container, typename ThrAccessor>
std::tuple<float, float, float, float, std::vector<int>> matchL1Container(
    const xAOD::Jet* jet, const Container& container, ThrAccessor thrAccessor,
    const std::vector<L1LegToken>& l1LegTokens, float drMax) {
  const RoI* bestL1 = nullptr;
  float minDRL1 = drMax;
  std::set<int> L1Thresholds;

  for (const RoI* l1_jet : container) {
    TLorentzVector l1_jet_p4;
    l1_jet_p4.SetPtEtaPhiM(
        getL1JetEt(l1_jet), l1_jet->eta(), l1_jet->phi(), 0.);
    const float dR = static_cast<float>(jet->p4().DeltaR(l1_jet_p4));
    if (dR < minDRL1) {
      minDRL1 = dR;
      bestL1 = l1_jet;
      // Reset so only the finally-selected RoI's thresholds are reported.
      L1Thresholds.clear();
      const std::vector<std::string> thrNames = thrAccessor(l1_jet);
      for (const L1LegToken& tok : l1LegTokens) {
        for (const auto& thr : thrNames) {
          if (thr == tok.name) {
            L1Thresholds.insert(tok.threshold);
          }
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
JetTriggerDecoratorAlg::JetTriggerDecoratorAlg(const std::string& name,
                                               ISvcLocator* svcLoc)
    : EL::AnaAlgorithm(name, svcLoc) {}

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
    }

    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode JetTriggerDecoratorAlg::rebuildJfexThresholdTable(
      const EventContext& ctx) {
    const TrigConf::L1Menu* l1menu = nullptr;
    try {
      l1menu = &m_trigConfigTool->l1Menu(ctx);
    } catch (const std::exception& e) {
      ANA_MSG_ERROR("Could not read the L1 menu in execute(): "
                    << e.what()
                    << ". The Phase-I jFEX threshold table "
                       "cannot be built. Ensure TrigConf::xAODConfigSvc has "
                       "loaded the L1 menu for this input file.");
      return StatusCode::FAILURE;
    }

    if (m_thresholdNamesLoaded && l1menu->name() == m_cachedL1MenuName) {
      return StatusCode::SUCCESS;
    }
    if (m_thresholdNamesLoaded) {
      ANA_MSG_INFO("L1 menu changed from '"
                   << m_cachedL1MenuName << "' to '" << l1menu->name()
                   << "' — rebuilding jFEX threshold table");
    }
    const auto& thresholds = l1menu->thresholds(m_l1ThresholdType.value());
    if (thresholds.empty()) {
      ANA_MSG_ERROR("L1 menu '" << l1menu->name()
                                << "' has no thresholds of type '"
                                << m_l1ThresholdType.value() << "'");
      return StatusCode::FAILURE;
    }
    m_jfexThresholdNames.clear();
    for (const auto& thr : thresholds) {
      const unsigned int bit = thr->mapping();
      if (bit >= m_jfexThresholdNames.size())
      m_jfexThresholdNames.resize(bit + 1);
      m_jfexThresholdNames[bit] = thr->name();
    }
    ANA_MSG_INFO("Loaded " << m_jfexThresholdNames.size()
                           << " jFEX threshold names from L1 menu '"
                           << l1menu->name() << "'");
    m_cachedL1MenuName = l1menu->name();
    m_thresholdNamesLoaded = true;
    return StatusCode::SUCCESS;
  }

  StatusCode JetTriggerDecoratorAlg::execute(const EventContext& ctx) {
    SG::ReadHandle<xAOD::JetRoIContainer> l1Jets;
    SG::ReadHandle<xAOD::jFexSRJetRoIContainer> l1JetsPhaseI;
    if (m_doL1Matching) {
      if (m_usePhaseIL1) {
        ANA_CHECK(rebuildJfexThresholdTable(ctx));
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
    const std::string& l1Name = hltChain->lower_chain_name();
    const std::vector<L1LegToken> l1LegTokens = parseL1LegTokens(l1Name);

    for (const auto& sys : m_systematicsList.systematicsVector()) {
      const xAOD::JetContainer* jets = nullptr;
      ANA_CHECK(m_jetsHandle.retrieve(jets, sys, ctx));

      for (const xAOD::Jet* jet : *jets) {
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
              const auto& jfexNames = m_jfexThresholdNames;
              std::tie(l1Et, l1Eta, l1Phi, minDRL1, l1ThresholdsVec) =
                  matchL1Container<xAOD::jFexSRJetRoI>(
                      jet, *l1JetsPhaseI,
                      [&jfexNames](const xAOD::jFexSRJetRoI* r) {
                        return getL1JetThresholds(r, jfexNames);
                      },
                      l1LegTokens, m_l1dR.value());
            } else {
              // Legacy L1Calo path: use JetRoI::thrNames().
              std::tie(l1Et, l1Eta, l1Phi, minDRL1, l1ThresholdsVec) =
                  matchL1Container<xAOD::JetRoI>(
                      jet, *l1Jets,
                      [](const xAOD::JetRoI* r) {
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

          if (isTrigPassed) {
            int ileg = 0;
            for (const ChainNameParser::LegInfo& legInfo :
                 ChainNameParser::HLTChainInfo(m_trigger)) {
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

                ////////////////////////////
                ////  Run 2 emulation  /////
                ////////////////////////////

                if (m_useEmulationTool) {
                  auto hlt_emulated_jets =
                      emulatedJets[legInfo.legName()];  // use pre-fetched
                                                        // emulation results
                  ANA_MSG_DEBUG(" Emulated jets for "
                                << legInfo.legName() << ": "
                                << hlt_emulated_jets.size());

                  for (const auto& [hlt_jet, passBtag] : hlt_emulated_jets) {
                    float dR = jet->p4().DeltaR(hlt_jet->p4());
                    ANA_MSG_VERBOSE("  pt: " << hlt_jet->pt()
                                             << " eta: " << hlt_jet->eta()
                                             << " phi: " << hlt_jet->phi()
                                             << " dR: " << dR);

                    if (bestHLT && isSameJet(bestHLT, hlt_jet))
                      HLTThresholds.insert(legThreshold);
                    else if (dR < minDRHLT) {
                      minDRHLT = dR;
                      bestHLT = hlt_jet;
                      HLTThresholds.clear();
                      HLTThresholds.insert(legThreshold);
                    }
                  }
                }

                ////////////////////////////
                ////  Run 3 access  /////
                ////////////////////////////

                else {
                  frd.setRestrictRequestToLeg(ileg);
                  auto hlt_jetsFromtrigDec =
                      m_trigDecisionTool->features<xAOD::IParticleContainer>(
                          frd);
                  std::vector<const xAOD::IParticle*> allHLTJets;

                  for (const auto& hlt_jet_link : hlt_jetsFromtrigDec) {
                    const xAOD::IParticle* hlt_jetFromtrigDec =
                        *hlt_jet_link.link;
                    if (!hlt_jetFromtrigDec)
                      continue;
                    allHLTJets.push_back(hlt_jetFromtrigDec);
                  }

                  // Start adding missing HLT jets -- only for buggy triggers
                  if (std::find(m_triggerNavBug.begin(), m_triggerNavBug.end(),
                                m_trigger.value()) != m_triggerNavBug.end()) {
                    for (const xAOD::Jet* jetFromCont : *hltJetsFromCont) {
                      bool alreadyIn = false;
                      for (const xAOD::IParticle* seenJet : allHLTJets) {
                        if (isSameJet(seenJet, jetFromCont)) {
                          alreadyIn = true;
                          break;
                        }
                      }
                      if (alreadyIn)
                        continue;

                      allHLTJets.push_back(jetFromCont);
                      ANA_MSG_DEBUG("Added missing HLT jet from container: pt="
                                    << jetFromCont->pt()
                                    << " eta=" << jetFromCont->eta()
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

                    ANA_MSG_VERBOSE(
                        "  pt: " << hlt_jet->pt() << " eta: " << hlt_jet->eta()
                                 << " phi: " << hlt_jet->phi() << " dR: " << dR
                                 << " (fromContainer=" << !fromtrigDec << ")");

                    if (bestHLT && isSameJet(bestHLT, hlt_jet))
                      HLTThresholds.insert(legThreshold);
                    else if (dR < minDRHLT) {
                      minDRHLT = dR;
                      bestHLT = hlt_jet;
                      HLTThresholds.clear();
                      HLTThresholds.insert(legThreshold);
                    }
                  }  // Loop over allHLTJets
                }    // end Run 3 access
              }  // end HLT matching

              ileg++;
            }
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
