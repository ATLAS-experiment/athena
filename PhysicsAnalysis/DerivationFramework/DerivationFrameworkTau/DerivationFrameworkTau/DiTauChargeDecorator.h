/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTAU_DITAUCHARGEDECORATOR_H
#define DERIVATIONFRAMEWORKTAU_DITAUCHARGEDECORATOR_H

#include <string>
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTau/DiTauJetContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

//Tool to decorate the charge of boosted ditaus

namespace DerivationFramework {

  class DiTauChargeDecorator : public extends<AthAlgTool, IAugmentationTool> {	
    public:
      DiTauChargeDecorator(const std::string& t, const std::string& n, const IInterface* p);

      virtual StatusCode initialize() override;
      virtual StatusCode addBranches(const EventContext& ctx) const override;

    private:
      SG::ReadHandleKey<xAOD::DiTauJetContainer> m_ditauContainerKey { this, "DiTauContainerName", "DiTauJets", "Input ditau container key" };
      SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_chargeKey{ this, "chargeKey", m_ditauContainerKey, "charge", "Decoration name"};
  };
}

#endif // DERIVATIONFRAMEWORKTAU_DITAUCHARGEDECORATOR_H

