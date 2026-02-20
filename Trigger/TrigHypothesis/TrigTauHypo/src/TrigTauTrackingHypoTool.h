// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauTrackingHypoTool_H
#define TrigTauHypo_TrigTauTrackingHypoTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"

#include "ITrigTauTrackingHypoTool.h"


/**
 * @name TrigTauTrackingHypoTool
 * @brief Hypothesis tool for the tracking steps
 **/
class TrigTauTrackingHypoTool : public extends<AthAlgTool, ITrigTauTrackingHypoTool>
{
public:
    TrigTauTrackingHypoTool(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode decide(std::vector<ITrigTauTrackingHypoTool::ToolInfo>& input) const override;
    virtual bool decide(const ITrigTauTrackingHypoTool::ToolInfo& i) const override;

private:
    HLT::Identifier m_decisionId;
};

#endif
