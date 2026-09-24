/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/JetIRCSafeLabelTool.h"
#include "ParticleJetTools/ParticleJetLabelCommon.h"
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgMessaging/Check.h"
#include "AsgTools/ToolHandle.h"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/contrib/FlavInfo.hh"
#include "fastjet/contrib/IFNPlugin.hh"
#include "fastjet/contrib/CMPPlugin.hh"
#include "fastjet/contrib/GHSAlgo.hh"
#include "fastjet/contrib/SDFPlugin.hh"

#include <iostream>
#include <fstream>
#include <numbers>
#include <array>
#include <algorithm>
#include <cctype>
#include <unordered_set>
#include <cmath>  //std::abs

using namespace xAOD;
using namespace fastjet;
using namespace fastjet::contrib;

// Compile-time mapping of algorithm names (matches Algo enum)
static constexpr std::array<const char*, JetIRCSafeLabelTool::N_ALGOS> ALGO_NAMES = {
  "IFN", "CMP", "GHS", "SDF", "AKT"
};

// Helper function to convert string to lowercase
static std::string toLower(const std::string& s) {
  std::string result;
  result.reserve(s.size());
  std::transform(s.begin(), s.end(), std::back_inserter(result),
                 [](unsigned char c){ return std::tolower(c); });
  return result;
}

// Helper function to make antiparticle hadron PIDs
template <size_t N>
constexpr std::array<int, N> negateID(const std::array<int, N>& in) {
  std::array<int, N> out{};
  for (size_t i = 0; i < N; ++i) {
    out[i] = -in[i];
  }
  return out;
}

// Helper function to check whether PID is in list of hadrons
template<typename Container>
inline bool hit(const Container& ids, int pdgId) {
    return std::ranges::any_of(ids, [pdgId](int id) {
        return id == pdgId;
    });
}

// Helper function to create JetDefinition that owns a plugin
template <typename PluginT, typename... Args>
std::unique_ptr<fastjet::JetDefinition> makeJetDefWithPlugin(Args&&... args) {
  auto plugin = std::make_unique<PluginT>(std::forward<Args>(args)...);
  auto jetDef = std::make_unique<fastjet::JetDefinition>(plugin.get());
  jetDef->delete_plugin_when_unused();
  //coverity[RESOURCE_LEAK]
  plugin.release(); // FastJet now owns the plugin.
  return jetDef;
}

// Helper function to extract tagged jets for jet algorithms
static void extractTaggedJets(const std::vector<fastjet::PseudoJet>& pseudojets,
                        std::vector<fastjet::PseudoJet>& btagged,
                        std::vector<fastjet::PseudoJet>& ctagged) {
    for (const auto& p : pseudojets) {
        if (p.user_info<fastjet::contrib::FlavHistory>().current_flavour()[5] != 0)
            btagged.push_back(p);
        if (p.user_info<fastjet::contrib::FlavHistory>().current_flavour()[4] != 0)
            ctagged.push_back(p);
    }
}

FlavInfo HeavyFlavourContent(int pdgId) {
  // Simply using the default FlavInfo-from-pdg_code constructor doesn't seem
  // right as it gives way too many +c flavours. It might be because we remove the children
  // and we might be left with quite exotic D hadrons with pdg codes of length larger than 4
  // and that default constructor is unpredictable then.

  // I'm tired of trying to be clever about this. The lists below should contain all
  // hadrons containing a non-zero net b or c flavour, split into groups. The FlavInfo
  // is then decided by checking whether the pdgId belongs to one of these groups.
  // The codes are taken from https://pdg.lbl.gov/2007/reviews/montecarlorpp.pdf

  static constexpr std::array<int, 33> one_b_Ids = {
    // BOTTOM MESONS WITH NO C
    -511, -521, -10511, -10521, -513, -523, -10513, -10523, -20513, -20523,
    -515, -525, -531, -10531, -533, -10533, -20533, 
    -535,
    // BOTTOM BARYONS WITH NO C
    5122, 5112, 5212, 5222, 5114, 5214, 5224, 5132, 5232, 5312, 5322, 5314,
    5324, 5332, 5334
  };
  static constexpr std::array<int,33> one_antib_Ids = negateID(one_b_Ids);
  static constexpr std::array<int,6> one_b_one_antic_Ids = {
    // BOTTOM MESONS WITH ONE B ONE ANTIC
    -541, -10541, -543, -10543, -20543, -545
  };
  static constexpr std::array<int,6> one_antib_one_c_Ids = negateID(one_b_one_antic_Ids);
  static constexpr std::array<int,9> one_b_one_c_Ids = {
    // BOTTOM BARYONS WITH ONE B ONE C
    5142, 5242, 5412, 5422, 5414, 5424, 5342, 5432, 5434
  };
  static constexpr std::array<int,9> one_antib_one_antic_Ids = negateID(one_b_one_c_Ids);
  static constexpr std::array<int,2> one_b_two_cs_Ids = {
    // BOTTOM BARYONS WITH ONE B TWO CS
    5442, 5444
  };
  static constexpr std::array<int,2> one_antib_two_antics_Ids = negateID(one_b_two_cs_Ids);
  static constexpr std::array<int,6> two_bs_Ids = {
    // BOTTOM BARYONS WITH TWO BS AND NO C
    5512, 5522, 5514, 5524, 5532, 5534
  };
  static constexpr std::array<int,6> two_antibs_Ids = negateID(two_bs_Ids);
  static constexpr std::array<int,2> two_bs_one_c_Ids = {
    // BOTTOM BARYONS WITH TWO BS AND ONE C
    5542, 5544
  };
  static constexpr std::array<int,2> two_antibs_one_antic_Ids = negateID(two_bs_one_c_Ids); 
  static constexpr std::array<int,1> three_bs = {
    // BOTTOM BARYON WITH THREE BS (OMEGA-)
    5554
  };
  static constexpr std::array<int,1> three_antibs = negateID(three_bs);

  static constexpr std::array<int,33> one_c_Ids = {
    // CHARMED MESONS WITH NO B (all of them; if they have B they become bottom hadrons)
    411, 421, 10411, 10421, 413, 423, 10413, 10423, 20413, 20423,
    415, 425, 431, 10431, 433, 10433, 20433, 435,
    // CHARMED BARYONS WITH ONE C AND NO B (all of them)
    4122, 4222, 4212, 4112, 4224, 4214, 4114, 4232, 4132, 4322, 4312, 4324,
    4314, 4332, 4334
  };
  static constexpr std::array<int,33> one_antic_Ids = negateID(one_c_Ids);
  static constexpr std::array<int,6> two_c_Ids = {
    // CHARMED BARYONS WITH TWO Cs
    4412, 4422, 4414, 4424, 4432, 4434
  };
  static constexpr std::array<int,6> two_antic_Ids = negateID(two_c_Ids);
  static constexpr std::array<int,1> three_c_Ids = {
    4444
  };
  static constexpr std::array<int,1> three_antic_Ids = negateID(three_c_Ids);
  

  int nb = 0, nc = 0;

  // Check most specific categories first
  if      (hit(three_bs, pdgId))                      { nb =  3; }
  else if (hit(three_antibs, pdgId))                  { nb = -3; }
  else if (hit(two_bs_one_c_Ids, pdgId))              { nb =  2; nc =  1; }
  else if (hit(two_antibs_one_antic_Ids, pdgId))      { nb = -2; nc = -1; }
  else if (hit(two_bs_Ids, pdgId))                    { nb =  2; }
  else if (hit(two_antibs_Ids, pdgId))                { nb = -2; }
  else if (hit(one_b_two_cs_Ids, pdgId))              { nb =  1; nc =  2; }
  else if (hit(one_antib_two_antics_Ids, pdgId))      { nb = -1; nc = -2; }
  else if (hit(one_b_one_c_Ids, pdgId))               { nb =  1; nc =  1; }
  else if (hit(one_antib_one_antic_Ids, pdgId))       { nb = -1; nc = -1; }
  else if (hit(one_b_one_antic_Ids, pdgId))           { nb =  1; nc = -1; }
  else if (hit(one_antib_one_c_Ids, pdgId))           { nb = -1; nc =  1; }
  // Then check more general categories
  else if (hit(one_b_Ids, pdgId))                     { nb =  1; }
  else if (hit(one_antib_Ids, pdgId))                 { nb = -1; }
  else if (hit(three_c_Ids, pdgId))                   { nc =  3; }
  else if (hit(three_antic_Ids, pdgId))               { nc = -3; }
  else if (hit(two_c_Ids, pdgId))                     { nc =  2; }
  else if (hit(two_antic_Ids, pdgId))                 { nc = -2; }
  else if (hit(one_c_Ids, pdgId))                     { nc =  1; }
  else if (hit(one_antic_Ids, pdgId))                 { nc = -1; }

  return FlavInfo(0,0,0,nc,nb,0,0);
}


JetIRCSafeLabelTool::JetIRCSafeLabelTool(const std::string& name) : AsgTool(name) {}

std::vector< std::vector<PseudoJet> > JetIRCSafeLabelTool::getJetInputs(
                                                   const TruthParticleContainer& parts,
                                                   const TruthParticleContainer& label_bs,
                                                   const TruthParticleContainer& label_cs) const {

  // Collect all particles needed
  std::vector<const TruthParticle*> truthparts(parts.begin(), parts.end());
  std::vector<const TruthParticle*> bs(label_bs.begin(), label_bs.end());
  std::vector<const TruthParticle*> cs(label_cs.begin(), label_cs.end());
  
  // Remove all children of B and D hadrons
  using ParticleJetTools::childrenRemoved;
  childrenRemoved(bs, bs);
  childrenRemoved(bs, cs);
  childrenRemoved(bs, truthparts);
  childrenRemoved(cs, cs);
  childrenRemoved(cs, truthparts);

  // Add original B and D hadrons to the truth particles
  truthparts.insert(truthparts.end(), bs.begin(), bs.end());
  truthparts.insert(truthparts.end(), cs.begin(), cs.end());

  // Now, create the PseudoJet event from them
  std::vector<PseudoJet> fullevent(truthparts.size());
  for (unsigned int ip = 0; ip < truthparts.size(); ip++) {
    const TruthParticle* part = truthparts[ip];
    double px = part->px();
    double py = part->py();
    double pz = part->pz();
    double E  = part->e();
    fullevent[ip] = PseudoJet(px,py,pz,E);
    FlavInfo partFlavInfo = HeavyFlavourContent(part->pdgId());
    // fastjet sets a weird example to take ownership. 
    // But we don't have a better option than using their interface the way it's designed.
    fullevent[ip].set_user_info(new FlavInfo(partFlavInfo));
  }

  std::array< std::vector<PseudoJet>, N_ALGOS > all_pseudojets_array{};
  
  bool doIFN = doAlgo(Algo::IFN);
  bool doCMP = doAlgo(Algo::CMP);
  bool doGHS = doAlgo(Algo::GHS);
  bool doSDF = doAlgo(Algo::SDF);
  bool doAKT = doAlgo(Algo::AKT);

  // If no algorithms enabled, return empty vector
  if (!(doIFN || doCMP || doGHS || doSDF || doAKT)) {
    std::vector<std::vector<PseudoJet>> empty_result;
    empty_result.resize(N_ALGOS);
    return empty_result;
  }

  const Selector& selectpt = *m_selectPt;
  const FlavRecombiner& flav_recombiner = *m_flavRecombiner;
  const JetDefinition& akt_jet_def = *m_aktJetDef;

  if (doIFN) {
    const JetDefinition& ifn_jet_def = *m_ifnJetDef;
    all_pseudojets_array[static_cast<std::size_t>(Algo::IFN)] = selectpt(ifn_jet_def(fullevent));
  }

  if (doCMP) {
    const JetDefinition& cmp_jet_def = *m_cmpJetDef;
    all_pseudojets_array[static_cast<std::size_t>(Algo::CMP)] = selectpt(cmp_jet_def(fullevent));
  }

  std::vector<PseudoJet> base_jets;
  if (doAKT || doGHS || doSDF) {
    base_jets = selectpt(akt_jet_def(fullevent));
  }

  // GHS parameters using class constants
  if (doGHS) {
    all_pseudojets_array[static_cast<std::size_t>(Algo::GHS)] = 
      run_GHS(base_jets, GHS_PT_CUT, GHS_ALPHA, GHS_OMEGA, flav_recombiner);
  }

  if (doSDF) {
    SDFlavourCalc sdFlavCalc;
    std::vector<PseudoJet> SDF_jets = base_jets;
    sdFlavCalc(SDF_jets);
    all_pseudojets_array[static_cast<std::size_t>(Algo::SDF)] = std::move(SDF_jets);
  }

  if (doAKT) {
    // Copy base_jets instead of moving, as it may be used by GHS and SDF
    all_pseudojets_array[static_cast<std::size_t>(Algo::AKT)] = std::move(base_jets);
  }

  std::vector<std::vector<PseudoJet>> all_pseudojets;
  all_pseudojets.reserve(N_ALGOS);
  for (std::size_t i = 0; i < N_ALGOS; ++i) {
    all_pseudojets.push_back(std::move(all_pseudojets_array[i]));
  }
  
  return all_pseudojets;
}

void setJetIRCSafeLabels(const xAOD::Jet& jet,
                         const ParticleJetTools::Tag_PseudoJets& tag_pjets,
                         const ParticleJetTools::IRCSafeLabelDecorators& decs,
                         bool doIFN, bool doCMP, bool doGHS, bool doSDF, bool doAKT) {

  int IFN_label = doIFN ? (tag_pjets.IFN_b.size() ? JetIRCSafeLabelTool::LABEL_B :
                           tag_pjets.IFN_c.size() ? JetIRCSafeLabelTool::LABEL_C : JetIRCSafeLabelTool::LABEL_LIGHT) : JetIRCSafeLabelTool::LABEL_DISABLED;
  int CMP_label = doCMP ? (tag_pjets.CMP_b.size() ? JetIRCSafeLabelTool::LABEL_B :
                           tag_pjets.CMP_c.size() ? JetIRCSafeLabelTool::LABEL_C : JetIRCSafeLabelTool::LABEL_LIGHT) : JetIRCSafeLabelTool::LABEL_DISABLED;
  int GHS_label = doGHS ? (tag_pjets.GHS_b.size() ? JetIRCSafeLabelTool::LABEL_B :
                           tag_pjets.GHS_c.size() ? JetIRCSafeLabelTool::LABEL_C : JetIRCSafeLabelTool::LABEL_LIGHT) : JetIRCSafeLabelTool::LABEL_DISABLED;
  int SDF_label = doSDF ? (tag_pjets.SDF_b.size() ? JetIRCSafeLabelTool::LABEL_B :
                           tag_pjets.SDF_c.size() ? JetIRCSafeLabelTool::LABEL_C : JetIRCSafeLabelTool::LABEL_LIGHT) : JetIRCSafeLabelTool::LABEL_DISABLED;
  int AKT_label = doAKT ? (tag_pjets.AKT_b.size() ? JetIRCSafeLabelTool::LABEL_B :
                           tag_pjets.AKT_c.size() ? JetIRCSafeLabelTool::LABEL_C : JetIRCSafeLabelTool::LABEL_LIGHT) : JetIRCSafeLabelTool::LABEL_DISABLED;

  decs.IFNsingleint(jet) = IFN_label;
  decs.CMPsingleint(jet) = CMP_label;
  decs.GHSsingleint(jet) = GHS_label;
  decs.SDFsingleint(jet) = SDF_label;
  decs.AKTsingleint(jet) = AKT_label;
}

StatusCode JetIRCSafeLabelTool::initialize(){

  ATH_MSG_DEBUG(" Initializing... ");
  
  // Initialize truth jet inputs key
  ATH_CHECK(m_outTruthPartKey.initialize());
  ATH_CHECK(m_bottomPartCollectionKey.initialize());
  ATH_CHECK(m_charmPartCollectionKey.initialize());
  
  // Update label names from properties 
  m_ircsafelabelnames.IFNsingleint = m_labelNameIFN.value();
  m_ircsafelabelnames.CMPsingleint = m_labelNameCMP.value();
  m_ircsafelabelnames.GHSsingleint = m_labelNameGHS.value();
  m_ircsafelabelnames.SDFsingleint = m_labelNameSDF.value();
  m_ircsafelabelnames.AKTsingleint = m_labelNameAKT.value();
  
  // Build decorators using updated label names
  m_ircsafelabeldecs =
    std::make_unique<ParticleJetTools::IRCSafeLabelDecorators>(m_ircsafelabelnames);

  // Initialize all algorithms to false
  m_doAlgo.fill(false);
  
  // Parse enabled algorithm names from configuration 
  for (const auto& name : m_enabledAlgorithms.value()) {
    std::string lowerName = toLower(name);
    bool found = false;
    
    for (std::size_t i = 0; i < N_ALGOS; ++i) {
      if (toLower(ALGO_NAMES[i]) == lowerName) {
        m_doAlgo[i] = true;
        found = true;
        break;
      }
    }
    
    if (!found) {
      ATH_MSG_WARNING("Unknown algorithm in EnabledAlgorithms: " << name);
    }
  }

  // Check if any algorithm is enabled
  bool anyEnabled = false;
  for (bool enabled : m_doAlgo) {
    if (enabled) {
      anyEnabled = true;
      break;
    }
  }
  
  if (!anyEnabled) {
    ATH_MSG_WARNING("EnabledAlgorithms is empty; no IRCSafe labelling will be performed.");
    return StatusCode::SUCCESS;
  }

  bool doIFN = doAlgo(Algo::IFN);
  bool doCMP = doAlgo(Algo::CMP);

  // Build FastJet objects using property values 
  m_selectPt = std::make_unique<Selector>(SelectorPtMin(m_truthJetPtMin.value()));
  m_flavRecombiner = std::make_unique<FlavRecombiner>(FlavRecombiner::net);
  m_aktJetDef = std::make_unique<JetDefinition>(antikt_algorithm, m_truthR.value());
  m_aktJetDef->set_recombiner(m_flavRecombiner.get());
  
  // Interleaved flavour neutralisation - IFN (2306.07314)
  if (doIFN) {
      m_ifnJetDef = makeJetDefWithPlugin<IFNPlugin>(
          *m_aktJetDef, IFN_RFACTOR, IFN_BETA, FlavRecombiner::net
      );
  }

  // Czakon, Mitov, Poncelet (CMP) algorithm - flavour anti-kt (2205.11879)
  if (doCMP) {
      m_cmpJetDef = makeJetDefWithPlugin<CMPPlugin>(
          m_truthR.value(), CMP_A,
          CMPPlugin::CorrectionType::OverAllCoshyCosPhi_a2,
          CMPPlugin::ClusteringType::DynamicKtMax
      );
      m_cmpJetDef->set_recombiner(m_flavRecombiner.get());
  }
  
  return StatusCode::SUCCESS;
}

std::vector< std::vector<const PseudoJet*> > JetIRCSafeLabelTool::match(
                                                    std::vector<PseudoJet>& tagged_pseudojets,
                                                    const JetContainer& jets) const {
  ATH_MSG_VERBOSE("In " << name() << "::match()");

  std::vector< std::vector<const PseudoJet*> > jetlabelparts(jets.size(), std::vector<const PseudoJet*>());

  // Loop over pseudojets and find the best matched jet
  for (unsigned int i_tj = 0; i_tj < tagged_pseudojets.size(); i_tj++) {

    const auto* tag_pjet = &tagged_pseudojets[i_tj];

    double mindr = DBL_MAX;
    int mindrjetidx = -1;
    
    for (unsigned int iJet = 0; iJet < jets.size(); iJet++) {

      const Jet& jet = *jets[iJet];

      double pt = jet.pt();
      if (pt < m_jetPtMin.value()) 
          continue;

      double drap = abs(jet.rapidity() - tag_pjet->rap());
      double dphi = abs(jet.phi() - tag_pjet->phi());
      if (dphi > numbers::pi) dphi = 2*numbers::pi - dphi;
      double dr = sqrt(drap*drap + dphi*dphi);

      // Too far for matching criterion
      if (dr > m_drMax.value())  
          continue;

      // Store the matched jet
      if (dr < mindr) {
          mindr = dr;
          mindrjetidx = iJet;
      }

    }

    // Store the label particle with the jet
    if (mindrjetidx >= 0) {
      jetlabelparts.at(mindrjetidx).push_back(tag_pjet);
    } 
  }

  return jetlabelparts;
}

StatusCode JetIRCSafeLabelTool::decorate(const JetContainer& jets) const {

  // Retrieve truth particle collections
  SG::ReadHandle<xAOD::TruthParticleContainer> truthPartReadHandle(m_outTruthPartKey);
  SG::ReadHandle<xAOD::TruthParticleContainer> bottomReadHandle(m_bottomPartCollectionKey);
  SG::ReadHandle<xAOD::TruthParticleContainer> charmReadHandle(m_charmPartCollectionKey);
  
  if (!truthPartReadHandle.isValid()) {
    ATH_MSG_ERROR("Invalid ReadHandle for TruthParticleCollection with key: " << truthPartReadHandle.key());
    return StatusCode::FAILURE;
  }
  if (!bottomReadHandle.isValid()) {
    ATH_MSG_ERROR("Invalid ReadHandle for bottomPartCollection with key: " << bottomReadHandle.key());
    return StatusCode::FAILURE;
  }
  if (!charmReadHandle.isValid()) {
    ATH_MSG_ERROR("Invalid ReadHandle for charmPartCollection with key: " << charmReadHandle.key());
    return StatusCode::FAILURE;
  }
  
  // Get algorithm flags
  bool doIFN = doAlgo(Algo::IFN);
  bool doCMP = doAlgo(Algo::CMP);
  bool doGHS = doAlgo(Algo::GHS);
  bool doSDF = doAlgo(Algo::SDF);
  bool doAKT = doAlgo(Algo::AKT);
  
  // Get all pseudo-jets from enabled algorithms
  std::vector<std::vector<PseudoJet>> all_pseudojets = 
      getJetInputs(*truthPartReadHandle, *bottomReadHandle, *charmReadHandle);
  
  // Extract b- and c-tagged jets for each algorithm
  std::vector<PseudoJet> btagged_pseudojetsIFN, ctagged_pseudojetsIFN;
  std::vector<PseudoJet> btagged_pseudojetsCMP, ctagged_pseudojetsCMP;
  std::vector<PseudoJet> btagged_pseudojetsGHS, ctagged_pseudojetsGHS;
  std::vector<PseudoJet> btagged_pseudojetsSDF, ctagged_pseudojetsSDF;
  std::vector<PseudoJet> btagged_pseudojetsAKT, ctagged_pseudojetsAKT;

  // Extract IFN jets
  if (doIFN && all_pseudojets.size() > static_cast<std::size_t>(Algo::IFN)) {
    extractTaggedJets(all_pseudojets[static_cast<std::size_t>(Algo::IFN)],
                      btagged_pseudojetsIFN, ctagged_pseudojetsIFN);
  }

  // Extract CMP jets
  if (doCMP && all_pseudojets.size() > static_cast<std::size_t>(Algo::CMP)) {
    extractTaggedJets(all_pseudojets[static_cast<std::size_t>(Algo::CMP)],
                      btagged_pseudojetsCMP, ctagged_pseudojetsCMP);
  }

  // Extract GHS jets
  if (doGHS && all_pseudojets.size() > static_cast<std::size_t>(Algo::GHS)) {
    extractTaggedJets(all_pseudojets[static_cast<std::size_t>(Algo::GHS)],
                      btagged_pseudojetsGHS, ctagged_pseudojetsGHS);
  }

  // Extract SDF jets
  if (doSDF && all_pseudojets.size() > static_cast<std::size_t>(Algo::SDF)) {
    extractTaggedJets(all_pseudojets[static_cast<std::size_t>(Algo::SDF)],
                      btagged_pseudojetsSDF, ctagged_pseudojetsSDF);
  }

  // Extract AKT jets
  if (doAKT && all_pseudojets.size() > static_cast<std::size_t>(Algo::AKT)) {
    extractTaggedJets(all_pseudojets[static_cast<std::size_t>(Algo::AKT)],
                      btagged_pseudojetsAKT, ctagged_pseudojetsAKT);
  }

  // Match the tagged pseudojets to reco jets
  std::vector<std::vector<const PseudoJet*>> jetlabelIFN_b = doIFN ? match(btagged_pseudojetsIFN, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelIFN_c = doIFN ? match(ctagged_pseudojetsIFN, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelCMP_b = doCMP ? match(btagged_pseudojetsCMP, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelCMP_c = doCMP ? match(ctagged_pseudojetsCMP, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelGHS_b = doGHS ? match(btagged_pseudojetsGHS, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelGHS_c = doGHS ? match(ctagged_pseudojetsGHS, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelSDF_b = doSDF ? match(btagged_pseudojetsSDF, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelSDF_c = doSDF ? match(ctagged_pseudojetsSDF, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelAKT_b = doAKT ? match(btagged_pseudojetsAKT, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());
  std::vector<std::vector<const PseudoJet*>> jetlabelAKT_c = doAKT ? match(ctagged_pseudojetsAKT, jets) : std::vector<std::vector<const PseudoJet*>>(jets.size());

  for (unsigned int iJet = 0; iJet < jets.size(); iJet++) {
    const Jet& jet = *jets[iJet];
    if (jet.pt() < m_jetPtMin.value()) { 
      m_ircsafelabeldecs->IFNsingleint(jet) = LABEL_DISABLED;
      m_ircsafelabeldecs->CMPsingleint(jet) = LABEL_DISABLED;
      m_ircsafelabeldecs->GHSsingleint(jet) = LABEL_DISABLED;
      m_ircsafelabeldecs->SDFsingleint(jet) = LABEL_DISABLED;
      m_ircsafelabeldecs->AKTsingleint(jet) = LABEL_DISABLED;
      continue;
    }

    ParticleJetTools::Tag_PseudoJets tag_pjet;
    tag_pjet.IFN_b = jetlabelIFN_b[iJet];
    tag_pjet.IFN_c = jetlabelIFN_c[iJet];
    tag_pjet.CMP_b = jetlabelCMP_b[iJet];
    tag_pjet.CMP_c = jetlabelCMP_c[iJet];
    tag_pjet.GHS_b = jetlabelGHS_b[iJet];
    tag_pjet.GHS_c = jetlabelGHS_c[iJet];
    tag_pjet.SDF_b = jetlabelSDF_b[iJet];
    tag_pjet.SDF_c = jetlabelSDF_c[iJet];
    tag_pjet.AKT_b = jetlabelAKT_b[iJet];
    tag_pjet.AKT_c = jetlabelAKT_c[iJet];

    setJetIRCSafeLabels(jet, tag_pjet, *m_ircsafelabeldecs, doIFN, doCMP, doGHS, doSDF, doAKT);
  }

  return StatusCode::SUCCESS;
}