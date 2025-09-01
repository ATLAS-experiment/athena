/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef TAU_EXTRA_VARIABLES_ALG_H
#define TAU_EXTRA_VARIABLES_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <xAODTau/TauJetContainer.h>

namespace CP {

  class TauExtraVariablesAlg final : public EL::AnaReentrantAlgorithm {

  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausKey { this, "taus", "", "the input tau jet container" };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_nTracksChargedKey { this, "nTracksCharged", "nTracksCharged", "decoration name for tau number of tracks" };
  };

} // namespace

#endif
