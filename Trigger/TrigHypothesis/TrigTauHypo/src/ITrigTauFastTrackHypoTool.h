// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_ITrigTauFastTrackHypoTool_H
#define TrigTauHypo_ITrigTauFastTrackHypoTool_H

#include "GaudiKernel/IAlgTool.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODTracking/TrackParticleContainer.h"


/**
 * @brief Base class for the TrigTauFastTrackHypoTool
 **/
class ITrigTauFastTrackHypoTool : virtual public ::IAlgTool
{
public: 
    DeclareInterfaceID(ITrigTauFastTrackHypoTool, 1, 0);

    virtual ~ITrigTauFastTrackHypoTool() {}

    struct ToolInfo {
        ToolInfo(TrigCompositeUtils::Decision* d, const TrigRoiDescriptor* r, const xAOD::TrackParticleContainer *c,
                 const TrigCompositeUtils::Decision* previousDecision)
            : decision(d),
              roi(r),
              trackCollection(c),
              previousDecisionIDs(TrigCompositeUtils::decisionIDs(previousDecision).begin(), 
              		          TrigCompositeUtils::decisionIDs(previousDecision).end())
        {}
      
        TrigCompositeUtils::Decision* decision;
        const TrigRoiDescriptor* roi;
        const xAOD::TrackParticleContainer* trackCollection;
        const TrigCompositeUtils::DecisionIDContainer previousDecisionIDs;
    };
    
    
    /**
     * @brief decides upon all tracks
     * Note it is for a reason a non-virtual method, it is an interface in gaudi sense and implementation.
     * There will be many tools called often to perform this quick operation and we do not want to pay for polymorphism which we do not need to use.
     * Will actually see when N obj hypos will enter the scene
     **/
    virtual StatusCode decide(std::vector<ToolInfo>& input) const = 0;

    /**
     * @brief Makes a decision for a single object
     * The decision needs to be returned
     **/ 
    virtual bool decide(const ToolInfo& i) const = 0;
}; 

#endif
