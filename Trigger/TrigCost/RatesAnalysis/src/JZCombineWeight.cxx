/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JZCombineWeight.h"

#include "AthenaKernel/Units.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"

#include <fstream>
#include <sstream>

using Athena::Units::GeV;

/**
 * This class provides the weight used to combine JZ sliced samples by querying JSON.
 */

StatusCode JZCombineWeight::initialize() {
  std::ifstream fs(m_weightsFile);
  try {
    m_weightsMap = nlohmann::json::parse(fs);
  } catch (const std::exception&) {
    ATH_MSG_ERROR("Failed to parse input " << m_weightsFile);
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode JZCombineWeight::getValue(double& value) const {
  const xAOD::EventInfo* evt {nullptr};
  ATH_CHECK(evtStore()->retrieve(evt, "EventInfo"));

  // Truth jets are only valid for MC
  bool isMC = evt->eventTypeBitmask() & xAOD::EventInfo::IS_SIMULATION;
  if (!isMC) { return StatusCode::SUCCESS; }

  auto mu_actual = static_cast<std::size_t>(std::ceil(evt->actualInteractionsPerCrossing()));


  // categories
  std::vector<std::size_t> categories(m_binning.size() - 1, 0);

  // HS jets
  double pt_j0_AK4HS {5.0};
  std::size_t idx_HS {0};
  const xAOD::JetContainer* hs_jets {nullptr};
  ATH_CHECK(evtStore()->retrieve(hs_jets, m_jetCollectionHS));
  if (hs_jets->size()) {
    auto iter_jet = std::max_element(hs_jets->begin(), hs_jets->end(), 
          [](const xAOD::Jet* j0, const xAOD::Jet* j1){ return j0->pt() < j1->pt(); });
    pt_j0_AK4HS = (*iter_jet)->pt() / GeV;
    idx_HS = getIndex(pt_j0_AK4HS);
  }

  ++categories[idx_HS];

  // PU jets
  std::vector<double> pt_j0_AK4PU;
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
  auto n_pileup_records = leading_pts.size();

  for (const auto& pair : leading_pts) {
    double pt = pair.second;
    pt_j0_AK4PU.push_back(pt);
    std::size_t idx_PU = getIndex(pt);
    ++categories[idx_PU];
  }

  // Remining low pT pileups
  categories[0] += mu_actual - n_pileup_records - 1;

  // Convert to string
  std::stringstream key_ss;
  std::string delim {"_"};
  for (std::size_t i = 0; i < categories.size(); ++i) {
    if (i != 0) key_ss << delim;
    key_ss << categories.at(i);
  }
  key_ss << "," << idx_HS;

  const std::string& key = key_ss.str();

  // categories that are not included in the weight mapping will be skipped
  if (m_weightsMap.find(key) == m_weightsMap.end()) {
    value = 0.0;
    return StatusCode::SUCCESS;
  }

  for (const auto& weightName : m_weightsName) {
    try {
      auto wt = m_weightsMap.at(key).at(weightName).get<double>();
      value *= wt;
    } catch (const std::exception&) {
      ATH_MSG_ERROR("Failed to get weight [" << weightName << "] in [" << key << "]!");
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

std::size_t JZCombineWeight::getIndex(double value) const {
    auto it = std::upper_bound(m_binning.begin(), m_binning.end(), value);
    return std::distance(m_binning.begin(), it) - 1;
}

