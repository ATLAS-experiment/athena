/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Antonio De Maria

#ifndef DITAU_EXTRA_VARIABLES_ALG_H
#define DITAU_EXTRA_VARIABLES_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <xAODTau/DiTauJetContainer.h>

namespace CP {

  class DiTauExtraVariablesAlg final : public EL::AnaReentrantAlgorithm {

  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::DiTauJetContainer> m_ditausKey { this, "ditaus", "", "the input ditau jet container" };
    
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_leadSubjetPtKey{this, "leadSubjetPt", "leadSubjetPt", "decoration name for leading subjet pt"};
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_leadSubjetEtaKey{this, "leadSubjetEta", "leadSubjetEta", "decoration name for leading subjet eta"};
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_leadSubjetPhiKey{this, "leadSubjetPhi", "leadSubjetPhi", "decoration name for leading subjet phi"};
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_leadSubjetEKey{this, "leadSubjetE", "leadSubjetE", "decoration name for leading subjet energy"};
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_leadSubjetNTracksKey{this, "leadSubjetNTracks", "leadSubjetNTracks", "decoration name for leading subjet number of tracks"};

    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_subleadSubjetPtKey{this, "subleadSubjetPt", "subleadSubjetPt", "decoration name for subleading subjet pt"};
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_subleadSubjetEtaKey{this, "subleadSubjetEta", "subleadSubjetEta", "decoration name for subleading subjet eta"};
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_subleadSubjetPhiKey{this, "subleadSubjetPhi", "subleadSubjetPhi", "decoration name for subleading subjet phi"};
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_subleadSubjetEKey{this, "subleadSubjetE", "subleadSubjetE", "decoration name for subleading subjet energy"};
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_subleadSubjetNTracksKey{this, "subleadSubjetNTracks", "subleadSubjetNTracks", "decoration name for subleading subjet number of tracks"};

  };

} // namespace

#endif
