/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTAU_TAUMASSDECORATOR_H
#define DERIVATIONFRAMEWORKTAU_TAUMASSDECORATOR_H

#include <string>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTau/TauJetContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"


//Tool to decorate the charge of boosted ditaus

namespace DerivationFramework {

  class TauMassDecorator : public AthReentrantAlgorithm{
    public:
      TauMassDecorator(const std::string& name, ISvcLocator* pSvcLocator);	     

      virtual StatusCode initialize() override final;
      StatusCode execute(const EventContext& ctx) const override final;

    private:
      /** @brief Name of the tau output collection*/ 
      SG::WriteHandleKey<xAOD::TauJetContainer> m_tauOutputKey {this, "TauOutputName", "TauJets", "Name of TauJet Container to be created"};
      
      /** @brief Name of the tau input collection */
      SG::ReadHandleKey<xAOD::TauJetContainer> m_tauInputKey {this, "TauInputName", "old_TauJets", "Name of TauJet container to be read in"};

      //SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_massKey{ this, "massKey", m_tauOutputKey, "m", "Decoration name"};

  };
}

#endif // DERIVATIONFRAMEWORKTAU_TAUMASSDECORATOR_H

