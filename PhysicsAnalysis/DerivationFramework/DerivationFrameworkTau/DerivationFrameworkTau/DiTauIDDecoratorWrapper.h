/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H
#define DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "DiTauRec/DiTauOnnxDiscriminantTool.h"
#include "DiTauRec/DiTauWPDecorator.h"
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
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_scoreDecorKey{this, "ScoreDecorationKey", m_ditauContainerKey, "omni_score" };
    SG::WriteDecorHandleKeyArray<xAOD::DiTauJetContainer> m_WPDecorKeys{ this, "WPDecorationKeys", m_ditauContainerKey, {}, "List of WP decorations added to the ditau"};

    ToolHandle<DiTauOnnxDiscriminantTool> m_tDiTauOnnxDiscriminantTool{this, "DiTauOnnxDiscriminantTool", ""};
    ToolHandle<DiTauWPDecorator> m_tDiTauWPDecoratorTool{this, "DiTauWPDecorator", ""};

    Gaudi::Property<bool> m_doWPDecor{this, "DoWPDecor", false, "Enable WP decoration"};

    Gaudi::Property<std::vector<float>> m_WPCuts{this, "DecorWPCuts", {}};

  };
}

#endif // DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H
