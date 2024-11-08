// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauPrecTrackHypoTool_H
#define TrigTauHypo_TrigTauPrecTrackHypoTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"

#include "ITrigTauPrecTrackHypoTool.h"


/**
 * @name TrigTauPrecTrackHypoTool
 * @brief Hypothesis tool for the precision tracking step
 **/
class TrigTauPrecTrackHypoTool : public extends<AthAlgTool, ITrigTauPrecTrackHypoTool>
{
public:
    TrigTauPrecTrackHypoTool(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode decide(std::vector<ITrigTauPrecTrackHypoTool::ToolInfo>& input) const override;
    virtual bool decide(const ITrigTauPrecTrackHypoTool::ToolInfo& i) const override;

private:
    HLT::Identifier m_decisionId;
};

#endif
