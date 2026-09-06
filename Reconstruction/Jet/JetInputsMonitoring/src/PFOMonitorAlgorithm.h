/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFOMONITORALGORITHM_H
#define PFOMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "xAODPFlow/FlowElementContainer.h"
#include <string>

class EventContext;

class PFOMonitorAlgorithm : public AthMonitorAlgorithm {
public:
    PFOMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~PFOMonitorAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
private:
    SG::ReadHandleKey<xAOD::FlowElementContainer> m_ChargedPFOContainerKey {this, "JetETMissChargedParticleFlowObjects", "JetETMissChargedParticleFlowObjects"};
    SG::ReadHandleKey<xAOD::FlowElementContainer> m_NeutralPFOContainerKey {this, "JetETMissNeutralParticleFlowObjects", "JetETMissNeutralParticleFlowObjects"};
};
#endif
