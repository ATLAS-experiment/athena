/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTAU_DITAUCHARGEDECORATOR_H
#define DERIVATIONFRAMEWORKTAU_DITAUCHARGEDECORATOR_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTau/DiTauJetContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"

//Tool to decorate the charge of boosted ditaus

namespace DerivationFramework {

  class DiTauChargeDecorator : public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::DiTauJetContainer> m_ditauContainerKey { this, "DiTauContainerName", "DiTauJets", "Input ditau container key" };
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_chargeKey{ this, "chargeKey", m_ditauContainerKey, "charge", "Decoration name"};

    ServiceHandle<IChronoStatSvc>      m_chronoSvc{this, "ChronoStatSvc",  "ChronoStatSvc"};
  };
}

#endif // DERIVATIONFRAMEWORKTAU_DITAUCHARGEDECORATOR_H
