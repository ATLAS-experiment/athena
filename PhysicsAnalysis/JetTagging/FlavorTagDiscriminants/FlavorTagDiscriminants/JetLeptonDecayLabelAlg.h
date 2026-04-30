/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_LEPTON_DECAY_LABEL_ALG_H
#define JET_LEPTON_DECAY_LABEL_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODJet/JetContainer.h"


namespace FlavorTagDiscriminants {

  class JetLeptonDecayLabelAlg : public AthReentrantAlgorithm {
  public:
    JetLeptonDecayLabelAlg(const std::string& name,
                           ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;

  private:

    // Input container
    SG::ReadHandleKey<xAOD::JetContainer> m_jetContainerKey {
      this, "jetContainer", "AntiKt4EMPFlowJets",
        "Key for the input jet collection"};

    // Output decorations
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_leptonDecayLabel {
      this, "LeptonDecayLabel", m_jetContainerKey, "LeptonDecayLabel",
        "Lepton decay label encoding b/c hadron semileptonic decays"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_tauDecayLabel {
      this, "TauDecayLabel", m_jetContainerKey, "TauDecayLabel",
        "Tau decay label encoding leptonic tau sub-decays"};
  };

}

#endif
