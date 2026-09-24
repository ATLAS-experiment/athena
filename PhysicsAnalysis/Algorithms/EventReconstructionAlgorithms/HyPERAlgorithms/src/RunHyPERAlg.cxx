/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <Math/Vector4D.h>
#include <math.h>

#include <vector>

#include "AthContainers/ConstDataVector.h"

// HyPER includes
#include "HyPERAlgorithms/RunHyPERAlg.h"
#include "HyPERAlgorithms/HyPERModel.h"
#include "HyPERAlgorithms/HyPERTtbarAllHadronicModel.h"
#include "HyPERAlgorithms/HyPERTtbarAllHadronicParser.h"
#include "HyPERAlgorithms/HyPERTtbarDiLeptonModel.h"
#include "HyPERAlgorithms/HyPERTtbarDiLeptonParser.h"
#include "HyPERAlgorithms/HyPERTtbarLJetsModel.h"
#include "HyPERAlgorithms/HyPERTtbarLJetsParser.h"
#include "HyPERAlgorithms/HyPERUtils.h"

namespace EventReco {

namespace {

PtEtaPhiMVector toPtEtaPhiM(const ROOT::Math::PtEtaPhiEVector& vec) {
  return PtEtaPhiMVector(vec.Pt(), vec.Eta(), vec.Phi(), vec.M());
}

ROOT::Math::PtEtaPhiEVector buildRecoP4(double pt, double eta, double phi,
                                        double e) {
  ROOT::Math::PtEtaPhiEVector vec;
  vec.SetCoordinates(pt, eta, phi, e);
  return vec;
}

}  // namespace

StatusCode RunHyPERAlg::initialize() {
  ANA_MSG_INFO("Initializing RunHyPER " << name());
  ANA_MSG_INFO("  --> topology: " << m_topology.value());
  ANA_MSG_INFO("  --> btagger: " << m_btagger.value());
  ANA_MSG_INFO("  --> debug level: " << static_cast<int>(this->msg().level()));
  ANA_MSG_INFO("  --> full log event number: " << m_fullLogEventNumber.value());

  // Retrieve the ONNX inference tools, one per cross-validation fold
  ANA_CHECK(m_onnxToolTrainedOnEven.retrieve());
  ANA_CHECK(m_onnxToolTrainedOnOdd.retrieve());

  // Load the b-tagging decoration
  m_btagDecorName = "ftag_quantile_" + m_btagger.value();
  m_bTagDecoAcc = std::make_unique<SG::ConstAccessor<int>>(m_btagDecorName);

  // Parse topology
  HyPERTopology hyperTopology = strToHyPERTopology(m_topology.value());
  if (hyperTopology == EventReco::HyPERTopology::NotSelected) {
    ANA_MSG_ERROR("Unrecognized HyPER topology: " << m_topology.value());
    return StatusCode::FAILURE;
  }
  m_hyperTopology = hyperTopology;
  m_ljetsUseBTag = (m_topology.value() == "TtbarLJets");

  // Initialise object containers
  ANA_CHECK(m_electronsHandle.initialize(m_systematicsList));
  ANA_CHECK(m_muonsHandle.initialize(m_systematicsList));
  ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));
  ANA_CHECK(m_metHandle.initialize(m_systematicsList));
  ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
  ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle,
                                           SG::AllowEmpty));
  ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle,
                                       SG::AllowEmpty));
  ANA_CHECK(m_jetSelection.initialize(m_systematicsList, m_jetsHandle,
                                      SG::AllowEmpty));
  // Intialise pre-selection
  ANA_CHECK(m_selection.initialize(m_systematicsList, m_eventInfoHandle,
                                   SG::AllowEmpty));

  // Decorations (topology dependent)
  if (hyperTopology == EventReco::HyPERTopology::TtbarAllHadronic) {
    ANA_CHECK(m_hyper_TtbarAllHadronic_Top1_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarAllHadronic_Top1_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarAllHadronic_Top2_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarAllHadronic_Top2_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarAllHadronic_W1_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarAllHadronic_W1_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarAllHadronic_W2_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarAllHadronic_W2_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_top_b_p4.initialize(m_systematicsList, m_eventInfoHandle,
                                    SG::AllowEmpty));
    ANA_CHECK(m_topbar_bbar_p4.initialize(m_systematicsList, m_eventInfoHandle,
                                          SG::AllowEmpty));
    ANA_CHECK(m_top_Wplus_decay0_p4.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_top_Wplus_decay1_p4.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_topbar_Wminus_decay0_p4.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_topbar_Wminus_decay1_p4.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
  }
  if (hyperTopology == EventReco::HyPERTopology::TtbarLJets) {
    ANA_CHECK(m_hyper_TtbarLJets_Classification_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_TopHad_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_TopLep_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_WHad_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_WLep_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_TopHad_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_TopLep_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_WHad_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_WLep_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_TopHad_IDs.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarLJets_TopLep_IDs.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_toplep_b_p4.initialize(m_systematicsList, m_eventInfoHandle,
                                       SG::AllowEmpty));
    ANA_CHECK(m_toplep_lep_p4.initialize(m_systematicsList, m_eventInfoHandle,
                                         SG::AllowEmpty));
    ANA_CHECK(m_tophad_b_p4.initialize(m_systematicsList, m_eventInfoHandle,
                                       SG::AllowEmpty));
    ANA_CHECK(m_tophad_w_decay0_p4.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_tophad_w_decay1_p4.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
  }
  if (hyperTopology == EventReco::HyPERTopology::TtbarDiLepton) {
    ANA_CHECK(m_hyper_TtbarDiLepton_Classification_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarDiLepton_Top1_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarDiLepton_Top2_Indices.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarDiLepton_Top1_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarDiLepton_Top2_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarDiLepton_Top1_IDs.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarDiLepton_Top2_IDs.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_hyper_TtbarDiLepton_HE_Score.initialize(
        m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_top_b_p4.initialize(m_systematicsList, m_eventInfoHandle,
                                    SG::AllowEmpty));
    ANA_CHECK(m_topbar_bbar_p4.initialize(m_systematicsList, m_eventInfoHandle,
                                          SG::AllowEmpty));
    ANA_CHECK(m_top_lep_p4.initialize(m_systematicsList, m_eventInfoHandle,
                                      SG::AllowEmpty));
    ANA_CHECK(m_topbar_lepbar_p4.initialize(m_systematicsList,
                                            m_eventInfoHandle, SG::AllowEmpty));
  }

  // Initialise systematics
  ANA_CHECK(m_systematicsList.initialize());

  // Initialise the correct HyPER model
  if (hyperTopology == EventReco::HyPERTopology::TtbarLJets) {
    ANA_MSG_INFO("Loading HyPER model.");
    m_hyperModel = std::make_unique<EventReco::HyPERTtbarLJetsModel>(
        &(*m_onnxToolTrainedOnEven), &(*m_onnxToolTrainedOnOdd));
    ANA_MSG_INFO("Loaded TtbarLJets HyPER model!");

    ANA_MSG_DEBUG("Initialising HyPERParser");
    m_hyperParser = std::make_unique<EventReco::HyPERTtbarLJetsParser>();
    ANA_MSG_DEBUG("Initialised HyPERParserTtbarLJets Parser");

  } else if (hyperTopology == EventReco::HyPERTopology::TtbarAllHadronic) {
    ANA_MSG_INFO("Loading HyPER model.");
    m_hyperModel = std::make_unique<EventReco::HyPERTtbarAllHadronicModel>(
        &(*m_onnxToolTrainedOnEven), &(*m_onnxToolTrainedOnOdd));
    ANA_MSG_INFO("Loaded TtbarAllHadronic HyPER model!");

    ANA_MSG_DEBUG("Initialising HyPERParser");
    m_hyperParser = std::make_unique<EventReco::HyPERTtbarAllHadronicParser>();
    ANA_MSG_DEBUG("Initialised HyPERParserTtbarAllHadronic Parser");
  } else if (hyperTopology == EventReco::HyPERTopology::TtbarDiLepton) {
    ANA_MSG_INFO("Loading HyPER model.");
    m_hyperModel = std::make_unique<EventReco::HyPERTtbarDiLeptonModel>(
        &(*m_onnxToolTrainedOnEven), &(*m_onnxToolTrainedOnOdd));
    ANA_MSG_INFO("Loaded TtbarDiLepton HyPER model!");

    ANA_MSG_DEBUG("Initialising HyPERParser");
    m_hyperParser = std::make_unique<EventReco::HyPERTtbarDiLeptonParser>();
    ANA_MSG_DEBUG("Initialised HyPERTtbarDiLepton Parser");
  }

  // Resolve the ONNX node names of the selected topology
  ANA_CHECK(m_hyperModel->initialize());

  // Initialisation of the HyPERGraph
  ANA_MSG_DEBUG("Initialising HyPERGraph");
  m_hyperGraph = std::make_unique<EventReco::HyPERGraph>();
  ANA_MSG_DEBUG("Initialised HyPERGraph");

  return StatusCode::SUCCESS;
}

bool RunHyPERAlg::buildDileptonPartialCandidate(const std::vector<int>& indices,
                                                const std::vector<int>& ids,
                                                PtEtaPhiMVector& bJetP4,
                                                PtEtaPhiMVector& leptonP4,
                                                float& analyserCharge) const {
  const int jetID = static_cast<int>(HyPERParticleID::jet);
  const int electronID = static_cast<int>(HyPERParticleID::e);
  const int muonID = static_cast<int>(HyPERParticleID::mu);

  if (indices.size() != 2 || ids.size() != 2)
    return false;

  int jetPosition = -1;
  int leptonPosition = -1;
  for (std::size_t i = 0; i < ids.size(); ++i) {
    if (ids.at(i) == jetID)
      jetPosition = static_cast<int>(i);
    if (ids.at(i) == electronID || ids.at(i) == muonID)
      leptonPosition = static_cast<int>(i);
  }

  if (jetPosition < 0 || leptonPosition < 0)
    return false;

  const int jetIndex = indices.at(jetPosition);
  const int leptonIndex = indices.at(leptonPosition);
  if (jetIndex < 0 || jetIndex >= static_cast<int>(m_hyperInputs.m_jets.size()))
    return false;

  const xAOD::Jet* jet = m_hyperInputs.m_jets.at(jetIndex);
  ROOT::Math::PtEtaPhiEVector jetP4 =
      buildRecoP4(jet->pt(), jet->eta(), jet->phi(), jet->e());

  ROOT::Math::PtEtaPhiEVector localLeptonP4;
  if (ids.at(leptonPosition) == electronID) {
    if (leptonIndex < 0 ||
        leptonIndex >= static_cast<int>(m_hyperInputs.m_electrons.size()))
      return false;
    const xAOD::Electron* electron = m_hyperInputs.m_electrons.at(leptonIndex);
    localLeptonP4 = buildRecoP4(electron->pt(), electron->eta(),
                                electron->phi(), electron->e());
    analyserCharge = electron->charge();
  } else {
    if (leptonIndex < 0 ||
        leptonIndex >= static_cast<int>(m_hyperInputs.m_muons.size()))
      return false;
    const xAOD::Muon* muon = m_hyperInputs.m_muons.at(leptonIndex);
    localLeptonP4 =
        buildRecoP4(muon->pt(), muon->eta(), muon->phi(), muon->e());
    analyserCharge = muon->charge();
  }

  bJetP4 = toPtEtaPhiM(jetP4);
  leptonP4 = toPtEtaPhiM(localLeptonP4);
  return true;
}

void RunHyPERAlg::buildTopP4TtbarAllHadronic(const std::vector<int>& topIndices,
                                             const std::vector<int>& wIndices,
                                             PtEtaPhiMVector& top_b_p4,
                                             PtEtaPhiMVector& top_W_decay0_p4,
                                             PtEtaPhiMVector& top_W_decay1_p4) {
  // Check we have enough indices
  if (topIndices.size() != 3 || wIndices.size() != 2) {
    top_b_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
    top_W_decay0_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
    top_W_decay1_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
    return;
  }

  // Check that W indices are within the top indices
  if (std::find(topIndices.begin(), topIndices.end(), wIndices[0]) ==
          topIndices.end() ||
      std::find(topIndices.begin(), topIndices.end(), wIndices[1]) ==
          topIndices.end()) {
    top_b_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
    top_W_decay0_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
    top_W_decay1_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
    return;
  }

  const xAOD::Jet* leadWJet = nullptr;
  const xAOD::Jet* subleadWJet = nullptr;

  // Filter good W indices (-1 is bad)
  std::vector<int> goodWIndices;
  for (const auto idx : wIndices) {
    if (idx != -1) {
      goodWIndices.push_back(idx);
    }
  }
  // If we only have one good W index, by definition is the leading W decay
  if (goodWIndices.size() == 1) {
    int leadingWIndex = goodWIndices[0];
    leadWJet = m_hyperInputs.m_jets.at(leadingWIndex);
  }
  // If there are two good W indices, we need to determine which is leading and
  // subleading
  else if (goodWIndices.size() == 2) {
    int leadingWIndex = goodWIndices[0];
    int subleadingWIndex = goodWIndices[1];
    const xAOD::Jet* jet1 = m_hyperInputs.m_jets.at(leadingWIndex);
    const xAOD::Jet* jet2 = m_hyperInputs.m_jets.at(subleadingWIndex);
    if (jet1->pt() >= jet2->pt()) {
      leadWJet = jet1;
      subleadWJet = jet2;
    } else {
      leadWJet = jet2;
      subleadWJet = jet1;
    }
  }
  // Take out the b-jet index as the one not in the W indices
  int bJetIndex = -1;
  for (const auto idx : topIndices) {
    if (std::find(wIndices.begin(), wIndices.end(), idx) == wIndices.end()) {
      bJetIndex = idx;
      break;
    }
  }
  const xAOD::Jet* bJet =
      (bJetIndex != -1) ? m_hyperInputs.m_jets.at(bJetIndex) : nullptr;

  // Build the 4-vectors
  top_b_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
  top_W_decay0_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
  top_W_decay1_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);

  if (bJet) {
    ROOT::Math::PtEtaPhiEVector bJetP4 =
        buildRecoP4(bJet->pt(), bJet->eta(), bJet->phi(), bJet->e());
    top_b_p4 = toPtEtaPhiM(bJetP4);
  }
  if (leadWJet) {
    ROOT::Math::PtEtaPhiEVector leadWJetP4 = buildRecoP4(
        leadWJet->pt(), leadWJet->eta(), leadWJet->phi(), leadWJet->e());
    top_W_decay0_p4 = toPtEtaPhiM(leadWJetP4);
  }
  if (subleadWJet) {
    ROOT::Math::PtEtaPhiEVector subleadWJetP4 =
        buildRecoP4(subleadWJet->pt(), subleadWJet->eta(), subleadWJet->phi(),
                    subleadWJet->e());
    top_W_decay1_p4 = toPtEtaPhiM(subleadWJetP4);
  }
}

void RunHyPERAlg::buildTopP4TtbarLJets(
    const std::vector<int>& topHadIndices, const std::vector<int>& wHadIndices,
    const std::vector<int>& topLepIndices, const std::vector<int>& topLepIDs,
    PtEtaPhiMVector& tophad_b_p4, PtEtaPhiMVector& tophad_w_decay0_p4,
    PtEtaPhiMVector& tophad_w_decay1_p4, PtEtaPhiMVector& toplep_b_p4,
    PtEtaPhiMVector& toplep_lep_p4) {
  tophad_b_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
  tophad_w_decay0_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
  tophad_w_decay1_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
  toplep_b_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);
  toplep_lep_p4.SetCoordinates(0.0, 0.0, 0.0, 0.0);

  // Hadronic top 4-vectors
  buildTopP4TtbarAllHadronic(topHadIndices, wHadIndices, tophad_b_p4,
                             tophad_w_decay0_p4, tophad_w_decay1_p4);

  // Leptonic top 4-vectors
  if (topLepIndices.size() == 3 && topLepIDs.size() == 3) {
    const int jetID = static_cast<int>(HyPERParticleID::jet);
    const int electronID = static_cast<int>(HyPERParticleID::e);
    const int muonID = static_cast<int>(HyPERParticleID::mu);

    for (std::size_t i = 0; i < 3; ++i) {
      int idx = topLepIndices[i];
      int id = topLepIDs[i];
      if (idx < 0)
        continue;

      if (id == jetID) {
        if (idx < static_cast<int>(m_hyperInputs.m_jets.size())) {
          const xAOD::Jet* jet = m_hyperInputs.m_jets.at(idx);
          toplep_b_p4 = toPtEtaPhiM(
              buildRecoP4(jet->pt(), jet->eta(), jet->phi(), jet->e()));
        }
      } else if (id == electronID) {
        if (idx < static_cast<int>(m_hyperInputs.m_electrons.size())) {
          const xAOD::Electron* el = m_hyperInputs.m_electrons.at(idx);
          toplep_lep_p4 =
              toPtEtaPhiM(buildRecoP4(el->pt(), el->eta(), el->phi(), el->e()));
        }
      } else if (id == muonID) {
        if (idx < static_cast<int>(m_hyperInputs.m_muons.size())) {
          const xAOD::Muon* mu = m_hyperInputs.m_muons.at(idx);
          toplep_lep_p4 =
              toPtEtaPhiM(buildRecoP4(mu->pt(), mu->eta(), mu->phi(), mu->e()));
        }
      }
    }
  }
}

StatusCode RunHyPERAlg::execute(const EventContext& ctx) {
  auto invalidRecoParton = []() {
    PtEtaPhiMVector vec;
    vec.SetCoordinates(0.0, 0.0, 0.0, 0.0);
    return vec;
  };
  // Loop over systematics
  for (const auto& sys : m_systematicsList.systematicsVector()) {
    // Check the event selection
    const xAOD::EventInfo* evtInfo = nullptr;
    ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys, ctx));

    // If user sets DEBUG the idea is the user will see general things about the
    // code running for all events. If user sets VERBOSE the idea is the user
    // will see very detailed information for one specific event.
    if (m_fullLogEventNumber.value() != 0) {  // User wants to log a particular event.
      if (evtInfo->eventNumber() == m_fullLogEventNumber.value()) {
        this->msg().setLevel(MSG::VERBOSE);
        g_hyper_msg_level = MSG::VERBOSE;
      } else {  // We don't want other events to pollute the log.
        this->msg().setLevel(MSG::INFO);
        g_hyper_msg_level = MSG::INFO;
      }
    }

    ANA_MSG_DEBUG("Event number.... : " << evtInfo->eventNumber());

    // Every CP::SysWriteDecorHandle must be locked before the output stream
    // flushes, otherwise copyAuxStoreThinned warns about unlocked decorations.
    // Invoked on every path that leaves this systematic iteration.
    auto lockAllDecorations = [&]() {
      if (m_hyperTopology == EventReco::HyPERTopology::TtbarAllHadronic) {
        m_hyper_TtbarAllHadronic_Top1_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarAllHadronic_Top1_Score.lock(*evtInfo, sys);
        m_hyper_TtbarAllHadronic_Top2_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarAllHadronic_Top2_Score.lock(*evtInfo, sys);
        m_hyper_TtbarAllHadronic_W1_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarAllHadronic_W1_Score.lock(*evtInfo, sys);
        m_hyper_TtbarAllHadronic_W2_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarAllHadronic_W2_Score.lock(*evtInfo, sys);
        if (!m_top_b_p4.empty()) m_top_b_p4.lock(*evtInfo, sys);
        if (!m_topbar_bbar_p4.empty()) m_topbar_bbar_p4.lock(*evtInfo, sys);
        if (!m_top_Wplus_decay0_p4.empty()) m_top_Wplus_decay0_p4.lock(*evtInfo, sys);
        if (!m_top_Wplus_decay1_p4.empty()) m_top_Wplus_decay1_p4.lock(*evtInfo, sys);
        if (!m_topbar_Wminus_decay0_p4.empty()) m_topbar_Wminus_decay0_p4.lock(*evtInfo, sys);
        if (!m_topbar_Wminus_decay1_p4.empty()) m_topbar_Wminus_decay1_p4.lock(*evtInfo, sys);
      }
      if (m_hyperTopology == EventReco::HyPERTopology::TtbarLJets) {
        m_hyper_TtbarLJets_Classification_Score.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_TopHad_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_TopLep_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_WHad_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_WLep_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_TopHad_Score.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_TopLep_Score.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_WHad_Score.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_WLep_Score.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_TopHad_IDs.lock(*evtInfo, sys);
        m_hyper_TtbarLJets_TopLep_IDs.lock(*evtInfo, sys);
        if (!m_toplep_b_p4.empty()) m_toplep_b_p4.lock(*evtInfo, sys);
        if (!m_toplep_lep_p4.empty()) m_toplep_lep_p4.lock(*evtInfo, sys);
        if (!m_tophad_b_p4.empty()) m_tophad_b_p4.lock(*evtInfo, sys);
        if (!m_tophad_w_decay0_p4.empty()) m_tophad_w_decay0_p4.lock(*evtInfo, sys);
        if (!m_tophad_w_decay1_p4.empty()) m_tophad_w_decay1_p4.lock(*evtInfo, sys);
      }
      if (m_hyperTopology == EventReco::HyPERTopology::TtbarDiLepton) {
        m_hyper_TtbarDiLepton_Classification_Score.lock(*evtInfo, sys);
        m_hyper_TtbarDiLepton_Top1_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarDiLepton_Top2_Indices.lock(*evtInfo, sys);
        m_hyper_TtbarDiLepton_Top1_Score.lock(*evtInfo, sys);
        m_hyper_TtbarDiLepton_Top2_Score.lock(*evtInfo, sys);
        m_hyper_TtbarDiLepton_Top1_IDs.lock(*evtInfo, sys);
        m_hyper_TtbarDiLepton_Top2_IDs.lock(*evtInfo, sys);
        m_hyper_TtbarDiLepton_HE_Score.lock(*evtInfo, sys);
        if (!m_top_b_p4.empty()) m_top_b_p4.lock(*evtInfo, sys);
        if (!m_topbar_bbar_p4.empty()) m_topbar_bbar_p4.lock(*evtInfo, sys);
        if (!m_top_lep_p4.empty()) m_top_lep_p4.lock(*evtInfo, sys);
        if (!m_topbar_lepbar_p4.empty()) m_topbar_lepbar_p4.lock(*evtInfo, sys);
      }
    };


    // Default decoration values (topology dependent)
    const HyPERTopology hyperTopology = m_hyperTopology;
    if (hyperTopology == EventReco::HyPERTopology::TtbarAllHadronic) {
      m_hyper_TtbarAllHadronic_Top1_Indices.set(
          *evtInfo, std::vector<int>{-1, -1, -1}, sys);
      m_hyper_TtbarAllHadronic_Top1_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarAllHadronic_Top2_Indices.set(
          *evtInfo, std::vector<int>{-1, -1, -1}, sys);
      m_hyper_TtbarAllHadronic_Top2_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarAllHadronic_W1_Indices.set(
          *evtInfo, std::vector<int>{-1, -1, -1}, sys);
      m_hyper_TtbarAllHadronic_W1_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarAllHadronic_W2_Indices.set(
          *evtInfo, std::vector<int>{-1, -1, -1}, sys);
      m_hyper_TtbarAllHadronic_W2_Score.set(*evtInfo, -1, sys);
      if (!m_top_b_p4.empty())
        m_top_b_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_topbar_bbar_p4.empty())
        m_topbar_bbar_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_top_Wplus_decay0_p4.empty())
        m_top_Wplus_decay0_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_top_Wplus_decay1_p4.empty())
        m_top_Wplus_decay1_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_topbar_Wminus_decay0_p4.empty())
        m_topbar_Wminus_decay0_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_topbar_Wminus_decay1_p4.empty())
        m_topbar_Wminus_decay1_p4.set(*evtInfo, invalidRecoParton(), sys);
    }
    if (hyperTopology == EventReco::HyPERTopology::TtbarLJets) {
      m_hyper_TtbarLJets_Classification_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarLJets_TopHad_Indices.set(*evtInfo,
                                            std::vector<int>{-1, -1, -1}, sys);
      m_hyper_TtbarLJets_TopLep_Indices.set(*evtInfo,
                                            std::vector<int>{-1, -1, -1}, sys);
      m_hyper_TtbarLJets_WHad_Indices.set(*evtInfo, std::vector<int>{-1, -1},
                                          sys);
      m_hyper_TtbarLJets_WLep_Indices.set(*evtInfo, std::vector<int>{-1, -1},
                                          sys);
      m_hyper_TtbarLJets_TopHad_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarLJets_TopLep_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarLJets_WHad_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarLJets_WLep_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarLJets_TopHad_IDs.set(*evtInfo, std::vector<int>{-1, -1, -1},
                                        sys);
      m_hyper_TtbarLJets_TopLep_IDs.set(*evtInfo, std::vector<int>{-1, -1, -1},
                                        sys);
      if (!m_toplep_b_p4.empty())
        m_toplep_b_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_toplep_lep_p4.empty())
        m_toplep_lep_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_tophad_b_p4.empty())
        m_tophad_b_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_tophad_w_decay0_p4.empty())
        m_tophad_w_decay0_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_tophad_w_decay1_p4.empty())
        m_tophad_w_decay1_p4.set(*evtInfo, invalidRecoParton(), sys);
    }
    if (hyperTopology == EventReco::HyPERTopology::TtbarDiLepton) {
      m_hyper_TtbarDiLepton_Classification_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarDiLepton_Top1_Indices.set(*evtInfo, std::vector<int>{-1, -1},
                                             sys);
      m_hyper_TtbarDiLepton_Top2_Indices.set(*evtInfo, std::vector<int>{-1, -1},
                                             sys);
      m_hyper_TtbarDiLepton_Top1_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarDiLepton_Top2_Score.set(*evtInfo, -1, sys);
      m_hyper_TtbarDiLepton_Top1_IDs.set(*evtInfo, std::vector<int>{-1, -1},
                                         sys);
      m_hyper_TtbarDiLepton_Top2_IDs.set(*evtInfo, std::vector<int>{-1, -1},
                                         sys);
      m_hyper_TtbarDiLepton_HE_Score.set(*evtInfo, -1, sys);
      if (!m_top_b_p4.empty())
        m_top_b_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_topbar_bbar_p4.empty())
        m_topbar_bbar_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_top_lep_p4.empty())
        m_top_lep_p4.set(*evtInfo, invalidRecoParton(), sys);
      if (!m_topbar_lepbar_p4.empty())
        m_topbar_lepbar_p4.set(*evtInfo, invalidRecoParton(), sys);
    }

    if (m_selection && !m_selection.getBool(*evtInfo, sys)) {
      lockAllDecorations();
      continue;
    }

    // Read the containers
    const xAOD::ElectronContainer* electrons = nullptr;
    ANA_CHECK(m_electronsHandle.retrieve(electrons, sys, ctx));
    const xAOD::MuonContainer* muons = nullptr;
    ANA_CHECK(m_muonsHandle.retrieve(muons, sys, ctx));
    const xAOD::JetContainer* jets = nullptr;
    ANA_CHECK(m_jetsHandle.retrieve(jets, sys, ctx));
    const xAOD::MissingETContainer* met = nullptr;
    ANA_CHECK(m_metHandle.retrieve(met, sys, ctx));

    // Apply object-wise selection
    ConstDataVector<xAOD::ElectronContainer> selected_electrons(
        SG::VIEW_ELEMENTS);
    ConstDataVector<xAOD::MuonContainer> selected_muons(SG::VIEW_ELEMENTS);
    ConstDataVector<xAOD::JetContainer> selected_jets(SG::VIEW_ELEMENTS);
    std::vector<int> selected_jets_btag;

    ANA_MSG_DEBUG("Building particle containers.");

    ANA_MSG_DEBUG("Building electrons container.");
    for (const xAOD::Electron* el : *electrons) {
      if (m_electronSelection.getBool(*el, sys))
        selected_electrons.push_back(el);
    }

    ANA_MSG_DEBUG("Building muons container.");
    for (const xAOD::Muon* mu : *muons) {
      if (m_muonSelection.getBool(*mu, sys))
        selected_muons.push_back(mu);
    }

    ANA_MSG_DEBUG("Building jets container.");
    for (const xAOD::Jet* jet : *jets) {
      if (m_jetSelection.getBool(*jet, sys))
        selected_jets.push_back(jet);
    }

    // Build the HyPER graph
    m_hyperInputs.m_electrons = selected_electrons;
    m_hyperInputs.m_muons = selected_muons;
    m_hyperInputs.m_jets = selected_jets;
    m_hyperInputs.m_met = met;
    ANA_MSG_DEBUG("Building graph.");
    ANA_CHECK(this->buildGraph());
    ANA_MSG_DEBUG("Graph building successful.");

    if (this->msg().level() == MSG::VERBOSE) {
      ANA_MSG_INFO("Printing the graph...");
      m_hyperGraph->printGraph();
    }

    // Parse the inputs to the HyPER algorithm
    ANA_MSG_DEBUG("Building the ONNX inputs.");
    m_hyperParser->buildONNXInputs(*m_hyperGraph, *m_hyperModel);
    ANA_MSG_DEBUG("ONNX inputs building successful.");

    // Run the HyPER algorithm
    ANA_MSG_DEBUG("Evaluating model.");
    unsigned int modelToUse;
    // The model TRAINED on even should always be at position [0] in the
    // m_session vector so, m_session[1] should be used for even events
    modelToUse = evtInfo->eventNumber() % 2 == 0 ? 1 : 0;

    ANA_MSG_DEBUG("Using model: " << modelToUse);
    ANA_CHECK(m_hyperModel->evaluate(modelToUse));
    ANA_MSG_DEBUG("Model evaluation successful.");

    if (this->msg().level() == MSG::VERBOSE) {
      ANA_MSG_INFO("Printing the ONNX inputs:");
      m_hyperModel->printInputInfo(true);
      ANA_MSG_INFO("Printing the ONNX outputs:");
      m_hyperModel->printOutputInfo(true);
    }

    // Process the outputs
    ANA_MSG_DEBUG("Extracting outputs from model.");
    m_hyperParser->getONNXOutputs(*m_hyperModel);
    ANA_MSG_DEBUG("Outputs extraction successful.");

    ANA_MSG_DEBUG("Performing output reconstruction.");
    m_hyperParser->reconstructOutputs(*m_hyperGraph);
    ANA_MSG_DEBUG("Output reconstruction successful.");

    // Print the inputs/outputs for PyTorch/TCT validation
    if (m_fullLogEventNumber.value() != 0 &&
        evtInfo->eventNumber() == m_fullLogEventNumber.value()) {
      m_hyperGraph->printGraphInputsForValidation();
      m_hyperParser->printOutputsForValidation();
    }

    // Write the outputs
    ANA_MSG_DEBUG("Writing the outputs.");

    if (hyperTopology == EventReco::HyPERTopology::TtbarAllHadronic) {
      std::vector<std::string> recoLabels = m_hyperParser->getLabels();
      std::vector<std::vector<int>> recoIndices = m_hyperParser->getIndices();
      std::vector<float> recoScores = m_hyperParser->getScores();
      // HyPER_Reco_Top1
      std::size_t index = getIndexFromLabel(recoLabels, "HyPER_Reco_Top1");
      std::vector<int> top1Indices(recoIndices[index]);
      m_hyper_TtbarAllHadronic_Top1_Indices.set(*evtInfo, top1Indices, sys);
      m_hyper_TtbarAllHadronic_Top1_Score.set(*evtInfo, recoScores[index], sys);
      // HyPER_Reco_Top2
      index = getIndexFromLabel(recoLabels, "HyPER_Reco_Top2");
      std::vector<int> top2Indices(recoIndices[index]);
      m_hyper_TtbarAllHadronic_Top2_Indices.set(*evtInfo, top2Indices, sys);
      m_hyper_TtbarAllHadronic_Top2_Score.set(*evtInfo, recoScores[index], sys);
      // HyPER_Reco_W1
      index = getIndexFromLabel(recoLabels, "HyPER_Reco_W1");
      std::vector<int> w1Indices(recoIndices[index]);
      m_hyper_TtbarAllHadronic_W1_Indices.set(*evtInfo, w1Indices, sys);
      m_hyper_TtbarAllHadronic_W1_Score.set(*evtInfo, recoScores[index], sys);
      // HyPER_Reco_W2
      index = getIndexFromLabel(recoLabels, "HyPER_Reco_W2");
      std::vector<int> w2Indices(recoIndices[index]);
      m_hyper_TtbarAllHadronic_W2_Indices.set(*evtInfo, w2Indices, sys);
      m_hyper_TtbarAllHadronic_W2_Score.set(*evtInfo, recoScores[index], sys);
      // Build the top 4-vectors
      // Top1
      PtEtaPhiMVector top_b_p4 = invalidRecoParton();
      PtEtaPhiMVector top_Wplus_decay0_p4 = invalidRecoParton();
      PtEtaPhiMVector top_Wplus_decay1_p4 = invalidRecoParton();
      buildTopP4TtbarAllHadronic(top1Indices, w1Indices, top_b_p4,
                                 top_Wplus_decay0_p4, top_Wplus_decay1_p4);
      if (!m_top_b_p4.empty())
        m_top_b_p4.set(*evtInfo, top_b_p4, sys);
      if (!m_top_Wplus_decay0_p4.empty())
        m_top_Wplus_decay0_p4.set(*evtInfo, top_Wplus_decay0_p4, sys);
      if (!m_top_Wplus_decay1_p4.empty())
        m_top_Wplus_decay1_p4.set(*evtInfo, top_Wplus_decay1_p4, sys);
      // Top2
      PtEtaPhiMVector topbar_bbar_p4 = invalidRecoParton();
      PtEtaPhiMVector topbar_Wminus_decay0_p4 = invalidRecoParton();
      PtEtaPhiMVector topbar_Wminus_decay1_p4 = invalidRecoParton();
      buildTopP4TtbarAllHadronic(top2Indices, w2Indices, topbar_bbar_p4,
                                 topbar_Wminus_decay0_p4,
                                 topbar_Wminus_decay1_p4);
      if (!m_topbar_bbar_p4.empty())
        m_topbar_bbar_p4.set(*evtInfo, topbar_bbar_p4, sys);
      if (!m_topbar_Wminus_decay0_p4.empty())
        m_topbar_Wminus_decay0_p4.set(*evtInfo, topbar_Wminus_decay0_p4, sys);
      if (!m_topbar_Wminus_decay1_p4.empty())
        m_topbar_Wminus_decay1_p4.set(*evtInfo, topbar_Wminus_decay1_p4, sys);
    }
    if (hyperTopology == EventReco::HyPERTopology::TtbarLJets) {
      std::vector<std::string> recoLabels = m_hyperParser->getLabels();
      std::vector<std::vector<int>> recoIndices = m_hyperParser->getIndices();
      std::vector<float> recoScores = m_hyperParser->getScores();
      std::vector<std::vector<int>> recoIDs = m_hyperParser->getIds();
      float class_score = m_hyperParser->getClassificationScore();
      // Classification score
      m_hyper_TtbarLJets_Classification_Score.set(*evtInfo, class_score, sys);
      // HyPER_Reco_TopHad
      std::size_t indexTopHad =
          getIndexFromLabel(recoLabels, "HyPER_Reco_TopHad");
      ANA_MSG_DEBUG("Index of HyPER_Reco_TopHad: " << indexTopHad);
      std::vector<int> topHadIndices = recoIndices[indexTopHad];
      m_hyper_TtbarLJets_TopHad_Indices.set(*evtInfo, topHadIndices, sys);
      m_hyper_TtbarLJets_TopHad_Score.set(*evtInfo, recoScores[indexTopHad],
                                          sys);
      m_hyper_TtbarLJets_TopHad_IDs.set(*evtInfo, recoIDs[indexTopHad], sys);
      // HyPER_Reco_TopLep
      std::size_t indexTopLep =
          getIndexFromLabel(recoLabels, "HyPER_Reco_TopLep");
      ANA_MSG_DEBUG("Index of HyPER_Reco_TopLep: " << indexTopLep);
      std::vector<int> topLepIndices = recoIndices[indexTopLep];
      std::vector<int> topLepIDs = recoIDs[indexTopLep];
      m_hyper_TtbarLJets_TopLep_Indices.set(*evtInfo, topLepIndices, sys);
      m_hyper_TtbarLJets_TopLep_Score.set(*evtInfo, recoScores[indexTopLep],
                                          sys);
      m_hyper_TtbarLJets_TopLep_IDs.set(*evtInfo, topLepIDs, sys);
      // HyPER_Reco_WHad
      std::size_t indexWHad = getIndexFromLabel(recoLabels, "HyPER_Reco_WHad");
      ANA_MSG_DEBUG("Index of HyPER_Reco_WHad: " << indexWHad);
      std::vector<int> wHadIndices = recoIndices[indexWHad];
      m_hyper_TtbarLJets_WHad_Indices.set(*evtInfo, wHadIndices, sys);
      m_hyper_TtbarLJets_WHad_Score.set(*evtInfo, recoScores[indexWHad], sys);
      // HyPER_Reco_WLep
      std::size_t indexWLep = getIndexFromLabel(recoLabels, "HyPER_Reco_WLep");
      ANA_MSG_DEBUG("Index of HyPER_Reco_WLep: " << indexWLep);
      m_hyper_TtbarLJets_WLep_Indices.set(*evtInfo, recoIndices[indexWLep],
                                          sys);
      m_hyper_TtbarLJets_WLep_Score.set(*evtInfo, recoScores[indexWLep], sys);

      // Build 4-vectors for TtbarLJets
      PtEtaPhiMVector tophad_b_p4 = invalidRecoParton();
      PtEtaPhiMVector tophad_w_decay0_p4 = invalidRecoParton();
      PtEtaPhiMVector tophad_w_decay1_p4 = invalidRecoParton();
      PtEtaPhiMVector toplep_b_p4 = invalidRecoParton();
      PtEtaPhiMVector toplep_lep_p4 = invalidRecoParton();

      buildTopP4TtbarLJets(topHadIndices, wHadIndices, topLepIndices, topLepIDs,
                           tophad_b_p4, tophad_w_decay0_p4, tophad_w_decay1_p4,
                           toplep_b_p4, toplep_lep_p4);

      if (!m_tophad_b_p4.empty())
        m_tophad_b_p4.set(*evtInfo, tophad_b_p4, sys);
      if (!m_tophad_w_decay0_p4.empty())
        m_tophad_w_decay0_p4.set(*evtInfo, tophad_w_decay0_p4, sys);
      if (!m_tophad_w_decay1_p4.empty())
        m_tophad_w_decay1_p4.set(*evtInfo, tophad_w_decay1_p4, sys);
      if (!m_toplep_b_p4.empty())
        m_toplep_b_p4.set(*evtInfo, toplep_b_p4, sys);
      if (!m_toplep_lep_p4.empty())
        m_toplep_lep_p4.set(*evtInfo, toplep_lep_p4, sys);
    }
    if (hyperTopology == EventReco::HyPERTopology::TtbarDiLepton) {
      std::vector<std::string> recoLabels = m_hyperParser->getLabels();
      std::vector<std::vector<int>> recoIndices = m_hyperParser->getIndices();
      std::vector<float> recoScores = m_hyperParser->getScores();
      std::vector<std::vector<int>> recoIDs = m_hyperParser->getIds();
      float class_score = m_hyperParser->getClassificationScore();
      PtEtaPhiMVector topBP4 = invalidRecoParton();
      PtEtaPhiMVector topbarBbarP4 = invalidRecoParton();
      PtEtaPhiMVector topLepP4 = invalidRecoParton();
      PtEtaPhiMVector topbarLepbarP4 = invalidRecoParton();
      // Classification score
      m_hyper_TtbarDiLepton_Classification_Score.set(*evtInfo, class_score,
                                                     sys);
      // HyPER_Reco_Top1
      std::size_t index = getIndexFromLabel(recoLabels, "HyPER_Reco_Top1");
      m_hyper_TtbarDiLepton_Top1_Indices.set(*evtInfo, recoIndices[index], sys);
      m_hyper_TtbarDiLepton_Top1_Score.set(*evtInfo, recoScores[index], sys);
      m_hyper_TtbarDiLepton_Top1_IDs.set(*evtInfo, recoIDs[index], sys);

      PtEtaPhiMVector candidateBJetP4 = invalidRecoParton();
      PtEtaPhiMVector candidateLeptonP4 = invalidRecoParton();
      float analyserCharge = 0.;
      if (buildDileptonPartialCandidate(recoIndices[index], recoIDs[index],
                                        candidateBJetP4, candidateLeptonP4,
                                        analyserCharge)) {
        if (analyserCharge > 0.) {
          topBP4 = candidateBJetP4;
          topLepP4 = candidateLeptonP4;
        } else if (analyserCharge < 0.) {
          topbarBbarP4 = candidateBJetP4;
          topbarLepbarP4 = candidateLeptonP4;
        }
      }

      // HyPER_Reco_Top2
      index = getIndexFromLabel(recoLabels, "HyPER_Reco_Top2");
      m_hyper_TtbarDiLepton_Top2_Indices.set(*evtInfo, recoIndices[index], sys);
      m_hyper_TtbarDiLepton_Top2_Score.set(*evtInfo, recoScores[index], sys);
      m_hyper_TtbarDiLepton_Top2_IDs.set(*evtInfo, recoIDs[index], sys);

      candidateBJetP4 = invalidRecoParton();
      candidateLeptonP4 = invalidRecoParton();
      analyserCharge = 0.;
      if (buildDileptonPartialCandidate(recoIndices[index], recoIDs[index],
                                        candidateBJetP4, candidateLeptonP4,
                                        analyserCharge)) {
        if (analyserCharge > 0.) {
          topBP4 = candidateBJetP4;
          topLepP4 = candidateLeptonP4;
        } else if (analyserCharge < 0.) {
          topbarBbarP4 = candidateBJetP4;
          topbarLepbarP4 = candidateLeptonP4;
        }
      }

      // HyPER_HE_Score
      index = getIndexFromLabel(recoLabels, "HyPER_Reco_HE");
      m_hyper_TtbarDiLepton_HE_Score.set(*evtInfo, recoScores[index], sys);

      if (!m_top_b_p4.empty())
        m_top_b_p4.set(*evtInfo, topBP4, sys);
      if (!m_topbar_bbar_p4.empty())
        m_topbar_bbar_p4.set(*evtInfo, topbarBbarP4, sys);
      if (!m_top_lep_p4.empty())
        m_top_lep_p4.set(*evtInfo, topLepP4, sys);
      if (!m_topbar_lepbar_p4.empty())
        m_topbar_lepbar_p4.set(*evtInfo, topbarLepbarP4, sys);
    }

    // Clear the graph and other relevant objects for the next event
    ANA_MSG_DEBUG("Cleaning HyPER objects for next event.");
    m_hyperGraph->clearGraph();
    m_hyperModel->clearInputs();
    m_hyperModel->clearOutputs();
    m_hyperParser->clear();
    m_hyperInputs.clear();
    ANA_MSG_DEBUG("Clean successful.");

    lockAllDecorations();
  }
  return StatusCode::SUCCESS;
}

StatusCode RunHyPERAlg::buildGraph() {
  HyPERTopology topology = m_hyperModel->getTopology();
  if (topology == EventReco::HyPERTopology::NotSelected) {
    ANA_MSG_ERROR("Bad topology while building HyPER graph");
    return StatusCode::FAILURE;
  }

  using Features = std::vector<float>;

  // Remember... For HyPER we use the following particle IDs -> jet=1 e=2 mu=3
  // met=4 tau=5 Also... VERY important! The order of filling the particles for
  // the graph is important! We use this later for the reconstruction. The order
  // is: jets -> electrons -> muons -> met -> taus
  // TODO: Write a checker function for the ordering.

  if (topology == EventReco::HyPERTopology::TtbarLJets)
    return this->buildTtbarLJetsGraph();
  else if (topology == EventReco::HyPERTopology::TtbarAllHadronic)
    return this->buildTtbarAllHadronicGraph();
  else if (topology == EventReco::HyPERTopology::TtbarDiLepton)
    return this->buildTtbarDiLeptonGraph();
  return StatusCode::FAILURE;  // No topology selected, this is bad behaviour.
}

StatusCode RunHyPERAlg::buildTtbarLJetsGraph() {
  // For this topology we use jets, electrons and muons:
  // [E, eta, phi, pT, charge, particleID]
  // And Edges with the following features:
  // [dEta, dPhi, dR, m(i,j)]
  // And, the following globals:
  // [nJets, nBTagJets]

  // Build particles
  std::vector<Features> particles;
  float nBJets90 =
      0;  // Store number of bjets per-quantile for the global features.
  float nBJets85 = 0;
  float nBJets77 = 0;
  float nBJets70 = 0;
  float nBJets65 = 0;
  float nJets = 0;  // Store number of jets for the global features.
  for (const xAOD::Jet* jet : m_hyperInputs.m_jets) {
    nJets += 1.0f;
    Features feats;
    if (m_ljetsUseBTag) {
      // Get b-tagging decision.
      if (!m_bTagDecoAcc->isAvailable(*jet)) {
        ANA_MSG_ERROR("HyPERAlg:: the jets do not have " << m_btagDecorName
                                                         << " aux variable!");
        return StatusCode::FAILURE;
      }
      int bTagQuantile = (*m_bTagDecoAcc)(*jet);
      if (bTagQuantile >= 2)
        nBJets90 += 1;
      if (bTagQuantile >= 3)
        nBJets85 += 1;
      if (bTagQuantile >= 4)
        nBJets77 += 1;
      if (bTagQuantile >= 5)
        nBJets70 += 1;
      if (bTagQuantile >= 6)
        nBJets65 += 1;
      feats = {float(jet->e() / 1000),
               float(jet->eta()),
               float(jet->phi()),
               float(jet->pt() / 1000),
               float(bTagQuantile),
               0.f,
               float(EventReco::HyPERParticleID::jet),
               1.f};  // TODO: This is needed because the training was
                      // done with new HyPER dataset.
    } else {
      feats = {float(jet->e() / 1000),
               float(jet->eta()),
               float(jet->phi()),
               float(jet->pt() / 1000),
               0.f,
               float(EventReco::HyPERParticleID::jet),
               1.f};  // TODO: This is needed because the training was
                      // done with new HyPER dataset.
    }
    particles.push_back(feats);
  }
  for (const xAOD::Electron* el : m_hyperInputs.m_electrons) {
    Features feats;
    if (m_ljetsUseBTag) {
      feats = {float(el->e() / 1000),
               float(el->eta()),
               float(el->phi()),
               float(el->pt() / 1000),
               0.f,
               float(el->charge()),
               float(EventReco::HyPERParticleID::e),
               2.f};
    } else {
      feats = {float(el->e() / 1000),
               float(el->eta()),
               float(el->phi()),
               float(el->pt() / 1000),
               float(el->charge()),
               float(EventReco::HyPERParticleID::e),
               2.f};  // TODO: This is needed because the training was
                      // done with new HyPER dataset.
    }
    particles.push_back(feats);
  }
  for (const xAOD::Muon* mu : m_hyperInputs.m_muons) {
    Features feats;
    if (m_ljetsUseBTag) {
      feats = {float(mu->e() / 1000),
               float(mu->eta()),
               float(mu->phi()),
               float(mu->pt() / 1000),
               0.f,
               float(mu->charge()),
               float(EventReco::HyPERParticleID::mu),
               2.f};
    } else {
      feats = {float(mu->e() / 1000),
               float(mu->eta()),
               float(mu->phi()),
               float(mu->pt() / 1000),
               float(mu->charge()),
               float(EventReco::HyPERParticleID::mu),
               2.f};  // TODO: This is needed because the training was
                      // done with new HyPER dataset.
    }
    particles.push_back(feats);
  }
  Features metFeats;
  if (m_ljetsUseBTag) {
    metFeats = {float((*m_hyperInputs.m_met)["Final"]->met() / 1000),
                0.f,
                float((*m_hyperInputs.m_met)["Final"]->phi()),
                float((*m_hyperInputs.m_met)["Final"]->met() / 1000),
                0.f,
                0.f,
                float(EventReco::HyPERParticleID::met),
                3.f};
  } else {
    metFeats = {float((*m_hyperInputs.m_met)["Final"]->met() / 1000),
                0.f,
                float((*m_hyperInputs.m_met)["Final"]->phi()),
                float((*m_hyperInputs.m_met)["Final"]->met() / 1000),
                0.f,
                float(EventReco::HyPERParticleID::met),
                3.f};
  }
  particles.push_back(metFeats);

  // Add the nodes
  for (std::size_t i = 0; i < particles.size(); i++) {
    m_hyperGraph->addNode(particles[i]);
  }

  // Add the globals, scaling already applied here.
  Features globalFeats;
  if (m_ljetsUseBTag)
    globalFeats = {nJets / 6.0f,    nBJets90 / 2.0f, nBJets85 / 2.0f,
                   nBJets77 / 2.0f, nBJets70 / 2.0f, nBJets65 / 2.0f};
  else
    globalFeats = {nJets / 6.0f};
  m_hyperGraph->addGlobal(globalFeats);

  // Build graph edges and hyperedges
  m_hyperGraph->buildEdgeIndices();
  m_hyperGraph->buildHyperEdges(3);

  // Add the edges
  // Loop over the edge indices
  for (const auto& edge : m_hyperGraph->getEdgeIndicesVector()) {
    // Get the source and target nodes
    int64_t source = edge.first;
    int64_t target = edge.second;
    // Get the features
    Features firstNodeFeats = m_hyperGraph->getNodeFeats(source);
    Features secondNodeFeats = m_hyperGraph->getNodeFeats(target);

    // Calculate the edge features
    float dEta = secondNodeFeats[1] - firstNodeFeats[1];
    float dPhi = deltaPhi(secondNodeFeats[2], firstNodeFeats[2]);
    float dR = sqrt(dEta * dEta + dPhi * dPhi);
    ROOT::Math::PtEtaPhiEVector particle1;
    ROOT::Math::PtEtaPhiEVector particle2;
    particle1.SetCoordinates(firstNodeFeats[3], firstNodeFeats[1],
                             firstNodeFeats[2], firstNodeFeats[0]);
    particle2.SetCoordinates(secondNodeFeats[3], secondNodeFeats[1],
                             secondNodeFeats[2], secondNodeFeats[0]);
    float m = (particle1 + particle2).M();

    Features edgeFeats = {dEta, dPhi, dR, log(m)};
    m_hyperGraph->addEdge(source, target, edgeFeats);
  }

  // Scaling the node inputs
  for (std::size_t i{0}; i < static_cast<std::size_t>(m_hyperGraph->nNodes());
       i++) {
    Features& nodeFeats = m_hyperGraph->getNodeFeats(i);
    nodeFeats.at(0) = log(nodeFeats.at(0));
    nodeFeats.at(3) = log(nodeFeats.at(3));
  }
  return StatusCode::SUCCESS;
}

StatusCode RunHyPERAlg::buildTtbarAllHadronicGraph() {
  // For this topology we only use jets:
  // [E, eta, phi, pT, bTag, particleID = 1]
  // And Edges with the following features:
  // [dEta, dPhi, dR, m(i,j)]
  // And, the following globals:
  // [nJets, nBTagJets]

  // Build particles
  std::vector<Features> particles;
  float nBJets90 =
      0;  // Store number of bjets per-quantile for the global features.
  float nBJets85 = 0;
  float nBJets77 = 0;
  float nBJets70 = 0;
  float nBJets65 = 0;
  float nJets = 0;  // Store number of jets for the global features.
  for (const xAOD::Jet* jet : m_hyperInputs.m_jets) {
    nJets += 1.0f;
    // Get b-tagging decision
    if (!m_bTagDecoAcc->isAvailable(*jet)) {
      ANA_MSG_ERROR("HyPERAlg:: the jets do not have " << m_btagDecorName
                                                       << " aux variable!");
      return StatusCode::FAILURE;
    }

    int bTagQuantile = (*m_bTagDecoAcc)(*jet);
    if (bTagQuantile >= 2)
      nBJets90 += 1;
    if (bTagQuantile >= 3)
      nBJets85 += 1;
    if (bTagQuantile >= 4)
      nBJets77 += 1;
    if (bTagQuantile >= 5)
      nBJets70 += 1;
    if (bTagQuantile >= 6)
      nBJets65 += 1;
    Features feats = {float(jet->e() / 1000), float(jet->eta()),
                      float(jet->phi()),      float(jet->pt() / 1000),
                      float(bTagQuantile),    float(EventReco::HyPERParticleID::jet)};
    particles.push_back(feats);
  }

  // Add the nodes
  for (std::size_t i = 0; i < particles.size(); i++) {
    m_hyperGraph->addNode(particles[i]);
  }

  // Add the globals, scaling already applied here.
  Features globalFeats = {nJets / 6.0f,    nBJets90 / 2.0f, nBJets85 / 2.0f,
                          nBJets77 / 2.0f, nBJets70 / 2.0f, nBJets65 / 2.0f};
  m_hyperGraph->addGlobal(globalFeats);

  // Build graph edges and hyperedges
  m_hyperGraph->buildEdgeIndices();
  m_hyperGraph->buildHyperEdges(3);

  // Add the edges
  // Loop over the edge indices
  for (const auto& edge : m_hyperGraph->getEdgeIndicesVector()) {
    // Get the source and target nodes
    int64_t source = edge.first;
    int64_t target = edge.second;
    // Get the features
    Features firstNodeFeats = m_hyperGraph->getNodeFeats(source);
    Features secondNodeFeats = m_hyperGraph->getNodeFeats(target);

    // Calculate the edge features
    float dEta = secondNodeFeats[1] - firstNodeFeats[1];
    float dPhi = deltaPhi(secondNodeFeats[2], firstNodeFeats[2]);
    float dR = sqrt(dEta * dEta + dPhi * dPhi);
    ROOT::Math::PtEtaPhiEVector particle1;
    ROOT::Math::PtEtaPhiEVector particle2;
    particle1.SetCoordinates(firstNodeFeats[3], firstNodeFeats[1],
                             firstNodeFeats[2], firstNodeFeats[0]);
    particle2.SetCoordinates(secondNodeFeats[3], secondNodeFeats[1],
                             secondNodeFeats[2], secondNodeFeats[0]);
    float m = (particle1 + particle2).M();

    Features edgeFeats = {dEta, dPhi, dR, log(m)};
    m_hyperGraph->addEdge(source, target, edgeFeats);
  }

  // Scaling the node inputs
  for (std::size_t i{0}; i < static_cast<std::size_t>(m_hyperGraph->nNodes());
       i++) {
    Features& nodeFeats = m_hyperGraph->getNodeFeats(i);
    nodeFeats.at(0) = log(nodeFeats.at(0));
    nodeFeats.at(3) = log(nodeFeats.at(3));
  }
  return StatusCode::SUCCESS;
}

StatusCode RunHyPERAlg::buildTtbarDiLeptonGraph() {
  // For this topology we use jets, electrons and muons:
  // [Log(E), eta, phi, Log(pT), bTagQuantile/6, charge, particleID/2]
  // And Edges with the following features:
  // [dEta, dPhi, dR, m(i,j)]
  // And, the following globals:
  // [nJets/6, Log(met_pT), met_phi, nBTagJets90/2, nBTagJets85/2,
  // nBTagJets77/2, nBTagJets70/2, nBTagJets65/2]

  // Build particles
  std::vector<Features> particles;
  float nBJets90 =
      0;  // Store number of bjets per-quantile for the global features.
  float nBJets85 = 0;
  float nBJets77 = 0;
  float nBJets70 = 0;
  float nBJets65 = 0;
  float nJets = 0;  // Store number of jets for the global features.
  for (const xAOD::Jet* jet : m_hyperInputs.m_jets) {

    nJets += 1.0f;
    // Get b-tagging decision.
    if (!m_bTagDecoAcc->isAvailable(*jet)) {
      ANA_MSG_ERROR("HyPERAlg:: the jets do not have " << m_btagDecorName
                                                       << " aux variable!");
      return StatusCode::FAILURE;
    }
    int bTagQuantile = (*m_bTagDecoAcc)(*jet);
    if (bTagQuantile >= 2)
      nBJets90 += 1;
    if (bTagQuantile >= 3)
      nBJets85 += 1;
    if (bTagQuantile >= 4)
      nBJets77 += 1;
    if (bTagQuantile >= 5)
      nBJets70 += 1;
    if (bTagQuantile >= 6)
      nBJets65 += 1;
    Features feats = {float(jet->e() / 1000),
                      float(jet->eta()),
                      float(jet->phi()),
                      float(jet->pt() / 1000),
                      float(bTagQuantile),
                      0.f,
                      float(EventReco::HyPERParticleID::jet),
                      1.f};  // TODO: This is needed because the training was
                             // done with new HyPER dataset.
    particles.push_back(feats);
  }

  for (const xAOD::Electron* el : m_hyperInputs.m_electrons) {

    Features feats = {float(el->e() / 1000),
                      float(el->eta()),
                      float(el->phi()),
                      float(el->pt() / 1000),
                      0.f,
                      float(el->charge()),
                      float(EventReco::HyPERParticleID::e),
                      2.f};
    particles.push_back(feats);
  }

  for (const xAOD::Muon* mu : m_hyperInputs.m_muons) {

    Features feats = {float(mu->e() / 1000),
                      float(mu->eta()),
                      float(mu->phi()),
                      float(mu->pt() / 1000),
                      0.f,
                      float(mu->charge()),
                      float(EventReco::HyPERParticleID::mu),
                      2.f};  // TODO: This is needed because the
                             // training was done with new HyPER
                             // dataset.
    particles.push_back(feats);
  }

  // MET features
  float met_pt = (*m_hyperInputs.m_met)["Final"]->met() / 1000;
  float met_phi = (*m_hyperInputs.m_met)["Final"]->phi();

  Features metFeats = {
      met_pt, 0.f, met_phi, met_pt, 0.f, 0.f, float(EventReco::HyPERParticleID::met),
      3.f};  // TODO: This is needed because the training was
             // done with new HyPER dataset.
  particles.push_back(metFeats);

  // Add the nodes
  for (std::size_t i = 0; i < particles.size(); i++) {
    m_hyperGraph->addNode(particles[i]);
  }

  // Add the globals, scaling already applied here.
  Features globalFeats;

  globalFeats = {nJets / 6.0f,    log(met_pt),     met_phi,
                 nBJets90 / 2.0f, nBJets85 / 2.0f, nBJets77 / 2.0f,
                 nBJets70 / 2.0f, nBJets65 / 2.0f};
  m_hyperGraph->addGlobal(globalFeats);

  // Build graph edges and hyperedges
  m_hyperGraph->buildEdgeIndices();
  m_hyperGraph->buildHyperEdges(4);

  // Add the edges
  // Loop over the edge indices
  for (const auto& edge : m_hyperGraph->getEdgeIndicesVector()) {
    // Get the source and target nodes
    int64_t source = edge.first;
    int64_t target = edge.second;
    // Get the features
    Features firstNodeFeats = m_hyperGraph->getNodeFeats(source);
    Features secondNodeFeats = m_hyperGraph->getNodeFeats(target);

    // Calculate the edge features
    float dEta = secondNodeFeats[1] - firstNodeFeats[1];
    float dPhi = deltaPhi(secondNodeFeats[2], firstNodeFeats[2]);
    float dR = sqrt(dEta * dEta + dPhi * dPhi);
    ROOT::Math::PtEtaPhiEVector particle1;
    ROOT::Math::PtEtaPhiEVector particle2;
    particle1.SetCoordinates(firstNodeFeats[3], firstNodeFeats[1],
                             firstNodeFeats[2], firstNodeFeats[0]);
    particle2.SetCoordinates(secondNodeFeats[3], secondNodeFeats[1],
                             secondNodeFeats[2], secondNodeFeats[0]);
    float m = (particle1 + particle2).M();

    Features edgeFeats = {dEta, dPhi, dR, log(m)};
    m_hyperGraph->addEdge(source, target, edgeFeats);
  }

  // Scaling the node inputs
  for (std::size_t i{0}; i < static_cast<std::size_t>(m_hyperGraph->nNodes());
       i++) {
    Features& nodeFeats = m_hyperGraph->getNodeFeats(i);
    nodeFeats.at(0) = log(nodeFeats.at(0));
    nodeFeats.at(3) = log(nodeFeats.at(3));
    nodeFeats.at(4) = nodeFeats.at(4) / 6.0f;
    nodeFeats.at(6) = nodeFeats.at(6) / 2.0f;
  }
  return StatusCode::SUCCESS;
}

StatusCode RunHyPERAlg::finalize() {
  ANA_MSG_INFO("Finalizing RunHyPER");
  return StatusCode::SUCCESS;
}

}  // namespace EventReco
