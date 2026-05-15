/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JZCombineWeightNtupler.h"

#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

JZCombineWeightNtupler::JZCombineWeightNtupler(const std::string& name, ISvcLocator* pSvcLocator)
    : RatesAnalysisAlg(name, pSvcLocator) {}

JZCombineWeightNtupler::~JZCombineWeightNtupler() {}

StatusCode JZCombineWeightNtupler::ratesInitialize() {
  // Check and retrieve emulated triggers
  ATH_CHECK(m_triggers.retrieve());
  
  // Ntuple output
  ATH_CHECK(resetValues());
  m_tree = new TTree("analysis", "Ntuple for JZ combine weight calculation");
  m_tree->Branch("event_number", &m_event_number);
  m_tree->Branch("weight_eb", &m_weight_eb);
  m_tree->Branch("mu_actual", &m_mu_actual);
  m_tree->Branch("pt_j0_AK4HS", &m_pt_j0_AK4HS);
  m_tree->Branch("index_JZ", &m_index_JZ);
  m_tree->Branch("n_pileup_records", &m_n_pileup_records);
  m_tree->Branch("pt_j0_AK4PU", &m_pt_j0_AK4PU);
  m_tree->Branch("pileup_event_number", &m_pileup_event_number);
  ATH_CHECK(addEmulatedThresholds());
  ATH_CHECK(histSvc()->regTree("/RATESTREAM/analysis", m_tree));

  // Set JZ index once
  const xAOD::EventInfo* evt {nullptr};
  ATH_CHECK(evtStore()->retrieve(evt, "EventInfo"));
  m_index_JZ = evt->mcChannelNumber() - m_dsid_JZ0;

  return StatusCode::SUCCESS;
}

StatusCode JZCombineWeightNtupler::ratesExecute() {
  ATH_CHECK(resetValues());
  
  // Event level variables
  const xAOD::EventInfo* evt {nullptr};
  ATH_CHECK(evtStore()->retrieve(evt, "EventInfo"));

  m_event_number = evt->eventNumber();
  m_weight_eb = m_weightingValues.m_enhancedBiasWeight; // this is exactly MC weight for MC
  m_mu_actual = std::ceil(evt->actualInteractionsPerCrossing()); // half integer to integer
  
  // Truth jets are only valid for MC
  bool isMC = evt->eventTypeBitmask() & xAOD::EventInfo::IS_SIMULATION;
  if (isMC) {
    // HS jets
    const xAOD::JetContainer* hs_jets {nullptr};
    ATH_CHECK(evtStore()->retrieve(hs_jets, m_jetCollectionHS));
    if (hs_jets->size()) {
      auto iter_jet = std::max_element(hs_jets->begin(), hs_jets->end(), 
            [](const xAOD::Jet* j0, const xAOD::Jet* j1){ return j0->pt() < j1->pt(); });
      m_pt_j0_AK4HS = (*iter_jet)->pt() / GeV;
    }

    // PU jets
    const xAOD::JetContainer* pu_jets {nullptr};
    ATH_CHECK(evtStore()->retrieve(pu_jets, m_jetCollectionPU));
    std::unordered_map<int, double> leading_pts;
    if (pu_jets->size()) {
      for (const xAOD::Jet* pu_jet : *pu_jets) {
        SG::ConstAccessor<int> pu_enum_acc("pileupEventNumber");
        const uint32_t pu_enum = static_cast<uint32_t>(pu_enum_acc.withDefault(*pu_jet, 0));
        double pu_jet_pt = pu_jet->pt() / GeV;
        if (!leading_pts.contains(pu_enum) || leading_pts.at(pu_enum) < pu_jet_pt) {
          leading_pts[pu_enum] = pu_jet_pt;
        } 
      }
    }
    m_n_pileup_records = leading_pts.size();

    for (const auto& [pu_enum, pu_jet_pt] : leading_pts) {
      m_pt_j0_AK4PU.push_back(pu_jet_pt);
      m_pileup_event_number.push_back(pu_enum);
    }
  }

  // Add extra branches for emulated triggers if required
  ATH_CHECK(setEmulatedThresholds());

  m_tree->Fill();
  return StatusCode::SUCCESS;
}

StatusCode JZCombineWeightNtupler::ratesFinalize() {
  return StatusCode::SUCCESS;
}

StatusCode JZCombineWeightNtupler::addEmulatedThresholds() {
  for (auto& trigger : m_triggers) {
    const std::string& name = trigger->branchName();
    if (m_thresholds.contains(name)) {
      ATH_MSG_WARNING(name << " already exists! Will skip");
      continue;
    }
    ATH_MSG_INFO("Add new branch - " << name);
    m_tree->Branch(name.c_str(), &m_thresholds[name]);
  }
  return StatusCode::SUCCESS;
}

StatusCode JZCombineWeightNtupler::setEmulatedThresholds() {
  for (auto& trigger : m_triggers) {
    const std::string& name = trigger->branchName();
    ATH_CHECK(trigger->updateThresholdValue());
    if (!m_thresholds.contains(name)) {
      ATH_MSG_ERROR(name << " does not exist! Please check `Triggers`!");
      return StatusCode::FAILURE;
    }
    m_thresholds.at(name) = trigger->thresholdValue();
  }
  return StatusCode::SUCCESS;
}

StatusCode JZCombineWeightNtupler::resetValues() {
  m_event_number = 0;
  m_weight_eb = 1.0;
  m_mu_actual = 0.0;
  m_pt_j0_AK4HS = 5.0; // GeV, JETMiss example uses this value
  m_n_pileup_records = 0;
  m_pt_j0_AK4PU.clear();
  m_pileup_event_number.clear();
  return StatusCode::SUCCESS;
}
