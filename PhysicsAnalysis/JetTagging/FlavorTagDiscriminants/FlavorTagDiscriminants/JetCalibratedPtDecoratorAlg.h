/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVOR_TAG_DISCRIMINANTS_JET_CALIBRATED_PT_DECORATOR_ALG_H
#define FLAVOR_TAG_DISCRIMINANTS_JET_CALIBRATED_PT_DECORATOR_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "JetInterface/IJetModifier.h"


namespace FlavorTagDiscriminants {

  class JetCalibratedPtDecoratorAlg : public AthReentrantAlgorithm {
  public:
    JetCalibratedPtDecoratorAlg(const std::string& name,
                                ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    ToolHandle<IJetModifier> m_calibTool;

    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey {
      this, "JetContainer", "AntiKt4EMPFlowJets",
        "Input jet container"};

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_ptCalibKey {
      this, "ptCalibratedKey", "AntiKt4EMPFlowJets.pt_calibrated",
        "Decorated calibrated pT"};
  };

} // namespace FlavorTagDiscriminants

#endif
