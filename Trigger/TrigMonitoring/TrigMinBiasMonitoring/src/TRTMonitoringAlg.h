/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGMINBIASMONITORING_TRTMonitoringAlg_H
#define TRIGMINBIASMONITORING_TRTMonitoringAlg_H

// Framework includes
#include <AthenaMonitoring/AthMonitorAlgorithm.h>
#include "GaudiKernel/ToolHandle.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "xAODTracking/TrackParticleContainer.h"

// STL includes
#include <string>

/**
 * @class TRTMonitoringAlg
 * @brief Monitoring algorithms for TRT trigger chains
 **/
class TRTMonitoringAlg : public AthMonitorAlgorithm {
public:
  TRTMonitoringAlg(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext &context) const override;

private:
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_offlineTrkKey { this, "OfflineTrkKey", "InDetTrackParticles", "Name of Offline track counts info object produced by the HLT track counting FEX algorithm" };
  ToolHandle<InDet::IInDetTrackSelectionTool> m_trackSelectionTool {this, "TrackSelectionTool", "InDetTrackSelectionTool", "Tool for selecting tracks"};

  Gaudi::Property<std::vector<std::string>> m_triggerList {this, "triggerList", {}, "List of triggers to be monitored"};
  Gaudi::Property<std::vector<std::string>> m_refTriggerList {this, "refTriggerList", {}, "List of reference triggers used to calculate efficiency"};
};

#endif // TRIGMINBIASMONITORING_TRTMonitoringAlg_H
