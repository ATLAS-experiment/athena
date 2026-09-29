/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Abicht

#include <EgammaAnalysisAlgorithms/PhotonExtraVariablesAlg.h>

namespace CP {

  StatusCode PhotonExtraVariablesAlg::initialize() {

    ANA_CHECK(m_photonsHandle.initialize(m_systematicsList));

    ANA_CHECK(m_conversionTypeHandle.initialize(m_systematicsList, m_photonsHandle));
    ANA_CHECK(m_caloEta2Handle.initialize(m_systematicsList, m_photonsHandle));

    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode PhotonExtraVariablesAlg::execute(const EventContext& ctx) {

    for (const auto &sys : m_systematicsList.systematicsVector()) {

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK(m_photonsHandle.retrieve(photons, sys, ctx));

      for (const xAOD::Photon *photon : *photons) {
        int conversionType = photon->conversionType();
        m_conversionTypeHandle.set(*photon, conversionType, sys);

        if (!photon->caloCluster()) {
          ANA_MSG_ERROR("photon without a calo cluster");
          return StatusCode::FAILURE;
        }
        float caloEta2 = photon->caloCluster()->etaBE(2);
        m_caloEta2Handle.set(*photon, caloEta2, sys);
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace
