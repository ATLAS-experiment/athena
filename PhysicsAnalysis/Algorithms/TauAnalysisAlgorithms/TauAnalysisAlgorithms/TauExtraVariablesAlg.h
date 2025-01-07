/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef TAU_EXTRA_VARIABLES_ALG_H
#define TAU_EXTRA_VARIABLES_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <xAODTau/TauJetContainer.h>

namespace CP {

  class TauExtraVariablesAlg final : public EL::AnaAlgorithm {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausKey { this, "taus", "", "the input tau jet container" };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_nTracksKey { this, "nTracks", "nTracks", "decoration name for tau number of tracks" };
  };

} // namespace

#endif
