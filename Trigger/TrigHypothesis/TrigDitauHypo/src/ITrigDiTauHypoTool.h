/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include "AsgTools/IAsgTool.h"
#include "xAODTau/DiTauJetContainer.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigCompositeUtils/HLTIdentifier.h"


class ITrigDiTauHypoTool : virtual public::IAlgTool {
    
public:
    DeclareInterfaceID(ITrigDiTauHypoTool, 1, 0);


    virtual ~ITrigDiTauHypoTool(){};
    struct ToolInfo {
            TrigCompositeUtils::Decision* decision;
            const TrigRoiDescriptor* roi;
            const xAOD::DiTauJetContainer* diTauContainer;
            const TrigCompositeUtils::DecisionIDContainer previousDecisionIDs;
            ToolInfo(TrigCompositeUtils::Decision* d, const TrigRoiDescriptor* r, const xAOD::DiTauJetContainer *c, const TrigCompositeUtils::Decision* previousDecision)
            : previousDecisionIDs(TrigCompositeUtils::decisionIDs(previousDecision).begin(), TrigCompositeUtils::decisionIDs(previousDecision).end())
            {
                decision = d;
                roi = r;
                diTauContainer = c;
            }
};
  virtual StatusCode decide(std::vector<ToolInfo>& input) const = 0;
  virtual bool decide(const ToolInfo& i) const = 0;
};
