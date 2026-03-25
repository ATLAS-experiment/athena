/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ParticleJetTools/IRCSafeLabelTool.h"
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
#include <algorithm>
#include <cctype>
#include <unordered_set>

using namespace std;
using namespace xAOD;
using namespace fastjet;
using namespace fastjet::contrib;

// this is from ChatGPT but I quite like it
inline bool hit(const std::valarray<int>& ids, int pdgId) {
  return (ids == pdgId).max();
}

FlavInfo HeavyFlavourContent(int pdgId) {
  // simply using the default FlavInfo-from-pdg_code constructor doesn't seem
  // right as it gives way too many +c flavours. It might be because we remove the children
  // and we might be left with quite exotic D hadrons with pdg codes of length larger than 4
  // and that default constructor is unpredictable then.

  // I'm tired of trying to be clever about this. The lists below should contain all
  // hadrons containing a non-zero net b or c flavour, split into groups. The FlavInfo
  // is then decided by checking whether the pdgId belongs to one of these groups.
  // The codes are taken from https://pdg.lbl.gov/2007/reviews/montecarlorpp.pdf

  valarray<int> one_b_Ids = {
    // BOTTOM MESONS WITH NO C
    -511, -521, -10511, -10521, -513, -523, -10513, -10523, -20513, -20523,
    -515, -525, -531, -10531, -533, -10533, -20533, 
    -535,
    // BOTTOM BARYONS WITH NO C
    5122, 5112, 5212, 5222, 5114, 5214, 5224, 5132, 5232, 5312, 5322, 5314,
    5324, 5332, 5334
  };
  valarray<int> one_antib_Ids = -one_b_Ids;
  valarray<int> one_b_one_antic_Ids = {
    // BOTTOM MESONS WITH ONE B ONE ANTIC
    -541, -10541, -543, -10543, -20543, -545
  };
  valarray<int> one_antib_one_c_Ids = -one_b_one_antic_Ids;
  valarray<int> one_b_one_c_Ids = {
    // BOTTOM BARYONS WITH ONE B ONE C
    5142, 5242, 5412, 5422, 5414, 5424, 5342, 5432, 5434
  };
  valarray<int> one_antib_one_antic_Ids = -one_b_one_c_Ids;
  valarray<int> one_b_two_cs_Ids = {
    // BOTTOM BARYONS WITH ONE B TWO CS
    5442, 5444
  };
  valarray<int> one_antib_two_antics_Ids = -one_b_two_cs_Ids;
  valarray<int> two_bs_Ids = {
    // BOTTOM BARYONS WITH TWO BS AND NO C
    5512, 5522, 5514, 5524, 5532, 5534
  };
  valarray<int> two_antibs_Ids = -two_bs_Ids;
  valarray<int> two_bs_one_c_Ids = {
    // BOTTOM BARYONS WITH TWO BS AND ONE C
    5542, 5544
  };
  valarray<int> two_antibs_one_antic_Ids = -two_bs_one_c_Ids; 
  valarray<int> three_bs = {
    // BOTTOM BARYON WITH THREE BS (OMEGA-)
    5554
  };
  valarray<int> three_antibs = -three_bs;

  valarray<int> one_c_Ids = {
    // CHARMED MESONS WITH NO B (all of them; if they have B they become bottom hadrons)
    411, 421, 10411, 10421, 413, 423, 10413, 10423, 20413, 20423,
    415, 425, 431, 10431, 433, 10433, 20433, 435,
    // CHARMED BARYONS WITH ONE C AND NO B (all of them)
    4122, 4222, 4212, 4112, 4224, 4214, 4114, 4232, 4132, 4322, 4312, 4324,
    4314, 4332, 4334
  };
  valarray<int> one_antic_Ids = -one_c_Ids;
  valarray<int> two_c_Ids = {
    // CHARMED BARYONS WITH TWO Cs
    4412, 4422, 4414, 4424, 4432, 4434
  };
  valarray<int> two_antic_Ids = -two_c_Ids;
  valarray<int> three_c_Ids = {
    4444
  };
  valarray<int> three_antic_Ids = -three_c_Ids;
  

  int nb = 0, nc = 0;

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


IRCSafeLabelTool::IRCSafeLabelTool(const std::string& name) : AsgTool(name) {
    m_ircsafelabelnames.IFNsingleint = "IRCSafeLabelIFN";
    m_ircsafelabelnames.CMPsingleint = "IRCSafeLabelCMP";
    m_ircsafelabelnames.GHSsingleint = "IRCSafeLabelGHS";
    m_ircsafelabelnames.SDFsingleint = "IRCSafeLabelSDFlav";
    m_ircsafelabelnames.AKTsingleint = "IRCSafeLabelAKT";

    declareProperty("LabelNameIFN", m_ircsafelabelnames.IFNsingleint, "Name of the jet label attribute to be added (IFN)");
    declareProperty("LabelNameCMP", m_ircsafelabelnames.CMPsingleint, "Name of the jet label attribute to be added (CMP)");
    declareProperty("LabelNameGHS", m_ircsafelabelnames.GHSsingleint, "Name of the jet label attribute to be added (GHS)");
    declareProperty("LabelNameSDF", m_ircsafelabelnames.SDFsingleint, "Name of the jet label attribute to be added (SDF, Marzani et al.)");
    declareProperty("LabelNameAKT", m_ircsafelabelnames.AKTsingleint, "Name of the jet label attribute to be added (anti-kt with net flavour - NOT IRC SAFE)");
    declareProperty("EnabledAlgorithms", m_enabledAlgos = {"IFN","CMP","GHS","SDF","AKT"}, "Subset of IRCSafe algorithms to run (IFN, CMP, GHS, SDF, AKT)");
    declareProperty("TruthJetPtMin", m_truthjetptmin=5000, "Minimum pT of truth jets that are matched to reco for labeling [MeV]");
    declareProperty("JetPtMin", m_jetptmin=10000, "Minimum pT of reco jets to be labeled [MeV]");
    declareProperty("DRMax", m_drmax=0.3, "Maximum deltaR between a particle and jet to be labeled");
    declareProperty("TruthR", m_truthR=0.4, "Radius with which truth particles will be clustered when determining the flavour "
                                            "(should be equal to the radius of the tagged jets)");

}

vector< vector<PseudoJet> > IRCSafeLabelTool::getJetInputs(const TruthParticleContainer& parts,
                                                          const TruthParticleContainer& label_bs,
                                                          const TruthParticleContainer& label_cs) const {

  // collect all particles needed
  vector<const TruthParticle*> bs, cs, truthparts;
  for (TruthParticleContainer::const_iterator part_itr = parts.begin();
          part_itr != parts.end(); ++part_itr) {
            const TruthParticle* part = (*part_itr);
            truthparts.push_back(part);
  }
  for (TruthParticleContainer::const_iterator labels_itr = label_bs.begin();
          labels_itr != label_bs.end(); ++labels_itr) {
            const TruthParticle* part = (*labels_itr);
            bs.push_back(part);
  }
  for (TruthParticleContainer::const_iterator labels_itr = label_cs.begin();
          labels_itr != label_cs.end(); ++labels_itr) {
            const TruthParticle* part = (*labels_itr);
            cs.push_back(part);
  }
  // remove all children of B and D hadrons
  using ParticleJetTools::childrenRemoved;
  childrenRemoved(bs, bs);
  childrenRemoved(bs, cs);
  childrenRemoved(bs, truthparts);
  childrenRemoved(cs, cs);
  childrenRemoved(cs, truthparts);

  // add original B and D hadrons to the truth particles
  if (bs.size() > 0) truthparts.insert(truthparts.end(), bs.begin(), bs.end());
  if (cs.size() > 0) truthparts.insert(truthparts.end(), cs.begin(), cs.end());

  // now, create the PseudoJet event from them
  vector<PseudoJet> fullevent(truthparts.size());
  for (unsigned int ip = 0; ip < truthparts.size(); ip++) {
    const TruthParticle* part = truthparts[ip];
    double px = part->px();
    double py = part->py();
    double pz = part->pz();
    double E  = part->e();
    fullevent[ip] = PseudoJet(px,py,pz,E);
    FlavInfo partFlavInfo = HeavyFlavourContent(part->pdgId());
    fullevent[ip].set_user_info(new FlavInfo(partFlavInfo));
  }

  vector< vector<PseudoJet> > all_pseudojets(5);
  if (!(m_doIFN || m_doCMP || m_doGHS || m_doSDF || m_doAKT)) {
    return all_pseudojets;
  }

  const Selector& selectpt = *m_selectPt;
  const FlavRecombiner& flav_recombiner = *m_flavRecombiner;
  const JetDefinition& akt_jet_def = *m_aktJetDef;

  if (m_doIFN) {
    const JetDefinition& ifn_jet_def = *m_ifnJetDef;
    all_pseudojets[0] = selectpt(ifn_jet_def(fullevent));
  }

  if (m_doCMP) {
    const JetDefinition& cmp_jet_def = *m_cmpJetDef;
    all_pseudojets[1] = selectpt(cmp_jet_def(fullevent));
  }

  vector<PseudoJet> base_jets;
  if (m_doAKT || m_doGHS || m_doSDF) {
    base_jets = selectpt(akt_jet_def(fullevent));
  }

  // GHS parameters:
  double GHS_alpha = 1.0; // < flav-kt distance parameter alpha
  double GHS_omega = 0.0; // < omega parameter for GHS_Omega (omega = 0 uses DeltaR_ij^2)
  double ptcut = 5000.0; // < overall ptcut which is an input to GHS; same as the cut on truth-level jets
  if (m_doGHS) {
    all_pseudojets[2] = run_GHS(base_jets, ptcut, GHS_alpha, GHS_omega, flav_recombiner);
  }

  if (m_doSDF) {
    SDFlavourCalc sdFlavCalc;
    vector<PseudoJet> SDF_jets = base_jets;
    sdFlavCalc(SDF_jets);
    all_pseudojets[3] = std::move(SDF_jets);
  }

  if (m_doAKT) {
    all_pseudojets[4] = std::move(base_jets);
  }

  return all_pseudojets;

}

void setJetIRCSafeLabels(const xAOD::Jet& jet,
                         const ParticleJetTools::Tag_PseudoJets& tag_pjets,
                         const ParticleJetTools::IRCSafeLabelDecorators& decs,
                         bool doIFN, bool doCMP, bool doGHS, bool doSDF, bool doAKT) {

  int IFN_label = doIFN ? (tag_pjets.IFN_b.size() ? 5 :
                           tag_pjets.IFN_c.size() ? 4 : 0) : -99;
  int CMP_label = doCMP ? (tag_pjets.CMP_b.size() ? 5 :
                           tag_pjets.CMP_c.size() ? 4 : 0) : -99;
  int GHS_label = doGHS ? (tag_pjets.GHS_b.size() ? 5 :
                           tag_pjets.GHS_c.size() ? 4 : 0) : -99;
  int SDF_label = doSDF ? (tag_pjets.SDF_b.size() ? 5 :
                           tag_pjets.SDF_c.size() ? 4 : 0) : -99;
  int AKT_label = doAKT ? (tag_pjets.AKT_b.size() ? 5 :
                           tag_pjets.AKT_c.size() ? 4 : 0) : -99;

  decs.IFNsingleint(jet) = IFN_label;
  decs.CMPsingleint(jet) = CMP_label;
  decs.GHSsingleint(jet) = GHS_label;
  decs.SDFsingleint(jet) = SDF_label;
  decs.AKTsingleint(jet) = AKT_label;

}

StatusCode IRCSafeLabelTool::initialize(){

  ATH_MSG_DEBUG(" Initializing... ");
  // initialize truth jet inputs key
  ATH_CHECK(m_outTruthPartKey.initialize());
  ATH_CHECK(m_bottomPartCollectionKey.initialize());
  ATH_CHECK(m_charmPartCollectionKey.initialize());
  // build label decorators
  m_ircsafelabeldecs =
    std::make_unique<ParticleJetTools::IRCSafeLabelDecorators>(m_ircsafelabelnames);

  std::unordered_set<std::string> enabled;
  for (auto name : m_enabledAlgos) {
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    enabled.insert(name);
  }
  m_doIFN = enabled.count("ifn") > 0;
  m_doCMP = enabled.count("cmp") > 0;
  m_doGHS = enabled.count("ghs") > 0;
  m_doSDF = enabled.count("sdf") > 0 || enabled.count("sdflav") > 0;
  m_doAKT = enabled.count("akt") > 0;

  for (const auto& name : m_enabledAlgos) {
    std::string lowered = name;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    if (!(lowered == "ifn" || lowered == "cmp" || lowered == "ghs" ||
          lowered == "sdf" || lowered == "sdflav" || lowered == "akt")) {
      ATH_MSG_WARNING("Unknown algorithm in EnabledAlgorithms: " << name);
    }
  }

  if (!(m_doIFN || m_doCMP || m_doGHS || m_doSDF || m_doAKT)) {
    ATH_MSG_WARNING("EnabledAlgorithms is empty; no IRCSafe labelling will be performed.");
    return StatusCode::SUCCESS;
  }

  // build algos and other fastjet objects:
  // pt-cut selector and flavour recombination scheme (b + bbar = no flavour, b + b = 2b)
  // another option if modulo_2 (b + bbar = b + b = no flavour), but the CMP and SDFlav algorithms are 
  // not set up to work with it.
  m_selectPt = std::make_unique<Selector>(SelectorPtMin(m_truthjetptmin));
  m_flavRecombiner = std::make_unique<FlavRecombiner>(FlavRecombiner::net);
  // anti-kt with net flavour
  m_aktJetDef = std::make_unique<JetDefinition>(antikt_algorithm, m_truthR);
  m_aktJetDef->set_recombiner(m_flavRecombiner.get());
  // interleaved flavour neutralisation - IFN (2306.07314)
  if (m_doIFN) {
    auto ifn_plugin = std::make_unique<IFNPlugin>(*m_aktJetDef, 1.0, 2.0, FlavRecombiner::net);
    m_ifnJetDef = std::make_unique<JetDefinition>(ifn_plugin.get());
    m_ifnJetDef->delete_plugin_when_unused();
    ifn_plugin.release();
  }
  // Czakon, Mitov, Poncelet (CMP) algorithm - flavour anti-kt (2205.11879)
  if (m_doCMP) {
    const double CMP_a = 0.1;
    // correction to original CMP algo: do not change this if you want IRC safety!
    const CMPPlugin::CorrectionType CMP_corr = CMPPlugin::CorrectionType::OverAllCoshyCosPhi_a2;
    // Dynamic definition of ktmax
    const CMPPlugin::ClusteringType CMP_clust = CMPPlugin::ClusteringType::DynamicKtMax;
    auto cmp_plugin = std::make_unique<CMPPlugin>(m_truthR, CMP_a, CMP_corr, CMP_clust);
    m_cmpJetDef = std::make_unique<JetDefinition>(cmp_plugin.get());
    // enable it to track flavours (default is net flavour)
    m_cmpJetDef->set_recombiner(m_flavRecombiner.get());
    m_cmpJetDef->delete_plugin_when_unused();
    cmp_plugin.release();
  }
  // Other algorithms - Gauld, Huss, Stagnitto (GHS) - flavour dressing (2208.11138) and
  // SDFlav (Caletti, Larkoski, Marzani, Reichelt, 2205.01109) are simple functions of 
  // final-state particles and don't need to be set up externally.
  return StatusCode::SUCCESS;

}

vector< vector<const PseudoJet*> > IRCSafeLabelTool::match(vector<PseudoJet>& tagged_pseudojets,
                                                           const JetContainer& jets) const {
  ATH_MSG_VERBOSE("In " << name() << "::match()");

  vector< vector<const PseudoJet*> > jetlabelparts(jets.size(), vector<const PseudoJet*>());

  // loop over pseudojets and find the best matched jet
  for (unsigned int i_tj = 0; i_tj < tagged_pseudojets.size(); i_tj++) {

    const PseudoJet* tag_pjet = static_cast<const PseudoJet*>(&tagged_pseudojets[i_tj]);

    double mindr = DBL_MAX;
    double maxpt = 0;
    int mindrjetidx = -1;
    int maxptjetidx = -1;
    for (unsigned int iJet = 0; iJet < jets.size(); iJet++) {

      const Jet& jet = *jets.at(iJet);

      double pt = jet.pt();
      if (pt < m_jetptmin)
          continue;

      double drap = abs(jet.rapidity() - tag_pjet->rap());
      double dphi = abs(jet.phi() - tag_pjet->phi());
      if (dphi > numbers::pi) dphi = 2*numbers::pi - dphi;
      double dr = pow(drap*drap + dphi*dphi, 0.5);
      // too far for matching criterion
      if (dr > m_drmax)
          continue;

      // store the matched jet
      if (dr < mindr) {
          mindr = dr;
          mindrjetidx = iJet;
      }

      if (pt > maxpt) {
          maxpt = pt;
          maxptjetidx = iJet;
      }

    }

    // store the label particle with the jet
    if (mindrjetidx >= 0) jetlabelparts.at(mindrjetidx).push_back(tag_pjet);
    else if (maxptjetidx >= 0) jetlabelparts.at(maxptjetidx).push_back(tag_pjet);
    }

    return jetlabelparts;

}

StatusCode IRCSafeLabelTool::decorate(const JetContainer& jets) const {

  ATH_MSG_VERBOSE("In " << name() << "::modify()");
  // Retrieve the particle and jet containers
  SG::ReadHandle<xAOD::TruthParticleContainer> truthPartReadHandle(m_outTruthPartKey);
  SG::ReadHandle<xAOD::TruthParticleContainer> truthbsReadHandle(m_bottomPartCollectionKey);
  SG::ReadHandle<xAOD::TruthParticleContainer> truthcsReadHandle(m_charmPartCollectionKey);
  if (!truthbsReadHandle.isValid()){
    ATH_MSG_DEBUG(" Invalid ReadHandle for xAOD::ParticleContainer with key: " << truthbsReadHandle.key());
    return StatusCode::FAILURE;
  }
  if (!truthcsReadHandle.isValid()){
    ATH_MSG_DEBUG(" Invalid ReadHandle for xAOD::ParticleContainer with key: " << truthcsReadHandle.key());
    return StatusCode::FAILURE;
  }
  if (!truthPartReadHandle.isValid()){
    ATH_MSG_DEBUG(" Invalid ReadHandle for xAOD::ParticleContainer with key: " << truthPartReadHandle.key());
    return StatusCode::FAILURE;
  }

  vector< vector<PseudoJet> > all_pseudojets = getJetInputs(*truthPartReadHandle, *truthbsReadHandle, *truthcsReadHandle);
  vector<PseudoJet> btagged_pseudojetsIFN, ctagged_pseudojetsIFN;
  vector<PseudoJet> btagged_pseudojetsCMP, ctagged_pseudojetsCMP;
  vector<PseudoJet> btagged_pseudojetsGHS, ctagged_pseudojetsGHS;
  vector<PseudoJet> btagged_pseudojetsSDF, ctagged_pseudojetsSDF;
  vector<PseudoJet> btagged_pseudojetsAKT, ctagged_pseudojetsAKT;

  vector<PseudoJet> selected_pseudojets = all_pseudojets[0];
  for (unsigned int i_jets = 0; i_jets < selected_pseudojets.size(); i_jets++) {
    PseudoJet p = selected_pseudojets[i_jets];
    if(p.user_info<FlavHistory>().current_flavour()[5] != 0) btagged_pseudojetsIFN.push_back(p);
    if(p.user_info<FlavHistory>().current_flavour()[4] != 0) ctagged_pseudojetsIFN.push_back(p);
  }

  selected_pseudojets = all_pseudojets[1];
  for (unsigned int i_jets = 0; i_jets < selected_pseudojets.size(); i_jets++) {
    PseudoJet p = selected_pseudojets[i_jets];
    if(p.user_info<FlavHistory>().current_flavour()[5] != 0) btagged_pseudojetsCMP.push_back(p);
    if(p.user_info<FlavHistory>().current_flavour()[4] != 0) ctagged_pseudojetsCMP.push_back(p);
  }

  selected_pseudojets = all_pseudojets[2];
  for (unsigned int i_jets = 0; i_jets < selected_pseudojets.size(); i_jets++) {
    PseudoJet p = selected_pseudojets[i_jets];
    if(p.user_info<FlavHistory>().current_flavour()[5] != 0) btagged_pseudojetsGHS.push_back(p);
    if(p.user_info<FlavHistory>().current_flavour()[4] != 0) ctagged_pseudojetsGHS.push_back(p);
  }

  selected_pseudojets = all_pseudojets[3];
  for (unsigned int i_jets = 0; i_jets < selected_pseudojets.size(); i_jets++) {
    PseudoJet p = selected_pseudojets[i_jets];
    if(p.user_info<FlavHistory>().current_flavour()[5] != 0) btagged_pseudojetsSDF.push_back(p);
    if(p.user_info<FlavHistory>().current_flavour()[4] != 0) ctagged_pseudojetsSDF.push_back(p);
  }

  selected_pseudojets = all_pseudojets[4];
  for (unsigned int i_jets = 0; i_jets < selected_pseudojets.size(); i_jets++) {
    PseudoJet p = selected_pseudojets[i_jets];
    if(p.user_info<FlavHistory>().current_flavour()[5] != 0) btagged_pseudojetsAKT.push_back(p);
    if(p.user_info<FlavHistory>().current_flavour()[4] != 0) ctagged_pseudojetsAKT.push_back(p);
  }

  // match the tagged pseudojets to reco jets
  vector< vector<const PseudoJet*> > jetlabelIFN_b = m_doIFN ? match(btagged_pseudojetsIFN, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelIFN_c = m_doIFN ? match(ctagged_pseudojetsIFN, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelCMP_b = m_doCMP ? match(btagged_pseudojetsCMP, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelCMP_c = m_doCMP ? match(ctagged_pseudojetsCMP, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelGHS_b = m_doGHS ? match(btagged_pseudojetsGHS, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelGHS_c = m_doGHS ? match(ctagged_pseudojetsGHS, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelSDF_b = m_doSDF ? match(btagged_pseudojetsSDF, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelSDF_c = m_doSDF ? match(ctagged_pseudojetsSDF, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelAKT_b = m_doAKT ? match(btagged_pseudojetsAKT, jets) : vector< vector<const PseudoJet*> >(jets.size());
  vector< vector<const PseudoJet*> > jetlabelAKT_c = m_doAKT ? match(ctagged_pseudojetsAKT, jets) : vector< vector<const PseudoJet*> >(jets.size());

  for (unsigned int iJet = 0; iJet < jets.size(); iJet++) {
    const Jet& jet = *jets.at(iJet);
    if (jet.pt() < m_jetptmin) {
      m_ircsafelabeldecs->IFNsingleint(jet) = 0;
      m_ircsafelabeldecs->CMPsingleint(jet) = 0;
      m_ircsafelabeldecs->GHSsingleint(jet) = 0;
      m_ircsafelabeldecs->SDFsingleint(jet) = 0;
      m_ircsafelabeldecs->AKTsingleint(jet) = 0;
      continue;
    }

    // set truth label for jets above pt threshold
    // hierarchy: b > c > light
    ParticleJetTools::Tag_PseudoJets tag_pjet;
    tag_pjet.IFN_b = jetlabelIFN_b.at(iJet);
    tag_pjet.IFN_c = jetlabelIFN_c.at(iJet);

    tag_pjet.CMP_b = jetlabelCMP_b.at(iJet);
    tag_pjet.CMP_c = jetlabelCMP_c.at(iJet);

    tag_pjet.GHS_b = jetlabelGHS_b.at(iJet);
    tag_pjet.GHS_c = jetlabelGHS_c.at(iJet);

    tag_pjet.SDF_b = jetlabelSDF_b.at(iJet);
    tag_pjet.SDF_c = jetlabelSDF_c.at(iJet);

    tag_pjet.AKT_b = jetlabelAKT_b.at(iJet);
    tag_pjet.AKT_c = jetlabelAKT_c.at(iJet);

    setJetIRCSafeLabels(jet, tag_pjet, *m_ircsafelabeldecs, m_doIFN, m_doCMP, m_doGHS, m_doSDF, m_doAKT);
  }

  return StatusCode::SUCCESS;

}
