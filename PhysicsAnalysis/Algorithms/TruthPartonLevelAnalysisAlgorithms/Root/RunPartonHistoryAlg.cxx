/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthPartonLevelAnalysisAlgorithms/RunPartonHistoryAlg.h"

#include "PartonHistory/PartonSchemeConfig.h"

namespace CP {
RunPartonHistoryAlg::RunPartonHistoryAlg(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator) {
}

StatusCode RunPartonHistoryAlg::initialize() {
  ANA_MSG_INFO("Initializing PartonHistory " << name());
  ANA_MSG_INFO("  - partonScheme: " << m_PartonScheme);

  // Look up scheme configuration from the registry
  try {
    const PartonSchemeConfig& cfg = getSchemeConfig(m_PartonScheme);
    m_PartonHistory = std::make_unique<CalcPartonHistory>(
        name() + "_CalcPartonHistory", cfg.truthCollections);
    m_PartonHistory->configure(cfg);
  } catch (const std::runtime_error& e) {
    ANA_MSG_ERROR("  ==> PartonScheme " << m_PartonScheme
                                        << " is not recognised! aborting.");
    return StatusCode::FAILURE;
  }
  ANA_CHECK(m_PartonHistory->setProperty("prefix", m_PartonScheme));
  ANA_CHECK(m_PartonHistory->setProperty("symbolFCNC", m_SymbolFCNC));
  ANA_CHECK(m_PartonHistory->initialize());
  return StatusCode::SUCCESS;
}

StatusCode RunPartonHistoryAlg::execute(const EventContext& /*ctx*/) {
  ANA_CHECK(m_PartonHistory->execute());
  return StatusCode::SUCCESS;
}

}  // namespace CP
