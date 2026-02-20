/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaContainer.h"
#include <EgammaAnalysisAlgorithms/EgammaCaloClusterEtaAlg.h>

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>


namespace CP {

  StatusCode EgammaCaloClusterEtaAlg::initialize() {

    if (m_caloEta2Key.contHandleKey().key() == m_caloEta2Key.key()) {
      m_caloEta2Key = m_particlesKey.key() + "." + m_caloEta2Key.key();
    }

    ANA_CHECK(m_particlesKey.initialize());
    ANA_CHECK(m_caloEta2Key.initialize());

    // create an accessor to register the type for output algorithms
    SG::ConstAccessor<float> (m_caloEta2Key.key().substr (m_caloEta2Key.key().find_last_of(".") + 1));

    return StatusCode::SUCCESS;
  }

  StatusCode EgammaCaloClusterEtaAlg::execute(const EventContext &ctx) const {

    SG::ReadHandle<xAOD::EgammaContainer> particles(m_particlesKey, ctx);

    SG::WriteDecorHandle<xAOD::EgammaContainer, float> caloEta2Handle(m_caloEta2Key, ctx);
    for (const xAOD::Egamma *particle : *particles) {
      caloEta2Handle(*particle) = particle->caloCluster()->etaBE(2);
    }

    return StatusCode::SUCCESS;
  }

} // namespace
