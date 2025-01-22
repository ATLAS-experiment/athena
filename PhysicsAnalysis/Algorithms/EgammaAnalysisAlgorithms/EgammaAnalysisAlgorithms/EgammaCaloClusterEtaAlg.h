/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef EGAMMA_ANALYSIS_ALGORITHMS__EGAMMA_CALO_CLUSTER_ETA_ALG__H
#define EGAMMA_ANALYSIS_ALGORITHMS__EGAMMA_CALO_CLUSTER_ETA_ALG__H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <xAODEgamma/EgammaContainer.h>

namespace CP {

  class EgammaCaloClusterEtaAlg final : public EL::AnaReentrantAlgorithm {

  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::EgammaContainer> m_particlesKey { this, "particles", "", "the input egamma container" };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_caloEta2Key { this, "caloEta2", "caloEta2", "decoration name for calo cluster eta in layer 2" };
  
  };

} // namespace

#endif
