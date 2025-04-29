/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oliver Majersky
/// @author Baptiste Ravina

#include "KLFitterAnalysisAlgorithms/KLFitterFinalizeOutputAlg.h"

namespace EventReco {

KLFitterFinalizeOutputAlg::KLFitterFinalizeOutputAlg(const std::string &name,
                                                     ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator) {}

StatusCode KLFitterFinalizeOutputAlg::initialize() {
  ANA_CHECK(m_inHandle.initialize(m_systematicsList));
  ANA_CHECK(m_outHandle.initialize(m_systematicsList));
  ANA_CHECK(m_systematicsList.initialize());
  return StatusCode::SUCCESS;
}

StatusCode KLFitterFinalizeOutputAlg::execute() {
  for (const auto &sys : m_systematicsList.systematicsVector()) {
    // for events, where the KLFitterResultContainer was not created (e.g. event
    // didn't pass any of the regions' selections) we have to record an empty
    // container -- this ensures that every single event has at least empty
    // container (no KLFitter permutations)
    if (!m_inHandle.isValid(sys)) {
      auto resultAuxContainer =
          std::make_unique<xAOD::KLFitterResultAuxContainer>();
      auto resultContainer = std::make_unique<xAOD::KLFitterResultContainer>();
      resultContainer->setStore(resultAuxContainer.get());
      ANA_CHECK(m_outHandle.record(std::move(resultContainer),
                                   std::move(resultAuxContainer), sys));
    }
  }
  return StatusCode::SUCCESS;
}

}  // namespace EventReco
