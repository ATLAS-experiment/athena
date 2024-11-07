// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauPrecTrackHypoAlg_H
#define TrigTauHypo_TrigTauPrecTrackHypoAlg_H

#include "DecisionHandling/HypoBase.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "ITrigTauPrecTrackHypoTool.h"


/**
 * @class TrigTauPrecTrackHypoAlg
 * @brief Hypothesis algorithm for the precision tracking step
 **/
class TrigTauPrecTrackHypoAlg : public ::HypoBase
{
public: 
    TrigTauPrecTrackHypoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private: 
    ToolHandleArray<ITrigTauPrecTrackHypoTool> m_hypoTools {this, "HypoTools", {}, "Hypo tools"};
     
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey {this, "TracksKey", "", "Precision Tracks (from PT step) in view"};

    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiForID2ReadKey {this, "RoIForIDReadHandleKey", "", "Updated RoI produced in view"};
}; 

#endif
