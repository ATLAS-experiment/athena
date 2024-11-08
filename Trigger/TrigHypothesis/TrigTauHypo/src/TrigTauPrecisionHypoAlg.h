/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauPrecisionHypoAlg_H
#define TrigTauHypo_TrigTauPrecisionHypoAlg_H

#include "DecisionHandling/HypoBase.h"
#include "xAODTau/TauJetContainer.h"

#include "ITrigTauPrecisionHypoTool.h"


/**
 * @class TrigTauPrecisionHypoAlg
 * @brief HLT Precision step TauJet selection hypothesis algorithm
 **/
class TrigTauPrecisionHypoAlg : public ::HypoBase
{
public: 
    TrigTauPrecisionHypoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private: 
    ToolHandleArray<ITrigTauPrecisionHypoTool> m_hypoTools {this, "HypoTools", {}, "Hypo tools"};
       
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tauJetKey {this, "TauJetsKey", "", "TauJets in view" };
};

#endif
