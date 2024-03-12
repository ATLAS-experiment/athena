/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

#include <string>
#include <vector>

/**
 * wrapper tool for decorating tau ID scores and WPs
*/

namespace DerivationFramework {

  class TauIDDecoratorWrapper : public AthAlgTool, public IAugmentationTool {
    public:
      TauIDDecoratorWrapper(const std::string& t, const std::string& n, const IInterface* p);

      StatusCode initialize() override;
      StatusCode finalize() override;
      virtual StatusCode addBranches() const override;

    private:
      SG::ReadHandleKey<xAOD::TauJetContainer> m_tauContainerKey { this, "TauContainerName", "TauJets", "Input tau container key" };
      SG::WriteDecorHandleKeyArray<xAOD::TauJetContainer> m_decorKeys{ this, "DecorationKeys", {}, "List of decorations added to the tau"};

      ToolHandleArray<TauRecToolBase> m_tauIDTools { this, "TauIDTools", {}, "" };
      bool m_doEvetoWP = false;
      std::vector<std::string> m_scores;
      std::vector<std::string> m_WPs;
  };
}

#endif // DERIVATIONFRAMEWORKTAU_TAUIDDECORATORWRAPPER_H
