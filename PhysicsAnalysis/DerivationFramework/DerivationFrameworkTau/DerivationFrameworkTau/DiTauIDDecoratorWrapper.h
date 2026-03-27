/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H
#define DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "DiTauRec/DiTauOnnxDiscriminantTool.h"
#include "DiTauRec/DiTauWPDecorator.h"
#include "xAODTau/DiTauJetContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"

#include <vector>

/**
 * wrapper tool for decorating tau ID scores and WPs
 */

namespace DerivationFramework {

  class DiTauIDDecoratorWrapper : public AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::DiTauJetContainer> m_ditauContainerKey { this, "DiTauContainerName", "DiTauJets", "Input tau container key" };
    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_scoreDecorKey{this, "ScoreDecorationKey", m_ditauContainerKey, "omni_score" };
    SG::WriteDecorHandleKeyArray<xAOD::DiTauJetContainer> m_WPDecorKeys{ this, "WPDecorationKeys", m_ditauContainerKey, {}, "List of WP decorations added to the ditau"};

    ToolHandle<DiTauOnnxDiscriminantTool> m_tDiTauOnnxDiscriminantTool{this, "DiTauOnnxDiscriminantTool", ""};
    ToolHandle<DiTauWPDecorator> m_tDiTauWPDecoratorTool{this, "DiTauWPDecorator", ""};

    Gaudi::Property<bool> m_doWPDecor{this, "DoWPDecor", false, "Enable WP decoration"};

    Gaudi::Property<std::vector<float>> m_WPCuts{this, "DecorWPCuts", {}};

    ServiceHandle<IChronoStatSvc>      m_chronoSvc{this, "ChronoStatSvc",  "ChronoStatSvc"};
  };
}

#endif // DERIVATIONFRAMEWORKTAU_DITAUIDDECORATORWRAPPER_H
