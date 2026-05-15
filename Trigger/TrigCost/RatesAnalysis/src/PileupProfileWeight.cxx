/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PileupProfileWeight.h"

#include "xAODEventInfo/EventInfo.h"

#include <fstream>

StatusCode PileupProfileWeight::initialize() {
  std::ifstream fs(m_weightsFile);
  try {
    m_weightsMap = nlohmann::json::parse(fs);
  } catch (const std::exception&) {
    ATH_MSG_ERROR("Failed to parse input " << m_weightsFile);
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode PileupProfileWeight::getValue(double& value) const {
  const xAOD::EventInfo* evt {nullptr};
  ATH_CHECK(evtStore()->retrieve(evt, "EventInfo"));

  auto mu_actual = static_cast<std::size_t>(std::ceil(evt->actualInteractionsPerCrossing()));
  auto mu = std::to_string(mu_actual);

  // mu values that are not included in the weight mapping will be skipped
  if (m_weightsMap.find(mu) == m_weightsMap.end()) {
    value = 0.0;
    return StatusCode::SUCCESS;
  }

  // retrieve the weigths
  try {
    auto wt = m_weightsMap.at(mu).get<double>();
    value = wt;
  } catch (const std::exception&) {
    ATH_MSG_ERROR("Failed to get the mu weight!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
