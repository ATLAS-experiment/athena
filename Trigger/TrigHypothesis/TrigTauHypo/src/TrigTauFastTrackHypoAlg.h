// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2014 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauFastTrackHypoAlg_H
#define TrigTauHypo_TrigTauFastTrackHypoAlg_H

#include "DecisionHandling/HypoBase.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "ITrigTauFastTrackHypoTool.h"


/**
 * @brief Hypotesis algorithm for the fast tracking steps
 **/
class TrigTauFastTrackHypoAlg : public ::HypoBase
{
public:
    TrigTauFastTrackHypoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private: 
    ToolHandleArray<ITrigTauFastTrackHypoTool> m_hypoTools {this, "HypoTools", {}, "Hypo tools"};

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey {this, "FastTracksKey", "", "Fast Tracks (from FTF steps) in view"};
  
    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiForID2ReadKey {this, "RoIForIDReadHandleKey", "", "Updated RoI produced in view"};

}; 

#endif
