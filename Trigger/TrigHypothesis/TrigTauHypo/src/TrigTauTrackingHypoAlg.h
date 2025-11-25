// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauTrackingHypoAlg_H
#define TrigTauHypo_TrigTauTrackingHypoAlg_H

#include "DecisionHandling/HypoBase.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "ITrigTauTrackingHypoTool.h"


/**
 * @class TrigTauTrackingHypoAlg
 * @brief Hypothesis algorithm for the tracking steps
 **/
class TrigTauTrackingHypoAlg : public ::HypoBase
{
public: 
    TrigTauTrackingHypoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private: 
    ToolHandleArray<ITrigTauTrackingHypoTool> m_hypoTools {this, "HypoTools", {}, "Hypo tools"};
     
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey {this, "TracksKey", "", "Track particles in view"};

    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiKey {this, "RoIKey", "", "Updated RoI produced in view"};
}; 

#endif
