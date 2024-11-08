// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauFastTrackHypoTool_H
#define TrigTauHypo_TrigTauFastTrackHypoTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"

#include "ITrigTauFastTrackHypoTool.h"


/**
 * @class TrigTauFastTrackHypoTool
 * @brief Hypothesis tool for the fast tracking steps
 **/
class TrigTauFastTrackHypoTool : public extends<AthAlgTool, ITrigTauFastTrackHypoTool>
{
public:
    TrigTauFastTrackHypoTool(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode decide(std::vector<ITrigTauFastTrackHypoTool::ToolInfo>& input) const override;
    virtual bool decide(const ITrigTauFastTrackHypoTool::ToolInfo& i) const override;

private:
    HLT::Identifier m_decisionId;
};

#endif
