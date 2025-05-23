/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "DecisionHandling/HypoBase.h"
#include "xAODTau/DiTauJetContainer.h"

#include "ITrigDiTauHypoTool.h"


/**
 * @class TrigDiTauHypoAlg
 * @brief HLT Precision step TauJet selection hypothesis algorithm
 **/
class TrigDiTauHypoAlg : public ::HypoBase
{
public: 
    TrigDiTauHypoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private: 
    ToolHandleArray<ITrigDiTauHypoTool> m_hypoTools {this, "HypoTools", {}, "Hypo tools"};
    SG::ReadHandleKey<xAOD::DiTauJetContainer> m_DiTauJets_key {this, "DiTauJets_key", "", "DiTauJets in view" };
};
