/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonJetDrAlg.h
///////////////////////////////////////////////////////////////////
#ifndef DERIVATIONFRAMEWORK_MuonJetDrAlg_H
#define DERIVATIONFRAMEWORK_MuonJetDrAlg_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"

namespace DerivationFramework {
  class MuonJetDrAlg : public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonSGKey{this, "ContainerKey", "Muons"};
    SG::ReadHandleKey<xAOD::JetContainer> m_jetSGKey{this, "JetContainerKey", "AntiKt4EMTopoJets"};
    SG::WriteDecorHandleKey<xAOD::MuonContainer> m_jetDR_SGKey{this, "dRDecoration", m_muonSGKey, "DFCommonJetDr"};

    Gaudi::Property<float> m_jetMinPt{this, "JetMinPt", 20.e3, "Minimal pt cut of the jets to be considered"};
  };
}  // namespace DerivationFramework
#endif  //
