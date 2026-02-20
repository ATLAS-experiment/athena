/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTAU_TAUIDDECORATORWRAPPER_H
#define DERIVATIONFRAMEWORKTAU_TAUIDDECORATORWRAPPER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "AsgTools/ToolHandleArray.h"
#include "tauRecTools/TauRecToolBase.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODMuon/MuonContainer.h"

#include <string>
#include <vector>

/**
 * wrapper tool for decorating tau ID scores and WPs
 */

namespace DerivationFramework {

  class TauIDDecoratorWrapper : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tauContainerKey { this, "TauContainerName", "TauJets", "Input tau container key" };
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonContainerKey { this, "MuonContainerName", "Muons", "Input muon container key" };
    SG::WriteDecorHandleKeyArray<xAOD::TauJetContainer> m_scoreDecorKeys{ this, "ScoreDecorationKeys", m_tauContainerKey, {}, "List of score decorations added to the tau"};
    SG::WriteDecorHandleKeyArray<xAOD::TauJetContainer> m_WPDecorKeys{ this, "WPDecorationKeys", m_tauContainerKey, {}, "List of WP decorations added to the tau"};
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_trackWidthKey{ this, "TrackWidthKey", m_tauContainerKey, "trackWidth", "Track width decoration name"};
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_passTATTauMuonOLRKey{ this, "PassTATTauMuonOLRKey", m_tauContainerKey, "passTATTauMuonOLR", "Decoration name"};

    ToolHandleArray<TauRecToolBase> m_tauIDTools { this, "TauIDTools", {}, "" };
    Gaudi::Property<bool> m_doEvetoWP{this, "DoEvetoWP", false};
  };
}

#endif // DERIVATIONFRAMEWORKTAU_TAUIDDECORATORWRAPPER_H
