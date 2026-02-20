// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUHYPO_ITrigTauJetHypoTool_H
#define TRIGTAUHYPO_ITrigTauJetHypoTool_H

#include "GaudiKernel/IAlgTool.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODTau/TauJetContainer.h"


/**
 * @brief Base class for the TrigTauJetHypoTool
 **/
class ITrigTauJetHypoTool : virtual public ::IAlgTool
{ 
public: 
    DeclareInterfaceID(ITrigTauJetHypoTool, 1, 0);

    struct ToolInfo {
        ToolInfo(TrigCompositeUtils::Decision* d, const TrigRoiDescriptor* r, const xAOD::TauJetContainer *c,
                 const TrigCompositeUtils::Decision* previousDecision)
            : decision(d), 
              roi(r), 
              tauContainer(c), 
              previousDecisionIDs(TrigCompositeUtils::decisionIDs(previousDecision).begin(), 
          		          TrigCompositeUtils::decisionIDs(previousDecision).end())
        {}
    
        TrigCompositeUtils::Decision* decision;
        const TrigRoiDescriptor* roi;
        const xAOD::TauJetContainer* tauContainer;
        const TrigCompositeUtils::DecisionIDContainer previousDecisionIDs;
    };
  
  
    /**
     * @brief decides upon all inputs.
     **/
    virtual StatusCode decide(std::vector<ToolInfo>& input) const = 0;

    /**
     * @brief Makes a decision for a single object.
     * The decision needs to be returned.
     **/ 
    virtual bool decide(const ToolInfo& i) const = 0;
};

#endif
