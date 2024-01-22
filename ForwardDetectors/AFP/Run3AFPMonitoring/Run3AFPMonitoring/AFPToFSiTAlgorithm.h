/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFPTOFSITALGORITHM_H
#define AFPTOFSITALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODForward/AFPSiHitContainer.h"
#include "xAODForward/AFPToFHitContainer.h"
#include "xAODForward/AFPTrackContainer.h"

#include <vector>

/**
 * @brief AFP monitoring algorithm combining data from Si-layer and ToF
 */
class AFPToFSiTAlgorithm : public AthMonitorAlgorithm {
  public:
    AFPToFSiTAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~AFPToFSiTAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ) const override;
    virtual StatusCode fillHistograms_eff( const xAOD::AFPTrackContainer&, const xAOD::AFPToFHitContainer& ) const;

  private:
    SG::ReadHandleKey<xAOD::AFPSiHitContainer> m_afpSiHitContainerKey;
    SG::ReadHandleKey<xAOD::AFPToFHitContainer> m_afpToFHitContainerKey;
    SG::ReadHandleKey<xAOD::AFPTrackContainer> m_afpTrackContainerKey;

    const std::vector<float> m_tofTrainsCoordinates[2] = {{-1.9, -4.9, -8.0, -13.0, -15.0},{-1.9, -5.3, -8.4, -13.4, -15.0}};
    const float m_tofTrainGapSize = 0.1;
};

#endif
