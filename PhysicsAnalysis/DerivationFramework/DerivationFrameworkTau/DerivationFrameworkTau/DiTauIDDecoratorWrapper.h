/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H
#define DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "DiTauRec/DiTauOnnxDiscriminantTool.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODTau/DiTauJetContainer.h"

#include <string>
#include <vector>

/**
 * wrapper tool for decorating tau ID scores and WPs
*/

namespace DerivationFramework {

  class DiTauIDDecoratorWrapper : public extends<AthAlgTool, IAugmentationTool> {
    public:
      using base_class::base_class;	    

      virtual StatusCode initialize() override;
      virtual StatusCode addBranches(const EventContext& ctx) const override;

    private:
      SG::ReadHandleKey<xAOD::DiTauJetContainer> m_ditauContainerKey { this, "DiTauContainerName", "DiTauJets", "Input tau container key" };
      SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_scoreDecorKey{this, "ScoreDecorationKey", "" };

      ToolHandle<DiTauOnnxDiscriminantTool> m_tDiTauOnnxDiscriminantTool{this, "DiTauOnnxDiscriminantTool", ""};

  };
}

#endif // DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H
