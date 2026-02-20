// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_ITrigTauTrackingHypoTool_H
#define TrigTauHypo_ITrigTauTrackingHypoTool_H

#include "GaudiKernel/IAlgTool.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODTracking/TrackParticleContainer.h"


/**
 * @brief Base class for the TrigTauTrackingHypoTool
 **/
class ITrigTauTrackingHypoTool : virtual public ::IAlgTool
{ 
public: 
    DeclareInterfaceID(ITrigTauTrackingHypoTool, 1, 0);

    struct ToolInfo {
        ToolInfo(TrigCompositeUtils::Decision* d, const TrigRoiDescriptor* r, const xAOD::TrackParticleContainer *c,
                 const TrigCompositeUtils::Decision* previousDecision)
            : decision(d),
              roi(r),
              trackParticles(c),
              previousDecisionIDs(TrigCompositeUtils::decisionIDs(previousDecision).begin(), 
          		                  TrigCompositeUtils::decisionIDs(previousDecision).end())
        {}
      
        TrigCompositeUtils::Decision* decision;
        const TrigRoiDescriptor* roi;
        const xAOD::TrackParticleContainer* trackParticles;
        const TrigCompositeUtils::DecisionIDContainer previousDecisionIDs;
    };
  
  
    /**
     * @brief decides upon all tracks.
     **/
    virtual StatusCode decide(std::vector<ToolInfo>& input) const = 0;

    /**
     * @brief Makes a decision for a single object.
     * The decision needs to be returned.
     **/ 
    virtual bool decide(const ToolInfo& i) const = 0;
}; 

#endif
