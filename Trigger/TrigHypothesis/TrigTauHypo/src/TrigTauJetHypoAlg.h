// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUHYPO_TrigTauJetHypoAlg_H
#define TRIGTAUHYPO_TrigTauJetHypoAlg_H

#include "DecisionHandling/HypoBase.h"
#include "xAODTau/TauJetContainer.h"

#include "ITrigTauJetHypoTool.h"


/**
 * @class TrigTauJetHypoAlg
 * @brief HLT CaloMVA/CaloHits/Precision step TauJet selection
 **/
class TrigTauJetHypoAlg : public ::HypoBase
{
public: 
    TrigTauJetHypoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private: 
    ToolHandleArray<ITrigTauJetHypoTool> m_hypoTools {this, "HypoTools", {}, "Hypo tools"};
     
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tauJetKey {this, "TauJetsKey", "", "TauJets in view"};
}; 

#endif
