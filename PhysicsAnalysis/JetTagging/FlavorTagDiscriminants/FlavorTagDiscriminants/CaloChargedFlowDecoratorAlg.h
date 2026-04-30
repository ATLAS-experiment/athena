/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALO_CHARGED_FLOW_DECORATOR_ALG_H
#define CALO_CHARGED_FLOW_DECORATOR_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODBase/IParticleContainer.h"


namespace FlavorTagDiscriminants {

  class CaloChargedFlowDecoratorAlg : public AthReentrantAlgorithm {
  public:
    CaloChargedFlowDecoratorAlg(const std::string& name,
                                ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;

  private:

    using IPC = xAOD::IParticleContainer;

    SG::ReadHandleKey<IPC> m_caloClusterCollection {
      this, "CaloClusterCollection", "CaloCalTopoClusters",
        "Input calo cluster container"};
    SG::ReadHandleKey<IPC> m_neutralPFOCollection {
      this, "NeutralPFOCollection", "CHSGNeutralParticleFlowObjects",
        "Input neutral PFO container"};
    SG::ReadHandleKey<IPC> m_chargedPFOCollection {
      this, "ChargedPFOCollection", "CHSGChargedParticleFlowObjects",
        "Input charged PFO container"};

    SG::WriteDecorHandleKey<IPC> m_objUsedInChargedDecorator {
      this, "ChargedFlowDecorator", m_caloClusterCollection,
        "usedInChargedFlow",
        "Decorator for charged flow flag on calo clusters"};
  };

}

#endif
