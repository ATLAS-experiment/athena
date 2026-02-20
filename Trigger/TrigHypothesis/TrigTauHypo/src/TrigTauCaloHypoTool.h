// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauCaloHypoTool_H
#define TrigTauHypo_TrigTauCaloHypoTool_H

/********************************************************************
 *
 * NAME:     TrigTauCaloHypoTool.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigTauHypo
 *
 * AUTHORS:   P.O. DeViveiros, J.Y. Beaucamp
 * CREATED:   Sometime in early 2015
 *
 * DESCRIPTION: Implementation of CaloMVA cuts. Based on the original
 *              TrigTauGenericHypo.
 *
 *********************************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "ITrigTauJetHypoTool.h"


/**
 * @class TrigTauCaloHypoTool
 * @brief HLT CaloMVA step TauJet selection hypothesis tools
 **/
class TrigTauCaloHypoTool : public extends<AthAlgTool, ITrigTauJetHypoTool> {
public:
    TrigTauCaloHypoTool(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode decide(std::vector<ITrigTauJetHypoTool::ToolInfo>& input) const override;
    virtual bool decide(const ITrigTauJetHypoTool::ToolInfo& i) const override;

private:
    HLT::Identifier m_decisionId;

    Gaudi::Property<float> m_ptMin {this, "PtMin", 0, "Tau pT minimum cut"};

    Gaudi::Property<bool> m_acceptAll {this, "AcceptAll", false, "Ignore selection"};

    ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool"};
};

#endif
