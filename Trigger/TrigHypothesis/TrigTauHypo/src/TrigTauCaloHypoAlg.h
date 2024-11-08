// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUHYPO_TrigTauCaloHypoAlg_H
#define TRIGTAUHYPO_TrigTauCaloHypoAlg_H

#include "DecisionHandling/HypoBase.h"
#include "xAODTau/TauJetContainer.h"

#include "ITrigTauCaloHypoTool.h"


/**
 * @class TrigTauCaloHypoAlg
 * @brief HLT CaloMVA step TauJet selection
 **/
class TrigTauCaloHypoAlg : public ::HypoBase
{
public: 
    TrigTauCaloHypoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private: 
    ToolHandleArray<ITrigTauCaloHypoTool> m_hypoTools {this, "HypoTools", {}, "Hypo tools"};
     
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tauJetKey {this, "TauJetsKey", "", "TauJets in view"};
}; 

#endif
