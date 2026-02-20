/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonJetDrTool.h
///////////////////////////////////////////////////////////////////
#ifndef DERIVATIONFRAMEWORK_MuonJetDrTool_H
#define DERIVATIONFRAMEWORK_MuonJetDrTool_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"

namespace DerivationFramework {
  class MuonJetDrTool : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonSGKey{this, "ContainerKey", "Muons"};
    SG::ReadHandleKey<xAOD::JetContainer> m_jetSGKey{this, "JetContainerKey", "AntiKt4EMTopoJets"};
    SG::WriteDecorHandleKey<xAOD::MuonContainer> m_jetDR_SGKey{this, "dRDecoration", m_muonSGKey, "DFCommonJetDr"};

    Gaudi::Property<float> m_jetMinPt{this, "JetMinPt", 20.e3, "Minimal pt cut of the jets to be considered"};
  };
}  // namespace DerivationFramework
#endif  //
